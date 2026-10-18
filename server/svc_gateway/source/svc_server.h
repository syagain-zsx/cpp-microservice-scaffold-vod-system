#pragma once
#include <bite_scaffold/etcd.h>
#include <bite_scaffold/rpc.h>
#include <bite_scaffold/util.h>
#include "svc_rpc.h"
#include "svc_data.h"

namespace svc_gateway {
    class GatewayServer {
        public:
            using ptr = std::shared_ptr<GatewayServer>;
            GatewayServer(const std::shared_ptr<brpc::Server> &server);
            void start();
        private:
            std::shared_ptr<brpc::Server> _server;
    };
    struct discovery_settings {
        std::string registry_center_addr; //注册中心地址
        std::string user_svc_name;
        std::string file_svc_name;
        std::string video_svc_name;
    };
    class GatewayServerBuilder {
        public:
            GatewayServerBuilder() = default;
            ~GatewayServerBuilder() = default;
            GatewayServerBuilder &withListenPort(unsigned int listen_port);
            GatewayServerBuilder &withDiscoverySettings(const discovery_settings &_dis_settings);
            GatewayServerBuilder &withMysqlSettings(const biteodb::mysql_settings &mysql_settings);
            GatewayServerBuilder &withRedisSettings(const biteredis::redis_settings &redis_settings);
            GatewayServer::ptr build();
        private:
            unsigned int _listen_port;
            discovery_settings _dis_settings;
            biteodb::mysql_settings _mysql_settings;
            biteredis::redis_settings _redis_settings;
    };
}