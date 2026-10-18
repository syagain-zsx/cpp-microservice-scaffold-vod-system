#pragma once
#include <bite_scaffold/rpc.h>
#include <bite_scaffold/etcd.h>
#include <video.pb.h>
#include <file.pb.h>
#include <user.pb.h>
#include <base.pb.h>
#include <http.pb.h>
#include "svc_data.h"

namespace svc_gateway {
    class HttpServiceImpl : public api_message::HttpService {
        public:
            HttpServiceImpl(const std::string &user_svc_name,
                const std::string &file_svc_name,
                const std::string &video_svc_name,
                const bitesvc::SvcWatcher::ptr &watcher,
                const biterpc::SvcChannels::ptr &channels,
                const SvcData::ptr &svc_data);
            virtual void tempLogin(::google::protobuf::RpcController* controller,
                const ::api_message::TempLoginReq* request,
                ::api_message::TempLoginRsp* response,
                ::google::protobuf::Closure* done);
            virtual void sessionLogin(::google::protobuf::RpcController* controller,
                const ::api_message::SessionLoginReq* request,
                ::api_message::SessionLoginRsp* response,
                ::google::protobuf::Closure* done);
            virtual void getCode(::google::protobuf::RpcController* controller,
                const ::api_message::GetCodeReq* request,
                ::api_message::GetCodeRsp* response,
                ::google::protobuf::Closure* done);
            virtual void vcodeLogin(::google::protobuf::RpcController* controller,
                const ::api_message::CodeLoginReq* request,
                ::api_message::CodeLoginRsp* response,
                ::google::protobuf::Closure* done);
            virtual void passwdLogin(::google::protobuf::RpcController* controller,
                const ::api_message::PasswordLoginReq* request,
                ::api_message::PasswordLoginRsp* response,
                ::google::protobuf::Closure* done);
            virtual void logout(::google::protobuf::RpcController* controller,
                const ::api_message::LogoutReq* request,
                ::api_message::LogoutRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setAvatar(::google::protobuf::RpcController* controller,
                const ::api_message::SetAvatarReq* request,
                ::api_message::SetAvatarRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setNickname(::google::protobuf::RpcController* controller,
                const ::api_message::SetNicknameReq* request,
                ::api_message::SetNicknameRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setPassword(::google::protobuf::RpcController* controller,
                const ::api_message::SetPasswordReq* request,
                ::api_message::SetPasswordRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setStatus(::google::protobuf::RpcController* controller,
                const ::api_message::SetUserStatusReq* request,
                ::api_message::SetUserStatusRsp* response,
                ::google::protobuf::Closure* done);
            virtual void getUserInfo(::google::protobuf::RpcController* controller,
                const ::api_message::GetUserInfoReq* request,
                ::api_message::GetUserInfoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void newAttention(::google::protobuf::RpcController* controller,
                const ::api_message::NewFollowReq* request,
                ::api_message::NewFollowRsp* response,
                ::google::protobuf::Closure* done);
            virtual void delAttention(::google::protobuf::RpcController* controller,
                const ::api_message::DelFollowReq* request,
                ::api_message::DelFollowRsp* response,
                ::google::protobuf::Closure* done);
            virtual void newAdministrator(::google::protobuf::RpcController* controller,
                const ::api_message::NewAdminReq* request,
                ::api_message::NewAdminRsp* response,
                ::google::protobuf::Closure* done);
            virtual void delAdministrator(::google::protobuf::RpcController* controller,
                const ::api_message::DelAdminReq* request,
                ::api_message::DelAdminRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setAdministrator(::google::protobuf::RpcController* controller,
                const ::api_message::SetAdminReq* request,
                ::api_message::SetAdminRsp* response,
                ::google::protobuf::Closure* done);
            virtual void getAdminByEmail(::google::protobuf::RpcController* controller,
                const ::api_message::GetAdminReq* request,
                ::api_message::GetAdminRsp* response,
                ::google::protobuf::Closure* done);
            virtual void getAdminListByStatus(::google::protobuf::RpcController* controller,
                const ::api_message::GetAdminListReq* request,
                ::api_message::GetAdminListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void uploadPhoto(::google::protobuf::RpcController* controller,
                const ::api_message::HttpUploadImageReq* request,
                ::api_message::HttpUploadImageRsp* response,
                ::google::protobuf::Closure* done);
            virtual void downloadPhoto(::google::protobuf::RpcController* controller,
                const ::api_message::HttpDownloadImageReq* request,
                ::api_message::HttpDownloadImageRsp* response,
                ::google::protobuf::Closure* done);
            virtual void uploadVideo(::google::protobuf::RpcController* controller,
                const ::api_message::HttpUploadVideoReq* request,
                ::api_message::HttpUploadVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void downloadVideo(::google::protobuf::RpcController* controller,
                const ::api_message::HttpDownloadVideoReq* request,
                ::api_message::HttpDownloadVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void newVideo(::google::protobuf::RpcController* controller,
                const ::api_message::NewVideoReq* request,
                ::api_message::NewVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void removeVideo(::google::protobuf::RpcController* controller,
                const ::api_message::RemoveVideoReq* request,
                ::api_message::RemoveVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void judgeLike(::google::protobuf::RpcController* controller,
                const ::api_message::JudgeLikeReq* request,
                ::api_message::JudgeLikeRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setLike(::google::protobuf::RpcController* controller,
                const ::api_message::setLikeReq* request,
                ::api_message::setLikeRsp* response,
                ::google::protobuf::Closure* done);
            virtual void setPlay(::google::protobuf::RpcController* controller,
                const ::api_message::PlayVideoReq* request,
                ::api_message::PlayVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void newBarrage(::google::protobuf::RpcController* controller,
                const ::api_message::newBarrageReq* request,
                ::api_message::newBarrageRsp* response,
                ::google::protobuf::Closure* done);
            virtual void getBarrage(::google::protobuf::RpcController* controller,
                const ::api_message::getBarrageListReq* request,
                ::api_message::getBarrageListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void checkVideo(::google::protobuf::RpcController* controller,
                const ::api_message::checkVideoReq* request,
                ::api_message::checkVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void saleVideo(::google::protobuf::RpcController* controller,
                const ::api_message::saleVideoReq* request,
                ::api_message::saleVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void haltVideo(::google::protobuf::RpcController* controller,
                const ::api_message::haltVideoReq* request,
                ::api_message::haltVideoRsp* response,
                ::google::protobuf::Closure* done);
            virtual void userVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::userVideoListReq* request,
                ::api_message::userVideoListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void statusVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::StatusVideoListReq* request,
                ::api_message::StatusVideoListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void allVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::AllVideoListReq* request,
                ::api_message::AllVideoListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void typeVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::TypeVideoListReq* request,
                ::api_message::TypeVideoListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void tagVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::TagVideoListReq* request,
                ::api_message::TagVideoListRsp* response,
                ::google::protobuf::Closure* done);
            virtual void keyVideoList(::google::protobuf::RpcController* controller,
                const ::api_message::keyVideoListReq* request,
                ::api_message::keyVideoListRsp* response,
                ::google::protobuf::Closure* done);   
        private:
            std::string _user_svc_name;
            std::string _file_svc_name;
            std::string _video_svc_name;
            bitesvc::SvcWatcher::ptr _watcher;
            biterpc::SvcChannels::ptr _channels;
            SvcData::ptr _svc_data;
    };
}