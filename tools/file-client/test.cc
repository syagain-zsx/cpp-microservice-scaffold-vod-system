#include <bite_scaffold/rpc.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include <bite_scaffold/etcd.h>
#include <bite_scaffold/util.h>
#include <gflags/gflags.h>

#include <file.pb.h>
#include <base.pb.h>
#include <message.pb.h>

#include <error.h>


DEFINE_string(registry_center_addr, "http://192.168.65.128:2379", "注册中心地址");
DEFINE_string(service_name, "file", "服务名称");

DEFINE_string(mq_url, "amqp://admin:123456@192.168.65.128:5672/", "消息队列地址");
DEFINE_string(delete_queue_info, R"({"exchange": "delete_file_exchange", "type": "direct", "queue": "delete_file_queue", "binding_key": "delete_file_queue"})", "删除文件队列信息");


int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 1. 初始化日志输出
    bitelog::bitelog_init();
    // 2. 定义MQClient对象
    auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(FLAGS_mq_url);
    // 3. 定义Publisher对象
    auto delete_queue_info = biteutil::JSON::unserialize(FLAGS_delete_queue_info);
    if (!delete_queue_info) {
        ERR("delete_queue_info 格式错误");
        return -1;
    }
    bitemq::declare_settings dsettings = {
        .exchange = (*delete_queue_info)["exchange"].asString(),
        .exchange_type = (*delete_queue_info)["type"].asString(),
        .queue = (*delete_queue_info)["queue"].asString(),
        .binding_key = (*delete_queue_info)["binding_key"].asString(),
    };
    auto publisher = bitemq::MQFactory::create<bitemq::Publisher>(mq_client, dsettings);
    // 4. 定义biterpc::SvcChannels服务管理对象
    biterpc::SvcChannels scs;
    scs.setWatch(FLAGS_service_name);
    auto online_cb = std::bind(&biterpc::SvcChannels::addNode, &scs, std::placeholders::_1, std::placeholders::_2);
    auto offline_cb = std::bind(&biterpc::SvcChannels::delNode, &scs, std::placeholders::_1, std::placeholders::_2);
    // 5. 定义bitesvc::SvcWatcher服务发现对象
    bitesvc::SvcWatcher watcher(FLAGS_registry_center_addr, online_cb, offline_cb);
    watcher.watch();

    std::string image_file_id, video_file_id;
    {
        //1. 上传图片文件
        //1.1 获取通信通道对象channel
        biterpc::ChannelPtr channel;
        while(!channel){
            // ERR("没有可供调用的服务！");
            channel = scs.getNode(FLAGS_service_name);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        DBG("找到能够提供服务的节点！");
        //1.2 定义假请求
        api_message::UploadImageReq request;
        api_message::UploadImageRsp response;
        request.set_requestid("上传图片请求ID");
        request.set_sessionid("hJ4LcW0taEsqGXKD0001");
        auto *file_info = request.mutable_fileinfo();
        file_info->set_filemime("image/jpeg");
        std::string body;
        biteutil::FUTIL::read("./jj.jpg", body);
        file_info->set_filedata(body);
        //1.3 向rpc服务器发起请求
        api_message::FileService_Stub stub(channel.get());
        brpc::Controller cntl;
        stub.uploadPhoto(&cntl, &request, &response, nullptr);
        if (cntl.Failed()) {
            ERR("rpc call failed! error = {}", cntl.ErrorText());
            return -1;
        }
        //1.4 处理响应（获取到了一个文件ID）
        if (response.errorcode() == (int32_t)vp_error::VpError::SUCCESS) {
            std::cout << "文件ID: " << response.result().fileid() << std::endl;
            image_file_id = response.result().fileid();
        }else {
            std::cout << "上传图片失败: " << response.errormsg() << std::endl;
            return -1;
        }
        std::cout << "上传图片ID:" << image_file_id << std::endl;
    }
    // {
    //     //2. 下载图片文件
    //     //1.1 获取通信通道对象channel
    //     biterpc::ChannelPtr channel;
    //     while(!channel){
    //         // ERR("没有可供调用的服务！");
    //         channel = scs.getNode(FLAGS_service_name);
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    //     }
    //     DBG("找到能够提供服务的节点！");
    //     //1.2 定义假请求
    //     api_message::DownloadImageReq request;
    //     api_message::DownloadImageRsp response;
    //     request.set_requestid("下载图片请求ID");
    //     request.set_sessionid("session_id_1");
    //     request.set_fileid(image_file_id);
    //     //1.3 向rpc服务器发起请求
    //     api_message::FileService_Stub stub(channel.get());
    //     brpc::Controller cntl;
    //     stub.downloadPhoto(&cntl, &request, &response, nullptr);
    //     //1.4 处理响应（获取到了一个文件mime和文件数据）
    //     if (cntl.Failed()) {
    //         ERR("rpc call failed! error = {}", cntl.ErrorText());
    //         return -1;
    //     }
    //     //1.5 把文件数据写入到本地文件中
    //     if (response.errorcode() == (int32_t)vp_error::VpError::SUCCESS) {
    //         std::cout << "文件mime: " << response.result().filemime() << std::endl;
    //         const std::string &body = response.result().filedata();
    //         biteutil::FUTIL::write("./jj2.jpg", body);
    //     }else {
    //         std::cout << "下载图片失败: " << response.errormsg() << std::endl;
    //         return -1;
    //     }
    // }
    
    // {
    //     //4. 上传视频文件
    //     //1.1 获取通信通道对象channel
    //     biterpc::ChannelPtr channel;
    //     while(!channel){
    //         // ERR("没有可供调用的服务！");
    //         channel = scs.getNode(FLAGS_service_name);
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    //     }
    //     DBG("找到能够提供服务的节点！");
    //     //1.2 定义假请求
    //     api_message::UploadVideoReq request;
    //     api_message::UploadVideoRsp response;
    //     request.set_requestid("上传视频请求ID");
    //     request.set_sessionid("hJ4LcW0taEsqGXKD0001");
    //     auto *file_info = request.mutable_fileinfo();
    //     file_info->set_filemime("mpeg/mp4");
    //     std::string body;
    //     biteutil::FUTIL::read("./movie.mp4", body);
    //     file_info->set_filedata(body);
    //     //1.3 向rpc服务器发起请求
    //     api_message::FileService_Stub stub(channel.get());
    //     brpc::Controller cntl;
    //     stub.uploadVideo(&cntl, &request, &response, nullptr);
    //     if (cntl.Failed()) {
    //         ERR("rpc call failed! error = {}", cntl.ErrorText());
    //         return -1;
    //     }
    //     //1.4 处理响应（获取到了一个文件ID）
    //     if (response.errorcode() == (int32_t)vp_error::VpError::SUCCESS) {
    //         std::cout << "文件ID: " << response.result().fileid() << std::endl;
    //         video_file_id = response.result().fileid();
    //     }else {
    //         std::cout << "上传视频失败: " << response.errormsg() << std::endl;
    //         return -1;
    //     }
    // }
    // {
    //     //5. 下载视频文件
    //     biterpc::ChannelPtr channel;
    //     while(!channel){
    //         // ERR("没有可供调用的服务！");
    //         channel = scs.getNode(FLAGS_service_name);
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    //     }
    //     DBG("找到能够提供服务的节点！");
    //     //1.2 定义假请求
    //     api_message::DownloadVideoReq request;
    //     api_message::DownloadVideoRsp response;
    //     request.set_requestid("下载视频请求ID");
    //     request.set_sessionid("zW9OaaOw11L8WGaw0001");
    //     request.set_fileid(video_file_id);
    //     //1.3 向rpc服务器发起请求
    //     api_message::FileService_Stub stub(channel.get());
    //     brpc::Controller cntl;
    //     stub.downloadVideo(&cntl, &request, &response, nullptr);
    //     //1.4 处理响应（获取到了一个文件mime和文件数据）
    //     if (cntl.Failed()) {
    //         ERR("rpc call failed! error = {}", cntl.ErrorText());
    //         return -1;
    //     }
    //     //1.5 把文件数据写入到本地文件中
    //     if (response.errorcode() == (int32_t)vp_error::VpError::SUCCESS) {
    //         std::cout << "文件mime: " << response.result().filemime() << std::endl;
    //         const std::string &body = response.result().filedata();
    //         biteutil::FUTIL::write("./movie2.mp4", body);
    //     }else {
    //         std::cout << "下载视频失败: " << response.errormsg() << std::endl;
    //         return -1;
    //     }
    // }
    // {
    //     //6. 删除视频文件
    //     api_message::DeleteFileMsg message;
    //     message.add_file_id(video_file_id);
    //     message.add_file_id(image_file_id);
    //     //3.2 发送消息到MQ中
    //     publisher->publish(message.SerializeAsString());
    // }
    return 0;
}
