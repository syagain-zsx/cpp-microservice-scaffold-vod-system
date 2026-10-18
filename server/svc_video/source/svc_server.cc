#include "svc_server.h"

namespace svc_video {
    VideoServer::VideoServer(const std::shared_ptr<brpc::Server> &server,
        const bitesvc::SvcProvider::ptr &registry)
        : _server(server), _registry(registry) {}
    void VideoServer::start() {
        _server->RunUntilAskedToQuit();
    }

    VideoServerBuilder &VideoServerBuilder::withListenPort(unsigned int listen_port) {
        _listen_port = listen_port;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withMqUrl(const std::string &mq_url){
        _mq_url = mq_url;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withRegistrySettings(const registry_settings &reg_settings){
        _reg_settings = reg_settings;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withMysqlSettings(const biteodb::mysql_settings &mysql_settings) {
        _mysql_settings = mysql_settings;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withRedisSettings(const biteredis::redis_settings &redis_settings){
        _redis_settings = redis_settings;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withElasticSettings(const std::string &url) {
        _es_url = url;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withMqDelCacheSettings(const std::string &mqsettings) {
        auto queue_info = biteutil::JSON::unserialize(mqsettings);
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
    VideoServerBuilder &VideoServerBuilder::withMqSyncCacheSettings(const std::string &mqsettings) {
        auto queue_info = biteutil::JSON::unserialize(mqsettings);
        if (!queue_info) {
            ERR("queue_info 格式错误");
            abort();
        }
        _sync_cache_settings.exchange = (*queue_info)["exchange"].asString();
        _sync_cache_settings.exchange_type = (*queue_info)["type"].asString();
        _sync_cache_settings.queue = (*queue_info)["queue"].asString();
        _sync_cache_settings.binding_key = (*queue_info)["binding_key"].asString();
        _sync_cache_settings.delayed_ttl = (*queue_info)["delayed_ttl"].asInt64() * 1000;
        return *this;
    }
    VideoServerBuilder &VideoServerBuilder::withMqDelFileSettings(const std::string &mqsettings)  {
        auto queue_info = biteutil::JSON::unserialize(mqsettings);
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
    VideoServerBuilder &VideoServerBuilder::withMqTranscodeSettings(const std::string &mqsettings) {
        auto queue_info = biteutil::JSON::unserialize(mqsettings);
        if (!queue_info) {
            ERR("queue_info 格式错误");
            abort();
        }
        _transcode_settings.exchange = (*queue_info)["exchange"].asString();
        _transcode_settings.exchange_type = (*queue_info)["type"].asString();
        _transcode_settings.queue = (*queue_info)["queue"].asString();
        _transcode_settings.binding_key = (*queue_info)["binding_key"].asString();
        return *this;
    }
    VideoServer::ptr VideoServerBuilder::build() {
        auto mysql = biteodb::DBFactory::mysql(_mysql_settings);
        auto redis = biteredis::RedisFactory::create(_redis_settings);
        std::vector<std::string> urls = {_es_url};
        auto elastic = std::make_shared<biteics::ESClient>(urls);
        vp_data::VideoSearchData::init(elastic);
        auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(_mq_url);
        auto del_file_publisher = bitemq::MQFactory::create<bitemq::Publisher>(mq_client, _del_file_settings);
        auto transcode_publisher = bitemq::MQFactory::create<bitemq::Publisher>(mq_client, _transcode_settings);
        
        auto del_cache = std::make_shared<svc_video::CacheDelete>(redis, mq_client, _del_cache_settings);
        auto sync_cache = std::make_shared<svc_video::CacheToDB>(mysql, redis, del_cache, mq_client, _sync_cache_settings);

        auto svc_data = std::make_shared<SvcData>(redis, mysql, elastic, sync_cache, 
            del_cache, del_file_publisher, transcode_publisher);

        auto *video_service = new VideoServiceImpl(svc_data);

        auto server = std::make_shared<brpc::Server>();
        int ret = server->AddService(video_service, brpc::SERVER_OWNS_SERVICE);
        if (ret == -1) {
            ERR("添加VideoServiceImpl失败");
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
        auto provider = std::make_shared<bitesvc::SvcProvider>(_reg_settings.registry_center_addr, 
            _reg_settings.service_name, _reg_settings.service_addr);
        provider->registry();
        auto video_server = std::make_shared<VideoServer>(server, provider);
        return video_server;
    }
}