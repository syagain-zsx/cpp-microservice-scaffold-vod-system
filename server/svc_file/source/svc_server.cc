#include "svc_server.h"

namespace svc_file {
    
    void FileServer::start() {
        _server->RunUntilAskedToQuit();
    }
    FileServer::FileServer(const bitesvc::SvcProvider::ptr &provider, 
        const FileRemoveMQ::ptr &file_remove_mq,
        const std::shared_ptr<brpc::Server> &server)
        : _provider(provider)
        , _file_remove_mq(file_remove_mq)
        , _server(server) { }

    
    FileServerBuilder &FileServerBuilder::withListenPort(unsigned int listen_port) {
        _listen_port = listen_port;
        return *this;
    }
    FileServerBuilder &FileServerBuilder::withMqUrl(const std::string &mq_url) {
        _mq_url = mq_url;
        return *this;
    }
    FileServerBuilder &FileServerBuilder::withRegistrySettings(const registry_settings &reg_settings) {
        _reg_settings = reg_settings;
        return *this;
    }
    FileServerBuilder &FileServerBuilder::withMysqlSettings(const biteodb::mysql_settings &mysql_settings) {
        _mysql_settings = mysql_settings;
        return *this;
    }
    FileServerBuilder &FileServerBuilder::withRedisSettings(const biteredis::redis_settings &redis_settings) {
        _redis_settings = redis_settings;
        return *this;
    }
    FileServerBuilder &FileServerBuilder::withMqDelFileSettings(const std::string &declare_settings) {
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
    FileServerBuilder &FileServerBuilder::withMqDelCacheSettings(const std::string &declare_settings) {
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
    FileServerBuilder &FileServerBuilder::withFdfsSettings(const bitefdfs::fdfs_settings &fdfs_settings) {
        _fdfs_settings = fdfs_settings;
        return *this;
    }
    FileServer::ptr FileServerBuilder::build() {
        auto mysql = biteodb::DBFactory::mysql(_mysql_settings);
        auto redis = biteredis::RedisFactory::create(_redis_settings);
        //1. 基于mq_url构造MQClient对象
        auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(_mq_url);
        auto del_cache = std::make_shared<svc_file::CacheDelete>(redis, mq_client, _del_cache_settings);
        //2. 基于mysql_settings & redis_settings & mq_client 构造SvcData对象
        auto svc_data = std::make_shared<SvcData>(redis, mysql, del_cache);
        //3. 基于fdfs_settings初始化FASTDFS客户端全局配置
        bitefdfs::FDFSClient::init(_fdfs_settings);
        //4. 基于SvcData & mq_settings构造FileRemoveMQ对象
        auto file_remove_mq = std::make_shared<FileRemoveMQ>(svc_data, mq_client, _del_file_settings);
        //5. 基于reg_settings构造SvcProvider对象
        auto provider = std::make_shared<bitesvc::SvcProvider>(_reg_settings.registry_center_addr, 
            _reg_settings.service_name, _reg_settings.service_addr);
        //6. 基于SvcData 构造FileServiceImpl对象
        auto file_service_impl = new FileServiceImpl(svc_data);
        //7. 基于FileServiceImpl构造brpc::Server对象
        auto server = std::make_shared<brpc::Server>();
        int ret = server->AddService(file_service_impl, brpc::SERVER_OWNS_SERVICE);
        if (ret == -1) {
            ERR("添加FileServiceImpl失败");
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
        //8. 基于SvcProvider & FileRemoveMQ & brpc::Server 构造FileServer对象
        auto file_server = std::make_shared<FileServer>(provider, file_remove_mq, server);
        return file_server;
    }
}