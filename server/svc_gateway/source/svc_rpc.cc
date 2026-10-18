#include "svc_rpc.h"
#include <google/protobuf/util/json_util.h>

namespace svc_gateway {
    HttpServiceImpl::HttpServiceImpl(const std::string &user_svc_name,
        const std::string &file_svc_name,
        const std::string &video_svc_name,
        const bitesvc::SvcWatcher::ptr &watcher,
        const biterpc::SvcChannels::ptr &channels,
        const SvcData::ptr &svc_data)
        : _user_svc_name(user_svc_name) 
        , _file_svc_name(file_svc_name)
        , _video_svc_name(video_svc_name)
        , _watcher(watcher)
        , _channels(channels)
        , _svc_data(svc_data) {}
    void HttpServiceImpl::tempLogin(::google::protobuf::RpcController* controller,
        const ::api_message::TempLoginReq* request,
        ::api_message::TempLoginRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到临时用户登录请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），
            //2. 提取HTTP请求的URI
            //3. 进行客户端请求鉴权  （若鉴权失败，则返回权限不足错误）（临时用户登录不需要鉴权）
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("临时用户登录请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.tempLogin(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("临时用户登录失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::sessionLogin(::google::protobuf::RpcController* controller,
        const ::api_message::SessionLoginReq* request,
        ::api_message::SessionLoginRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到会话用户登录请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("会话用户登录请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.sessionLogin(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("会话用户登录失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::getCode(::google::protobuf::RpcController* controller,
        const ::api_message::GetCodeReq* request,
        ::api_message::GetCodeRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到获取验证码请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取验证码请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.getCode(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取验证码失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::vcodeLogin(::google::protobuf::RpcController* controller,
        const ::api_message::CodeLoginReq* request,
        ::api_message::CodeLoginRsp* response,
        ::google::protobuf::Closure* done)  {
        DBG("收到验证码注册登录请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("验证码注册登录请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.vcodeLogin(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("验证码注册登录失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::passwdLogin(::google::protobuf::RpcController* controller,
        const ::api_message::PasswordLoginReq* request,
        ::api_message::PasswordLoginRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到用户名密码登录请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("用户名密码登录请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.passwdLogin(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("用户名密码登录录失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::logout(::google::protobuf::RpcController* controller,
        const ::api_message::LogoutReq* request,
        ::api_message::LogoutRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到退出登录请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("退出登录请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.logout(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("退出登录失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setAvatar(::google::protobuf::RpcController* controller,
        const ::api_message::SetAvatarReq* request,
        ::api_message::SetAvatarRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到设置头像请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("设置头像请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.setAvatar(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("设置头像失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setNickname(::google::protobuf::RpcController* controller,
        const ::api_message::SetNicknameReq* request,
        ::api_message::SetNicknameRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到设置昵称请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("设置昵称请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.setNickname(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("设置昵称失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setPassword(::google::protobuf::RpcController* controller,
        const ::api_message::SetPasswordReq* request,
        ::api_message::SetPasswordRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到设置密码称请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("设置密码请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.setPassword(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("设置密码失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setStatus(::google::protobuf::RpcController* controller,
        const ::api_message::SetUserStatusReq* request,
        ::api_message::SetUserStatusRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到设置状态称请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("设置状态请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.setStatus(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("设置状态失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::getUserInfo(::google::protobuf::RpcController* controller,
        const ::api_message::GetUserInfoReq* request,
        ::api_message::GetUserInfoRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到获取用户信息请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取用户信息请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.getUserInfo(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取用户信息失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::newAttention(::google::protobuf::RpcController* controller,
        const ::api_message::NewFollowReq* request,
        ::api_message::NewFollowRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到新增关注请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("新增关注请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.newAttention(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("新增关注失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::delAttention(::google::protobuf::RpcController* controller,
        const ::api_message::DelFollowReq* request,
        ::api_message::DelFollowRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到取消关注请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("取消关注请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.delAttention(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("取消关注失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::newAdministrator(::google::protobuf::RpcController* controller,
        const ::api_message::NewAdminReq* request,
        ::api_message::NewAdminRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到新增管理员请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("新增管理员请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.newAdministrator(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("新增管理员失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::delAdministrator(::google::protobuf::RpcController* controller,
        const ::api_message::DelAdminReq* request,
        ::api_message::DelAdminRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到删除管理员请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("删除管理员请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.delAdministrator(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("删除管理员失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setAdministrator(::google::protobuf::RpcController* controller,
        const ::api_message::SetAdminReq* request,
        ::api_message::SetAdminRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到编辑管理员请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("编辑管理员请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.setAdministrator(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("编辑管理员失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::getAdminByEmail(::google::protobuf::RpcController* controller,
        const ::api_message::GetAdminReq* request,
        ::api_message::GetAdminRsp* response,
        ::google::protobuf::Closure* done) {
        DBG("收到通过邮箱获取管理员请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("通过邮箱获取管理员请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.getAdminByEmail(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("通过邮箱获取管理员失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::getAdminListByStatus(::google::protobuf::RpcController* controller,
        const ::api_message::GetAdminListReq* request,
        ::api_message::GetAdminListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取管理员列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_user_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取管理员列表请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::UserService_Stub stub(channel.get());
            stub.getAdminListByStatus(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取管理员列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::uploadPhoto(::google::protobuf::RpcController* controller,
        const ::api_message::HttpUploadImageReq* request,
        ::api_message::HttpUploadImageRsp* response,
        ::google::protobuf::Closure* done) {
        //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string uri = http_cntl->http_request().uri().path();
        const std::string *ssid = http_cntl->http_request().uri().GetQuery("sessionId");
        const std::string *rid = http_cntl->http_request().uri().GetQuery("requestId");
        try {
            if (!ssid) {
                throw vp_error::VPException(vp_error::VpError::SESSION_INVALID);
            }
            _svc_data->judgePermission(*ssid, uri);
            //2. 从请求正文中提取文件数据，从请求头部中提取正文类型mime
            std::string mime = http_cntl->http_request().content_type();
            auto &body = http_cntl->request_attachment();
            //3. 根据HTTP请求中提取到的信息，组织rpc请求对象
            api_message::UploadImageReq *rpc_req = new api_message::UploadImageReq;
            api_message::UploadImageRsp *rpc_rsp = new api_message::UploadImageRsp;
            if (rid) rpc_req->set_requestid(*rid);
            rpc_req->set_sessionid(*ssid);
            auto file_info = rpc_req->mutable_fileinfo();
            file_info->set_filemime(mime);
            file_info->set_filedata(body.to_string());
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_file_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                std::unique_ptr<api_message::UploadImageReq> req_guard(rpc_req);
                std::unique_ptr<api_message::UploadImageRsp> rsp_guard(rpc_rsp);
                if (cntl->Failed() || rpc_rsp->errorcode() != (int)vp_error::VpError::SUCCESS) {
                    //将rpc响应，转换成为http响应，发送给客户端
                    ERR("上传图片请求失败!");
                    http_cntl->http_response().set_status_code(500);
                    return ;
                }
                http_cntl->http_response().set_status_code(200);
                http_cntl->http_response().set_content_type("application/json");
                //应该将子服务响应的数据，先转换为json格式的字符串，再设置到http响应正文中
                std::string json_str;
                google::protobuf::util::MessageToJsonString(*rpc_rsp, &json_str);
                http_cntl->response_attachment().append(json_str);
            });
            api_message::FileService_Stub stub(channel.get());
            stub.uploadPhoto(cntl, rpc_req, rpc_rsp, closure);
        } catch (const vp_error::VPException& e) {
            ERR("上传图片失败!");
            brpc::ClosureGuard done_guard(done);
            api_message::UploadImageRsp rsp;
            if (rid) rsp.set_requestid(*rid);
            rsp.set_errorcode((int)e.errCode());
            rsp.set_errormsg(e.what());
            http_cntl->http_response().set_status_code(500);
            http_cntl->http_response().set_content_type("application/json");
            http_cntl->response_attachment().append(rsp.SerializeAsString());
        }
    }
    void HttpServiceImpl::downloadPhoto(::google::protobuf::RpcController* controller,
        const ::api_message::HttpDownloadImageReq* request,
        ::api_message::HttpDownloadImageRsp* response,
        ::google::protobuf::Closure* done) {
        //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string uri = http_cntl->http_request().uri().path();
        const std::string *ssid = http_cntl->http_request().uri().GetQuery("sessionId");
        const std::string *rid = http_cntl->http_request().uri().GetQuery("requestId");
        try {
            if (!ssid) {
                throw vp_error::VPException(vp_error::VpError::SESSION_INVALID);
            }
            _svc_data->judgePermission(*ssid, uri);
            //2. 提取请求中的图片文件ID（http请求的查询字符串中）
            const std::string *file_id = http_cntl->http_request ().uri().GetQuery("fileId");
            if (!file_id) {
                throw vp_error::VPException(vp_error::VpError::CLIENT_REQUEST_PARAM_ERROR);
            }
            //3. 组织rpc请求对象
            api_message::DownloadImageReq *rpc_req = new api_message::DownloadImageReq;
            api_message::DownloadImageRsp *rpc_rsp = new api_message::DownloadImageRsp;
            if (rid) rpc_req->set_requestid(*rid);
            rpc_req->set_sessionid(*ssid);
            rpc_req->set_fileid(*file_id);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_file_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                std::unique_ptr<api_message::DownloadImageReq> req_guard(rpc_req);
                std::unique_ptr<api_message::DownloadImageRsp> rsp_guard(rpc_rsp);
                if (cntl->Failed() || rpc_rsp->errorcode() != (int)vp_error::VpError::SUCCESS) {
                    //将rpc响应，转换成为http响应，发送给客户端
                    ERR("下载图片请求失败!");
                    http_cntl->http_response().set_status_code(500);
                    return ;
                }
                http_cntl->http_response().set_status_code(200);
                http_cntl->http_response().set_content_type(rpc_rsp->result().filemime());
                http_cntl->response_attachment().append(rpc_rsp->result().filedata());
            });
            api_message::FileService_Stub stub(channel.get());
            stub.downloadPhoto(cntl, rpc_req, rpc_rsp, closure);
        } catch (const vp_error::VPException& e) {
            ERR("下载图片失败!");
            brpc::ClosureGuard done_guard(done);
            http_cntl->http_response().set_status_code(500);
        }
    }
    void HttpServiceImpl::uploadVideo(::google::protobuf::RpcController* controller,
        const ::api_message::HttpUploadVideoReq* request,
        ::api_message::HttpUploadVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //别忘了最后调用done->Run()
        brpc::ClosureGuard done_guard(done);
        //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string uri = http_cntl->http_request().uri().path();
        const std::string *ssid = http_cntl->http_request().uri().GetQuery("sessionId");
        const std::string *rid = http_cntl->http_request().uri().GetQuery("requestId");
        try {
            if (!ssid) {
                throw vp_error::VPException(vp_error::VpError::SESSION_INVALID);
            }
            _svc_data->judgePermission(*ssid, uri);
            //2. 从请求正文中提取文件数据，从请求头部中提取正文类型mime
            std::string mime = http_cntl->http_request().content_type();
            auto &body = http_cntl->request_attachment();
            //3. 根据HTTP请求中提取到的信息，组织rpc请求对象
            api_message::UploadVideoReq rpc_req;
            api_message::UploadVideoRsp rpc_rsp;
            if (rid) rpc_req.set_requestid(*rid);
            rpc_req.set_sessionid(*ssid);
            auto file_info = rpc_req.mutable_fileinfo();
            file_info->set_filemime(mime);
            file_info->set_filedata(body.to_string());
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            // 改用HTTP协议进行大数据传输 --- 也就是这里不能再直接获取通信信道，而是获取一个服务节点地址，然后创建HTTP协议信道发起HTTP请求
            auto node_addr = _channels->getNodeAddr(_file_svc_name);
            if (!node_addr) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            //创建HTTP协议信道
            brpc::ChannelOptions options;
            options.protocol = brpc::PROTOCOL_HTTP;
            options.timeout_ms = 300000;
            brpc::Channel channel;
            channel.Init(node_addr->c_str(), &options);

            //填充HTTP请求信息
            brpc::Controller cntl;
            cntl.http_request().set_method(brpc::HTTP_METHOD_POST);//设置请求方法
            cntl.http_request().uri().set_path("/FileService/uploadVideo"); // 设置请求资源路径 -- 要与服务端对应的服务接口名称对应起来
            cntl.http_request().SetHeader("Content-Type", "application/protobuf");
            cntl.request_attachment().append(rpc_req.SerializeAsString());
            channel.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);
            if (cntl.Failed() == true){
                ERR("上传视频失败!");
                throw vp_error::VPException(vp_error::VpError::RPC_REQUEST_FAILED);
            }
            //判断响应状态码
            if (cntl.http_response().status_code() != 200) {
                ERR("上传视频失败!");
                throw vp_error::VPException(vp_error::VpError::RPC_REQUEST_FAILED);
            }
            //对响应正文进行反序列化
            bool ret = rpc_rsp.ParseFromString(cntl.response_attachment().to_string());
            if (ret == false) {
                ERR("上传视频失败!");   
                throw vp_error::VPException(vp_error::VpError::RPC_REQUEST_FAILED);
            }
            http_cntl->http_response().set_status_code(200);
            http_cntl->http_response().set_content_type("application/json");
            //应该将子服务响应的数据，先转换为json格式的字符串，再设置到http响应正文中
            std::string json_str;
            google::protobuf::util::MessageToJsonString(rpc_rsp, &json_str);
            http_cntl->response_attachment().append(json_str);
            DBG("视频上传请求完毕");
        } catch (const vp_error::VPException& e) {
            ERR("上传视频失败!");
            api_message::UploadImageRsp rsp;
            if (rid) rsp.set_requestid(*rid);
            rsp.set_errorcode((int)e.errCode());
            rsp.set_errormsg(e.what());
            http_cntl->http_response().set_status_code(500);
            http_cntl->http_response().set_content_type("application/json");
            http_cntl->response_attachment().append(rsp.SerializeAsString());
        }
    }
    void HttpServiceImpl::downloadVideo(::google::protobuf::RpcController* controller,
        const ::api_message::HttpDownloadVideoReq* request,
        ::api_message::HttpDownloadVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string uri = http_cntl->http_request().uri().path();
        const std::string *ssid = http_cntl->http_request().uri().GetQuery("sessionId");
        const std::string *rid = http_cntl->http_request().uri().GetQuery("requestId");
        try {
            // if (!ssid) {
            //     throw vp_error::VPException(vp_error::VpError::SESSION_INVALID);
            // }
            // _svc_data->judgePermission(*ssid, uri);
            //2. 提取请求中的图片文件ID（http请求的查询字符串中）
            const std::string *file_id = http_cntl->http_request ().uri().GetQuery("fileId");
            if (!file_id) {
                throw vp_error::VPException(vp_error::VpError::CLIENT_REQUEST_PARAM_ERROR);
            }
            //3. 组织rpc请求对象
            api_message::DownloadVideoReq *rpc_req = new api_message::DownloadVideoReq;
            api_message::DownloadVideoRsp *rpc_rsp = new api_message::DownloadVideoRsp;
            if (rid) rpc_req->set_requestid(*rid);
            if (ssid) rpc_req->set_sessionid(*ssid);
            if (file_id) rpc_req->set_fileid(*file_id);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_file_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                std::unique_ptr<api_message::DownloadVideoReq> req_guard(rpc_req);
                std::unique_ptr<api_message::DownloadVideoRsp> rsp_guard(rpc_rsp);
                if (cntl->Failed() || rpc_rsp->errorcode() != (int)vp_error::VpError::SUCCESS) {
                    //将rpc响应，转换成为http响应，发送给客户端
                    ERR("下载视频请求失败!");
                    http_cntl->http_response().set_status_code(500);
                    return ;
                }
                http_cntl->http_response().set_status_code(200);
                http_cntl->http_response().set_content_type(rpc_rsp->result().filemime());
                http_cntl->response_attachment().append(rpc_rsp->result().filedata());
            });
            api_message::FileService_Stub stub(channel.get());
            stub.downloadVideo(cntl, rpc_req, rpc_rsp, closure);
        } catch (const vp_error::VPException& e) {
            ERR("下载视频失败: {}!", e.what());
            brpc::ClosureGuard done_guard(done);
            http_cntl->http_response().set_status_code(500);
        }
    }
    void HttpServiceImpl::newVideo(::google::protobuf::RpcController* controller,
        const ::api_message::NewVideoReq* request,
        ::api_message::NewVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到新增视频信息请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("新增视频信息请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.newVideo(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("新增视频信息失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::removeVideo(::google::protobuf::RpcController* controller,
        const ::api_message::RemoveVideoReq* request,
        ::api_message::RemoveVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到删除视频信息请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("删除视频信息请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.removeVideo(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("删除视频信息失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::judgeLike(::google::protobuf::RpcController* controller,
        const ::api_message::JudgeLikeReq* request,
        ::api_message::JudgeLikeRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到视频点赞判断请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("视频点赞判断请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.judgeLike(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("视频点赞判断失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setLike(::google::protobuf::RpcController* controller,
        const ::api_message::setLikeReq* request,
        ::api_message::setLikeRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到视频点赞操作请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("视频点赞操作请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.setLike(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("视频点赞操作失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::setPlay(::google::protobuf::RpcController* controller,
        const ::api_message::PlayVideoReq* request,
        ::api_message::PlayVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到视频播放请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("视频播放请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.setPlay(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("视频播放失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::newBarrage(::google::protobuf::RpcController* controller,
        const ::api_message::newBarrageReq* request,
        ::api_message::newBarrageRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到新增弹幕请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("新增弹幕请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.newBarrage(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("新增弹幕失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::getBarrage(::google::protobuf::RpcController* controller,
        const ::api_message::getBarrageListReq* request,
        ::api_message::getBarrageListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取视频弹幕请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取视频弹幕请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.getBarrage(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取视频弹幕失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::checkVideo(::google::protobuf::RpcController* controller,
        const ::api_message::checkVideoReq* request,
        ::api_message::checkVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到视频审核请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("视频审核请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.checkVideo(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("视频审核失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::saleVideo(::google::protobuf::RpcController* controller,
        const ::api_message::saleVideoReq* request,
        ::api_message::saleVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到视频上架请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("视频上架请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.saleVideo(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("视频上架失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::haltVideo(::google::protobuf::RpcController* controller,
        const ::api_message::haltVideoReq* request,
        ::api_message::haltVideoRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到视频下架请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("视频下架请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.haltVideo(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("视频下架失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::userVideoList(::google::protobuf::RpcController* controller,
        const ::api_message::userVideoListReq* request,
        ::api_message::userVideoListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取用户视频列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取用户视频列表请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.userVideoList(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取用户视频列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::statusVideoList(::google::protobuf::RpcController* controller,
        const ::api_message::StatusVideoListReq* request,
        ::api_message::StatusVideoListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取状态视频列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取状态视频列表请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.statusVideoList(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取状态视频列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::allVideoList(::google::protobuf::RpcController* controller,
        const ::api_message::AllVideoListReq* request,
        ::api_message::AllVideoListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取主页视频列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取主页视频列表失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.allVideoList(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取主页视频列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::typeVideoList(::google::protobuf::RpcController* controller,
        const ::api_message::TypeVideoListReq* request,
        ::api_message::TypeVideoListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("获取分类视频列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取分类视频列表请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.typeVideoList(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取分类视频列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::tagVideoList(::google::protobuf::RpcController* controller,
        const ::api_message::TagVideoListReq* request,
        ::api_message::TagVideoListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取标签视频列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取标签视频列表请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.tagVideoList(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取标签视频列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }
    void HttpServiceImpl::keyVideoList(::google::protobuf::RpcController* controller,
        const ::api_message::keyVideoListReq* request,
        ::api_message::keyVideoListRsp* response,
        ::google::protobuf::Closure* done) {
        //
        DBG("收到获取关键字视频列表请求...");
        auto err_response = [=](vp_error::VpError err_code) {
            response->set_requestid(request->requestid());
            response->set_errorcode((int)err_code);
            response->set_errormsg(vp_error::vp_error_to_string(err_code));
        };
        try {
            //1. 提取会话ID（临时用户登录不需要），提取HTTP请求的URI，进行客户端请求鉴权
            std::string ssid = request->sessionid();
            brpc::Controller* http_cntl = (brpc::Controller*)controller;
            std::string uri = http_cntl->http_request().uri().path();
            _svc_data->judgePermission(ssid, uri);
            //4. 获取子服务信道，向子服务转发请求（异步处理过程，需要先提前编写并设定响应回调函数--内部就是将响应转发给客户端即可）
            biterpc::ChannelPtr channel = _channels->getNode(_video_svc_name);
            if (!channel) throw vp_error::VPException(vp_error::VpError::SERVICE_NOT_FOUND);
            brpc::Controller* cntl = new brpc::Controller;
            auto closure = biterpc::ClosureFactory::create([=](){
                brpc::ClosureGuard done_guard(done);
                std::unique_ptr<brpc::Controller> cntl_guard(cntl);
                if (cntl->Failed()) {
                    ERR("获取关键字视频列表请求失败!");
                    err_response(vp_error::VpError::RPC_REQUEST_FAILED);
                }
            });
            api_message::VideoService_Stub stub(channel.get());
            stub.keyVideoList(cntl, request, response, closure);
        } catch (const vp_error::VPException& e) {
            ERR("获取关键字视频列表失败!");
            brpc::ClosureGuard done_guard(done);
            err_response(e.errCode());
        }
    }   
}