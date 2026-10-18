#pragma once
#include <bite_scaffold/rpc.h>
#include <video.pb.h>
#include <base.pb.h>
#include <message.pb.h>
#include "svc_data.h"

namespace svc_video {
    class VideoServiceImpl : public api_message::VideoService {
        public:
            VideoServiceImpl(const SvcData::ptr &data);
            void newVideo(::google::protobuf::RpcController* controller,
                const ::api_message::NewVideoReq* request,
                ::api_message::NewVideoRsp* response,
                ::google::protobuf::Closure* done);
            void removeVideo(::google::protobuf::RpcController* controller,
                const ::api_message::RemoveVideoReq* request,
                ::api_message::RemoveVideoRsp* response,
                ::google::protobuf::Closure* done);
            void judgeLike(::google::protobuf::RpcController* controller,
                const ::api_message::JudgeLikeReq* request,
                ::api_message::JudgeLikeRsp* response,
                ::google::protobuf::Closure* done);
            void setLike(::google::protobuf::RpcController* controller,
                const ::api_message::setLikeReq* request,
                ::api_message::setLikeRsp* response,
                ::google::protobuf::Closure* done);
            void setPlay(::google::protobuf::RpcController* controller,
                const ::api_message::PlayVideoReq* request,
                ::api_message::PlayVideoRsp* response,
                ::google::protobuf::Closure* done);
            void newBarrage(::google::protobuf::RpcController* controller,
                const ::api_message::newBarrageReq* request,
                ::api_message::newBarrageRsp* response,
                ::google::protobuf::Closure* done);
            void getBarrage(::google::protobuf::RpcController* controller,
                const ::api_message::getBarrageListReq* request,
                ::api_message::getBarrageListRsp* response,
                ::google::protobuf::Closure* done);
            void checkVideo(::google::protobuf::RpcController* controller,
                const ::api_message::checkVideoReq* request,
                ::api_message::checkVideoRsp* response,
                ::google::protobuf::Closure* done);
            void saleVideo(::google::protobuf::RpcController* controller,
                const ::api_message::saleVideoReq* request,
                ::api_message::saleVideoRsp* response,
                ::google::protobuf::Closure* done);
            void haltVideo(::google::protobuf::RpcController* controller,
                const ::api_message::haltVideoReq* request,
                ::api_message::haltVideoRsp* response,
                ::google::protobuf::Closure* done);
            void userVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::userVideoListReq* request,
                ::api_message::userVideoListRsp* response,
                ::google::protobuf::Closure* done);
            void statusVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::StatusVideoListReq* request,
                ::api_message::StatusVideoListRsp* response,
                ::google::protobuf::Closure* done);
            void allVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::AllVideoListReq* request,
                ::api_message::AllVideoListRsp* response,
                ::google::protobuf::Closure* done);
            void typeVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::TypeVideoListReq* request,
                ::api_message::TypeVideoListRsp* response,
                ::google::protobuf::Closure* done);
            void tagVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::TagVideoListReq* request,
                ::api_message::TagVideoListRsp* response,
                ::google::protobuf::Closure* done);
            void keyVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::keyVideoListReq* request,
                ::api_message::keyVideoListRsp* response,
                ::google::protobuf::Closure* done);
        private:
            SvcData::ptr _data;
    };
}