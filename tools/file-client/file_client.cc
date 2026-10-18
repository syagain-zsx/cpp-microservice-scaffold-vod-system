// ============================================================
// 笺流文件服务联调工具
//
// 上传：
//   ./file_client --session_id=<sid> --file=./jj.jpg --mime=image/jpeg
// 下载：
//   ./file_client --session_id=<sid> --download_id=<fileId> --output=./out.jpg
//
// 说明：file 服务会通过 session_id 取上传者，必须是有效登录会话；
//       会话可通过网关 tempLogin + passwdLogin 获取。
// ============================================================
#include <JianLiuScaffold/rpc.h>
#include <JianLiuScaffold/etcd.h>
#include <JianLiuScaffold/log.h>
#include <JianLiuScaffold/util.h>

#include <file.pb.h>
#include <base.pb.h>
#include <error.h>

#include <gflags/gflags.h>

#include <chrono>
#include <iostream>
#include <thread>

DEFINE_string(registry_center_addr, "http://dev-etcd:2379", "注册中心地址");
DEFINE_string(service_name, "file", "文件服务名称");
DEFINE_string(session_id, "", "已登录会话 ID（必填）");

DEFINE_string(file, "./jj.jpg", "上传文件路径");
DEFINE_string(mime, "image/jpeg", "文件 MIME（video/* 走 uploadVideo）");

DEFINE_string(download_id, "", "下载模式：指定 file_id");
DEFINE_string(output, "./download.out", "下载保存路径");

namespace {

JianLiuRpc::ChannelPtr wait_channel(JianLiuRpc::SvcChannels &channels, const std::string &name) {
    for (int i = 0; i < 10; i++) {
        auto channel = channels.getNode(name);
        if (channel) {
            return channel;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return nullptr;
}

}  // namespace

int main(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    JianLiuLog::init();

    if (FLAGS_session_id.empty()) {
        std::cerr << "缺少 --session_id（先用网关 tempLogin + passwdLogin 获取会话）" << std::endl;
        return 1;
    }

    // 1. 服务发现
    JianLiuRpc::SvcChannels channels;
    channels.setWatch(FLAGS_service_name);
    auto online_cb = [&channels](const std::string &svc, const std::string &addr, const std::string &key) {
        channels.addNode(svc, addr, key);
    };
    auto offline_cb = [&channels](const std::string &svc, const std::string &addr, const std::string &key) {
        (void)addr;
        channels.delNode(svc, key);
    };
    JianLiuSvc::SvcWatcher watcher(FLAGS_registry_center_addr, online_cb, offline_cb);
    watcher.watch();

    auto channel = wait_channel(channels, FLAGS_service_name);
    if (!channel) {
        std::cerr << "未找到文件服务节点: " << FLAGS_service_name << std::endl;
        return 1;
    }
    api_message::FileService_Stub stub(channel.get());

    // 2. 下载模式
    if (!FLAGS_download_id.empty()) {
        api_message::DownloadImageReq request;
        api_message::DownloadImageRsp response;
        request.set_requestid("cli-download");
        request.set_sessionid(FLAGS_session_id);
        request.set_fileid(FLAGS_download_id);
        brpc::Controller cntl;
        stub.downloadPhoto(&cntl, &request, &response, nullptr);
        if (cntl.Failed()) {
            std::cerr << "rpc 调用失败: " << cntl.ErrorText() << std::endl;
            return 1;
        }
        if (response.errorcode() != (int32_t)vp_error::VpError::SUCCESS) {
            std::cerr << "下载失败: " << response.errormsg() << std::endl;
            return 1;
        }
        const std::string &body = response.result().filedata();
        if (!JianLiuUtil::FUTIL::write(FLAGS_output, body)) {
            std::cerr << "写文件失败: " << FLAGS_output << std::endl;
            return 1;
        }
        std::cout << "下载成功: " << body.size() << " 字节 -> " << FLAGS_output << std::endl;
        return 0;
    }

    // 3. 上传模式
    std::string body;
    if (!JianLiuUtil::FUTIL::read(FLAGS_file, body)) {
        std::cerr << "读取文件失败: " << FLAGS_file << std::endl;
        return 1;
    }
    std::cout << "上传文件: " << FLAGS_file << "（" << body.size() << " 字节, " << FLAGS_mime << "）" << std::endl;

    bool is_video = FLAGS_mime.rfind("video/", 0) == 0;
    std::string file_id;
    std::string error_msg;

    if (is_video) {
        // 视频上传走 HTTP + protobuf attachment（与网关的转发方式一致）
        auto addr = channels.getNodeAddr(FLAGS_service_name);
        if (!addr) {
            std::cerr << "未找到文件服务节点地址" << std::endl;
            return 1;
        }
        brpc::Channel http_channel;
        brpc::ChannelOptions options;
        options.protocol = brpc::PROTOCOL_HTTP;
        options.timeout_ms = 300000;
        if (http_channel.Init(addr->c_str(), &options) != 0) {
            std::cerr << "初始化 HTTP 信道失败: " << *addr << std::endl;
            return 1;
        }
        api_message::UploadVideoReq request;
        request.set_requestid("cli-upload-video");
        request.set_sessionid(FLAGS_session_id);
        auto file_info = request.mutable_fileinfo();
        file_info->set_filemime(FLAGS_mime);
        file_info->set_filedata(body);

        brpc::Controller cntl;
        cntl.http_request().set_method(brpc::HTTP_METHOD_POST);
        cntl.http_request().uri().set_path("/FileService/uploadVideo");
        cntl.http_request().SetHeader("Content-Type", "application/protobuf");
        cntl.request_attachment().append(request.SerializeAsString());
        http_channel.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);

        if (cntl.Failed()) {
            std::cerr << "HTTP 调用失败: " << cntl.ErrorText() << std::endl;
            return 1;
        }
        if (cntl.http_response().status_code() != 200) {
            std::cerr << "上传视频失败，HTTP " << cntl.http_response().status_code() << std::endl;
            return 1;
        }
        api_message::UploadVideoRsp response;
        if (!response.ParseFromString(cntl.response_attachment().to_string())) {
            std::cerr << "解析上传响应失败" << std::endl;
            return 1;
        }
        if (response.errorcode() != (int32_t)vp_error::VpError::SUCCESS) {
            error_msg = response.errormsg();
        } else {
            file_id = response.result().fileid();
        }
    } else {
        api_message::UploadImageReq request;
        api_message::UploadImageRsp response;
        request.set_requestid("cli-upload-image");
        request.set_sessionid(FLAGS_session_id);
        auto file_info = request.mutable_fileinfo();
        file_info->set_filemime(FLAGS_mime);
        file_info->set_filedata(body);
        brpc::Controller cntl;
        stub.uploadPhoto(&cntl, &request, &response, nullptr);
        if (cntl.Failed()) {
            std::cerr << "rpc 调用失败: " << cntl.ErrorText() << std::endl;
            return 1;
        }
        if (response.errorcode() != (int32_t)vp_error::VpError::SUCCESS) {
            error_msg = response.errormsg();
        } else {
            file_id = response.result().fileid();
        }
    }

    if (!file_id.empty()) {
        std::cout << "上传成功，fileId = " << file_id << std::endl;
        return 0;
    }
    std::cerr << "上传失败: " << error_msg << std::endl;
    return 1;
}
