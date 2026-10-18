#pragma once
#include <JianLiuScaffold/etcd.h>
#include <JianLiuScaffold/rpc.h>
#include <JianLiuScaffold/util.h>
#include "svc_rpc.h"
#include "svc_data.h"
#include "svc_mq.h"
#include "svc_sync.h"

namespace svc_file {
    class FileServerBuilde;
    class FileServer {
        public:
            using ptr = std::shared_ptr<FileServer>;
            void start();
            FileServer(const JianLiuSvc::SvcProvider::ptr &provider, 
                const FileRemoveMQ::ptr &file_remove_mq,
                const std::shared_ptr<brpc::Server> &server);
        private:
            friend class FileServerBuilde;
            //1. 服务注册对象（需要一直保存，需要进行服务信息保活）
            JianLiuSvc::SvcProvider::ptr _provider;
            //2. brpc::Server对象（需要一直保存，需要接收客户端rpc请求进行处理）
            std::shared_ptr<brpc::Server> _server;
            //3. 消息处理对象FileRemoveMQ对象（需要一直保存，需要接收消息队列消息进行处理）
            FileRemoveMQ::ptr _file_remove_mq;
    };

    struct registry_settings {
        std::string registry_center_addr; //注册中心地址
        std::string service_name; //服务名称
        std::string service_addr; //服务地址
    };
    class FileServerBuilder {
        public:
            FileServerBuilder() = default;
            ~FileServerBuilder() = default;
            FileServerBuilder &withListenPort(unsigned int listen_port);
            FileServerBuilder &withMqUrl(const std::string &mq_url);
            FileServerBuilder &withRegistrySettings(const registry_settings &reg_settings);
            FileServerBuilder &withMysqlSettings(const JianLiuOdb::mysql_settings &mysql_settings);
            FileServerBuilder &withRedisSettings(const JianLiuRedis::redis_settings &redis_settings);
            FileServerBuilder &withMqDelFileSettings(const std::string &declare_settings);
            FileServerBuilder &withMqDelCacheSettings(const std::string &declare_settings);
            FileServerBuilder &withFdfsSettings(const JianLiuFdfs::fdfs_settings &fdfs_settings);
            FileServer::ptr build();
        private:
            unsigned int _listen_port;
            std::string _mq_url;
            registry_settings _reg_settings;
            JianLiuOdb::mysql_settings _mysql_settings;
            JianLiuRedis::redis_settings _redis_settings;
            JianLiuMq::declare_settings _del_file_settings;
            JianLiuMq::declare_settings _del_cache_settings;
            JianLiuFdfs::fdfs_settings _fdfs_settings;
    };
}