
#include <bite_scaffold/etcd.h>
#include <bite_scaffold/log.h>
#include "svc_rpc.h"
#include "svc_data.h"
#include "svc_sync.h"

namespace svc_user {
    class UserServer {
        public:
            using ptr = std::shared_ptr<UserServer>;
            UserServer(const std::shared_ptr<brpc::Server> &server,
                const bitesvc::SvcProvider::ptr &registry);
            void start();
        private:
            std::shared_ptr<brpc::Server> _server; //用于搭建网络服务器
            bitesvc::SvcProvider::ptr _registry; //服务注册客户端;
    };

    struct registry_settings {
        std::string registry_center_addr; //注册中心地址
        std::string service_name; //服务名称
        std::string service_addr; //服务地址
    };
    class UserServerBuilder {
        public:
            UserServerBuilder() = default;
            ~UserServerBuilder() = default;
            UserServerBuilder &withListenPort(unsigned int listen_port);
            UserServerBuilder &withMqUrl(const std::string &mq_url);
            UserServerBuilder &withRegistrySettings(const registry_settings &reg_settings);
            UserServerBuilder &withMysqlSettings(const biteodb::mysql_settings &mysql_settings);
            UserServerBuilder &withRedisSettings(const biteredis::redis_settings &redis_settings);
            UserServerBuilder &withMqDelFileSettings(const std::string &declare_settings);
            UserServerBuilder &withMqDelCacheSettings(const std::string &declare_settings);
            UserServerBuilder &withEmailSettings(const bitecode::mail_settings &mail_settings);
            UserServer::ptr build();
        private:
            unsigned int _listen_port;
            std::string _mq_url;
            registry_settings _reg_settings;
            biteodb::mysql_settings _mysql_settings;
            biteredis::redis_settings _redis_settings;
            bitemq::declare_settings _del_file_settings;
            bitemq::declare_settings _del_cache_settings;
            bitecode::mail_settings _mail_settings;
    };
}