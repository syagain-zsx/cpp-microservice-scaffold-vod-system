#include "svc_server.h"

namespace svc_gateway {

    GatewayServer::GatewayServer(const std::shared_ptr<brpc::Server> &server): _server(server) {}
    void GatewayServer::start() {
        _server->RunUntilAskedToQuit();
    }
    
    GatewayServerBuilder &GatewayServerBuilder::withListenPort(unsigned int listen_port) {
        _listen_port = listen_port;
        return *this;
    }
    GatewayServerBuilder &GatewayServerBuilder::withDiscoverySettings(const discovery_settings &dis_settings) {
        _dis_settings = dis_settings;
        return *this;
    }
    GatewayServerBuilder &GatewayServerBuilder::withMysqlSettings(const biteodb::mysql_settings &mysql_settings) {
        _mysql_settings = mysql_settings;
        return *this;
    }
    GatewayServerBuilder &GatewayServerBuilder::withRedisSettings(const biteredis::redis_settings &redis_settings) {
        _redis_settings = redis_settings;
        return *this;
    }
    GatewayServer::ptr GatewayServerBuilder::build() {
        //1. 构造数据库以及缓存操作对象
        auto mysql = biteodb::DBFactory::mysql(_mysql_settings);
        auto redis = biteredis::RedisFactory::create(_redis_settings);
        //2. 构造数据操作对象
        auto svc_data = std::make_shared<SvcData>(redis, mysql);
        //3. 构造信道管理对象
        auto channels = std::make_shared<biterpc::SvcChannels>();
        channels->setWatch(_dis_settings.user_svc_name);
        channels->setWatch(_dis_settings.file_svc_name);
        channels->setWatch(_dis_settings.video_svc_name);
        auto online_cb = std::bind(&biterpc::SvcChannels::addNode, channels.get(), std::placeholders::_1, std::placeholders::_2);
        auto offline_cb = std::bind(&biterpc::SvcChannels::delNode, channels.get(), std::placeholders::_1, std::placeholders::_2);
        //4. 构造服务发现对象
        auto watcher = std::make_shared<bitesvc::SvcWatcher>(_dis_settings.registry_center_addr, online_cb, offline_cb);
        watcher->watch();


        auto http_service_impl = new HttpServiceImpl(_dis_settings.user_svc_name,_dis_settings.file_svc_name,
            _dis_settings.video_svc_name, watcher, channels, svc_data);
        //5. 基于FileServiceImpl构造brpc::Server对象
        auto server = std::make_shared<brpc::Server>();
        int ret = server->AddService(http_service_impl, brpc::SERVER_OWNS_SERVICE);
        if (ret == -1) {
            ERR("添加HttpServiceImpl失败");
            abort();
        }
        brpc::ServerOptions options;
        options.idle_timeout_sec = -1;
        ret = server->Start(_listen_port, &options);
        if (ret == -1) {
            ERR("启动brpc::Server失败，端口：%d", _listen_port);
            abort();
        }
        //6. 构造网关服务器对象
        auto gateway_server = std::make_shared<GatewayServer>(server);
        return gateway_server;
    }
}