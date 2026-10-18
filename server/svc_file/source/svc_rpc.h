#pragma once
#include "svc_data.h"
#include <file.pb.h>
#include <base.pb.h>

namespace svc_file {
    class FileServiceImpl : public api_message::FileService {
        public:
            FileServiceImpl(const SvcData::ptr &svc_data);
            virtual void uploadPhoto(::google::protobuf::RpcController* controller,
                const ::api_message::UploadImageReq* request,
                ::api_message::UploadImageRsp* response,
                ::google::protobuf::Closure* done);
            virtual void downloadPhoto(::google::protobuf::RpcController* controller,
                const ::api_message::DownloadImageReq* request,
                ::api_message::DownloadImageRsp* response,
                ::google::protobuf::Closure* done);
            virtual void uploadVideo(::google::protobuf::RpcController* controller,
                const ::api_message::HttpUploadVideoReq* request,
                ::api_message::HttpUploadVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void downloadVideo(::google::protobuf::RpcController* controller,
                const ::api_message::DownloadVideoReq* request,
                ::api_message::DownloadVideoRsp* response,
                ::google::protobuf::Closure* done);
        private:
            static const size_t uuid_length;
            SvcData::ptr _svc_data;
    };
}