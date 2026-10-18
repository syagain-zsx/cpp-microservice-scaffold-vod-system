#include <bite_scaffold/redis.h>
#include <bite_scaffold/odb.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include <sync.h>
#include <error.h>
#include <message.pb.h>

namespace svc_video {
    class CacheDelete : public vp_sync::CacheSync {
        public:
            using ptr = std::shared_ptr<CacheDelete>;
            // 1. 通过mq_client对象，构造消息发布者和订阅者对象，以及通过redis对象初始redis操作对象
            //      1. 函数内部定于固定的缓存同步交换机和队列，并进行绑定，以及消息订阅
            CacheDelete(const std::shared_ptr<sw::redis::Redis> &redis, 
                const bitemq::MQClient::ptr &mq_client, 
                const bitemq::declare_settings &mqsettings);
            //2. 缓存同步接口，传入缓存key，内部发布删除消息
            virtual void sync(const std::string &key) override; 
        private:
            //3. 消息订阅回调函数，收到消息后，取出key对缓存进行删除
            void callback(const char *body, size_t len); 
        private:
            std::shared_ptr<sw::redis::Redis> _redis; //缓存操作对象
            bitemq::Publisher::ptr _publisher;  //消息发布者对象
            bitemq::Subscriber::ptr _subscriber;//消息订阅者对象
    };

    class CacheToDB : public vp_sync::CacheSync {
        public:
            using ptr = std::shared_ptr<CacheToDB>;
            CacheToDB(const std::shared_ptr<odb::database> &mysql,
                const std::shared_ptr<sw::redis::Redis> &redis,
                const CacheDelete::ptr &delete_cache,
                const bitemq::MQClient::ptr &mq_client, 
                const bitemq::declare_settings &mqsettings);
            virtual void sync(const std::string &key) override; 
        private:
            void callback(const char *body, size_t len); 
        private:
            std::shared_ptr<odb::database> _mysql;
            std::shared_ptr<sw::redis::Redis> _redis;
            CacheDelete::ptr _delete_cache;
            bitemq::Publisher::ptr _publisher;  //消息发布者对象
            bitemq::Subscriber::ptr _subscriber;//消息订阅者对象
    };
}