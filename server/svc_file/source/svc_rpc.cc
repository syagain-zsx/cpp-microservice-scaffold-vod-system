#include <bite_scaffold/util.h>
#include <bite_scaffold/rpc.h>
#include "svc_rpc.h"
#include <error.h>
#include <bite_scaffold/fdfs.h>


namespace svc_file {

    const size_t FileServiceImpl::uuid_length = 16;
    
    FileServiceImpl::FileServiceImpl(const SvcData::ptr &svc_data): _svc_data(svc_data) {}
    void FileServiceImpl::uploadPhoto(::google::protobuf::RpcController* controller,
        const ::api_message::UploadImageReq* request,
        ::api_message::UploadImageRsp* response,
        ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        // 1. 获取请求元素: 请求ID, 会话ID（用于获取用户ID）, 文件数据, 文件mime
        const std::string &rid = request->requestid();
        const std::string &session_id = request->sessionid();
        auto &file_info = request->fileinfo();
        const std::string &file_mime = file_info.filemime();
        const std::string &file_data = file_info.filedata();

		vp_error::VpError res_status_code = vp_error::VpError::SUCCESS;
        try {
            // 2. 通过会话ID查询会话信息, 获得上传者用户ID（缓存未命中则从数据库获取）
            auto p_uploader = _svc_data->getSessionUser(session_id);
            DBG("图片上传用户: {}", p_uploader);
            // 3. 将文件数据上传到FDFS进行存储,并获取FDFS文件ID--作为存储路径PATH
            size_t file_size = file_data.size();
            auto file_path = bitefdfs::FDFSClient::upload_from_buff(file_data);
            if (!file_path) {
                throw vp_error::VPException(vp_error::VpError::UPLOAD_FILE_TO_FDFS_FAILED);
            }
            // 4. 生成文件ID, 向数据库新增文件信息（文件ID，文件大小，文件mime，上传用户ID，存储路径）
            std::string file_id = biteutil::Random::code(uuid_length);
            vp_data::File file;
            file.set_file_id(file_id);
            file.set_uploader_uid(p_uploader);
            file.set_path(*file_path);
            file.set_size(file_size);
            file.set_mime(file_mime);
            _svc_data->newFileMeta(file);
            auto *result = response->mutable_result();
            result->set_fileid(file_id);
        }catch (vp_error::VPException &e) {
			ERR("上传图片数据处理错误: {}-{}", rid, e.what());
			res_status_code = e.errCode();
        }catch (const std::exception &e) {
			ERR("上传图片数据处理错误: {}-{}", rid, e.what());
			res_status_code = vp_error::VpError::SERVER_INERNAL_ERROR;
		}
        // 5. 返回响应信息: 文件ID
        response->set_requestid(rid);
        response->set_errorcode((int)res_status_code);
        response->set_errormsg(vp_error::vp_error_to_string(res_status_code));
    }
    void FileServiceImpl::downloadPhoto(::google::protobuf::RpcController* controller,
        const ::api_message::DownloadImageReq* request,
        ::api_message::DownloadImageRsp* response,
        ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        // 1. 获取请求元素: 请求ID，文件ID
        const std::string &rid = request->requestid();
        const std::string &file_id = request->fileid();

		vp_error::VpError res_status_code = vp_error::VpError::SUCCESS;
        try {
            // 2. 从数据库通过文件ID查询文件信息（文件ID，文件大小，文件mime，上传用户ID，存储路径）
            auto file = _svc_data->getFileMeta(file_id);
            // 3. 通过文件信息中的路径,从FDFS进行文件数据下载到内存中
            std::string path = file->path();
            std::string file_data;
            bool ret = bitefdfs::FDFSClient::download_to_buff(path, file_data);
            if (!ret) {
                throw vp_error::VPException(vp_error::VpError::DOWNLOAD_FILE_FROM_FDFS_FAILED);
            }
            // 4. 返回响应信息: 文件ID, 上传者ID, mime, 文件大小, 文件内容
            auto *result = response->mutable_result();
            result->set_fileid(file->file_id());
            result->set_filesize(file->size());
            result->set_uploader(file->uploader_uid());
            result->set_filemime(file->mime());
            result->set_filedata(file_data);
        }catch (vp_error::VPException &e) {
			ERR("下载图片数据处理错误: {}-{}", rid, e.what());
			res_status_code = e.errCode();
        }catch (const std::exception &e) {
			ERR("下载图片数据处理错误: {}-{}", rid, e.what());
			res_status_code = vp_error::VpError::SERVER_INERNAL_ERROR;
		}
        response->set_requestid(rid);
        response->set_errorcode((int)res_status_code);
        response->set_errormsg(vp_error::vp_error_to_string(res_status_code));
    }
    void FileServiceImpl::uploadVideo(::google::protobuf::RpcController* controller,
        const ::api_message::HttpUploadVideoReq* request,
        ::api_message::HttpUploadVideoRsp* response,
        ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* http_cntl = (brpc::Controller*)controller;
		vp_error::VpError res_status_code = vp_error::VpError::SUCCESS;
        try {
            //1. 针对请求正文进行反序列化
            api_message::UploadVideoReq req;
            api_message::UploadVideoRsp rsp;
            bool ret = req.ParseFromString(http_cntl->request_attachment().to_string());
            if (!ret) {
                ERR("反序列化请求正文失败");
                throw vp_error::VPException(vp_error::VpError::CLIENT_REQUEST_PARAM_ERROR);
            }

            // 1. 获取请求元素: 请求ID, 会话ID（用于获取用户ID）, 文件数据, 文件mime
            const std::string &rid = req.requestid();
            const std::string &session_id = req.sessionid();
            auto &file_info = req.fileinfo();
            const std::string &file_mime = file_info.filemime();
            const std::string &file_data = file_info.filedata();

            // 2. 通过会话ID查询会话信息, 获得上传者用户ID（缓存未命中则从数据库获取）
            auto p_uploader = _svc_data->getSessionUser(session_id);
            DBG("视频上传用户: {}", p_uploader);
            // 3. 将文件数据上传到FDFS进行存储,并获取FDFS文件ID--作为存储路径PATH
            size_t file_size = file_data.size();
            auto file_path = bitefdfs::FDFSClient::upload_from_buff(file_data);
            if (!file_path) {
                throw vp_error::VPException(vp_error::VpError::UPLOAD_FILE_TO_FDFS_FAILED);
            }
            // 4. 生成文件ID, 向数据库新增文件信息（文件ID，文件大小，文件mime，上传用户ID，存储路径）
            std::string file_id = biteutil::Random::code(uuid_length);
            vp_data::File file;
            file.set_file_id(file_id);
            file.set_uploader_uid(p_uploader);
            file.set_path(*file_path);
            file.set_size(file_size);
            file.set_mime(file_mime);
            _svc_data->newFileMeta(file);


            // 5. 组织响应信息，并发送给网关
            rsp.set_requestid(rid);
            rsp.set_errorcode((int)res_status_code);
            rsp.set_errormsg(vp_error::vp_error_to_string(res_status_code));
            auto *result = rsp.mutable_result();
            result->set_fileid(file_id);

            http_cntl->http_response().set_status_code(200);
            http_cntl->http_response().set_content_type("application/protobuf");
            http_cntl->response_attachment().append(rsp.SerializeAsString());
            DBG("视频上传处理完毕");
        }catch (vp_error::VPException &e) {
			ERR("上传视频数据处理错误: {}", e.what());
            http_cntl->http_response().set_status_code(500);
        }catch (const std::exception &e) {
			ERR("上传视频数据处理错误: {}", e.what());
            http_cntl->http_response().set_status_code(500);
		}
    }
    void FileServiceImpl::downloadVideo(::google::protobuf::RpcController* controller,
        const ::api_message::DownloadVideoReq* request,
        ::api_message::DownloadVideoRsp* response,
        ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        // 1. 获取请求元素: 请求ID，文件ID
        const std::string &rid = request->requestid();
        const std::string &file_id = request->fileid();
        
		vp_error::VpError res_status_code = vp_error::VpError::SUCCESS;
        try {
            // 2. 从数据库通过文件ID查询文件信息（文件ID，文件大小，文件mime，上传用户ID，存储路径）
            auto file = _svc_data->getFileMeta(file_id);
            // 3. 通过文件信息中的路径,从FDFS进行文件数据下载到内存中
            std::string path = file->path();
            std::string file_data;
            bool ret = bitefdfs::FDFSClient::download_to_buff(path, file_data);
            if (!ret) {
                throw vp_error::VPException(vp_error::VpError::DOWNLOAD_FILE_FROM_FDFS_FAILED);
            }
            // 4. 返回响应信息: 文件ID, 上传者ID, mime, 文件大小, 文件内容
            auto *result = response->mutable_result();
            result->set_fileid(file->file_id());
            result->set_filesize(file->size());
            result->set_uploader(file->uploader_uid());
            result->set_filemime(file->mime());
            result->set_filedata(file_data);
        }catch (vp_error::VPException &e) {
			ERR("下载视频数据处理错误: {}-{}", rid, e.what());
			res_status_code = e.errCode();
        }catch (const std::exception &e) {
			ERR("下载视频数据处理错误: {}-{}", rid, e.what());
			res_status_code = vp_error::VpError::SERVER_INERNAL_ERROR;
		}
        response->set_requestid(rid);
        response->set_errorcode((int)res_status_code);
        response->set_errormsg(vp_error::vp_error_to_string(res_status_code));
    }
}
