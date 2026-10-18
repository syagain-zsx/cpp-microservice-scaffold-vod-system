#pragma once
#include "svc_data.h"
#include <JianLiuScaffold/util.h>
#include <JianLiuScaffold/mq.h>
#include <JianLiuScaffold/log.h>
#include <JianLiuScaffold/hls.h>
#include <JianLiuScaffold/fdfs.h>
namespace svc_file {

    class FileRemoveMQ {
        public:
            using ptr = std::shared_ptr<FileRemoveMQ>;
            // 实例化类成员，在实例化_subscriber的时候需要先构造对象，声明交换机和队列，订阅队列消息
            FileRemoveMQ(const SvcData::ptr &svc_data, 
                const JianLiuMq::MQClient::ptr &mq_client,
                const JianLiuMq::declare_settings &settings);
        private:
            //订阅队列消息的回调处理函数：接收消息获取文件ID，删除文件数据和文件元信息
            void callback(const char *body, size_t len); 
            void remove_subfile(const std::string &path);
        private:
            //业务层数据操作对象：获取/删除/新增文件元数据
            SvcData::ptr _svc_data;
            //消息队列订阅者对象：订阅文件删除消息进行处理
            JianLiuMq::Subscriber::ptr _subscriber;//消息订阅者对象
    };
}