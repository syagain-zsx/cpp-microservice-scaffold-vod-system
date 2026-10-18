#include "svc_server.h"

namespace svc_user {
    UserServer::UserServer(const std::shared_ptr<brpc::Server> &server, const bitesvc::SvcProvider::ptr &registry)
        : _server(server), _registry(registry) {}
    void UserServer::start() {
        _server->RunUntilAskedToQuit();
    }


    UserServerBuilder &UserServerBuilder::withListenPort(unsigned int listen_port) {
        _listen_port = listen_port;
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withMqUrl(const std::string &mq_url) {
        _mq_url = mq_url;
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withRegistrySettings(const registry_settings &reg_settings){
        _reg_settings = reg_settings;
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withMysqlSettings(const biteodb::mysql_settings &mysql_settings) {
        _mysql_settings = mysql_settings;
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withRedisSettings(const biteredis::redis_settings &redis_settings) {
        _redis_settings = redis_settings;
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withMqDelFileSettings(const std::string &declare_settings) {
        auto queue_info = biteutil::JSON::unserialize(declare_settings);
        if (!queue_info) {
            ERR("queue_info 格式错误");
            abort();
        }
        _del_file_settings.exchange = (*queue_info)["exchange"].asString();
        _del_file_settings.exchange_type = (*queue_info)["type"].asString();
        _del_file_settings.queue = (*queue_info)["queue"].asString();
        _del_file_settings.binding_key = (*queue_info)["binding_key"].asString();
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withMqDelCacheSettings(const std::string &declare_settings) {
        auto queue_info = biteutil::JSON::unserialize(declare_settings);
        if (!queue_info) {
            ERR("queue_info 格式错误");
            abort();
        }
        _del_cache_settings.exchange = (*queue_info)["exchange"].asString();
        _del_cache_settings.exchange_type = (*queue_info)["type"].asString();
        _del_cache_settings.queue = (*queue_info)["queue"].asString();
        _del_cache_settings.binding_key = (*queue_info)["binding_key"].asString();
        _del_cache_settings.delayed_ttl = (*queue_info)["delayed_ttl"].asInt64() * 1000;
        return *this;
    }
    UserServerBuilder &UserServerBuilder::withEmailSettings(const bitecode::mail_settings &mail_settings) {
        _mail_settings = mail_settings;
        return *this;
    }
    UserServer::ptr UserServerBuilder::build() {
        auto mysql = biteodb::DBFactory::mysql(_mysql_settings);
        auto redis = biteredis::RedisFactory::create(_redis_settings);
        auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(_mq_url);
        auto del_file_publisher = bitemq::MQFactory::create<bitemq::Publisher>(mq_client, _del_file_settings);
        auto del_cache = std::make_shared<svc_user::CacheDelete>(redis, mq_client, _del_cache_settings);
        auto svc_data = std::make_shared<SvcData>(redis, mysql, del_cache);
        auto mail_client = std::make_shared<bitecode::MailClient>(_mail_settings);
        auto provider = std::make_shared<bitesvc::SvcProvider>(_reg_settings.registry_center_addr, 
            _reg_settings.service_name, _reg_settings.service_addr);
        
        auto user_service_impl = new UserServiceImpl(svc_data, del_file_publisher, mail_client);
        auto server = std::make_shared<brpc::Server>();
        int ret = server->AddService(user_service_impl, brpc::SERVER_OWNS_SERVICE);
        if (ret == -1) {
            ERR("添加UserServiceImpl失败");
            abort();
        }
        brpc::ServerOptions options;
        options.idle_timeout_sec = -1;
        ret = server->Start(_listen_port, &options);
        if (ret == -1) {
            ERR("启动brpc::Server失败，端口：%d", _listen_port);
            abort();
        }
        //注册服务
        provider->registry();
        auto user_server = std::make_shared<UserServer>(server, provider);
        return user_server;
    }
}