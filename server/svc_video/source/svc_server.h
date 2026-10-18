#include <JianLiuScaffold/etcd.h>
#include <JianLiuScaffold/log.h>
#include "svc_rpc.h"
#include "svc_data.h"


namespace svc_video {
    class VideoServer {
        public:
            using ptr = std::shared_ptr<VideoServer>;
            VideoServer(const std::shared_ptr<brpc::Server> &server,
                const JianLiuSvc::SvcProvider::ptr &registry);
            void start();
        private:
            std::shared_ptr<brpc::Server> _server; //用于搭建网络服务器
            JianLiuSvc::SvcProvider::ptr _registry; //服务注册客户端;
    };

    struct registry_settings {
        std::string registry_center_addr; //注册中心地址
        std::string service_name; //服务名称
        std::string service_addr; //服务地址
    };

    class VideoServerBuilder {
        public:
            VideoServerBuilder() = default;
            ~VideoServerBuilder() = default;
            VideoServerBuilder &withListenPort(unsigned int listen_port);
            VideoServerBuilder &withMqUrl(const std::string &mq_url);
            VideoServerBuilder &withElasticSettings(const std::string &url);
            VideoServerBuilder &withRegistrySettings(const registry_settings &reg_settings);
            VideoServerBuilder &withMysqlSettings(const JianLiuOdb::mysql_settings &mysql_settings);
            VideoServerBuilder &withRedisSettings(const JianLiuRedis::redis_settings &redis_settings);
            VideoServerBuilder &withMqDelCacheSettings(const std::string &mqsettings);
            VideoServerBuilder &withMqSyncCacheSettings(const std::string &mqsettings);
            VideoServerBuilder &withMqDelFileSettings(const std::string &mqsettings);
            VideoServerBuilder &withMqTranscodeSettings(const std::string &mqsettings);
            VideoServer::ptr build();
        private:
            unsigned int _listen_port;
            std::string _mq_url;
            std::string _es_url;
            registry_settings _reg_settings;
            JianLiuOdb::mysql_settings _mysql_settings;
            JianLiuRedis::redis_settings _redis_settings;
            JianLiuMq::declare_settings _del_cache_settings;
            JianLiuMq::declare_settings _sync_cache_settings;
            JianLiuMq::declare_settings _del_file_settings;
            JianLiuMq::declare_settings _transcode_settings;
    };
}