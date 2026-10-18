#pragma once
#include <bite_scaffold/rpc.h>
#include <bite_scaffold/mail.h>
#include <user.pb.h>
#include <base.pb.h>
#include <message.pb.h>
#include "svc_data.h"


namespace svc_user {
    class UserServiceImpl : public api_message::UserService {
        public:
            UserServiceImpl(const SvcData::ptr &svc_data, 
                const bitemq::Publisher::ptr &publisher,
                const bitecode::MailClient::ptr &mail_client);
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
        private:
            // 业务数据层的封装对象
            SvcData::ptr _svc_data;
            // 文件删除消息发布者对象
            bitemq::Publisher::ptr _publisher;
            // 邮箱客户端发送邮件验证码
            bitecode::MailClient::ptr _mail_client;
    };
}