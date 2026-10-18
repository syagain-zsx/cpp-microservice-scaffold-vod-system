#include "svc_sync.h"
#include <message.pb.h>

namespace svc_user {
    CacheDelete::CacheDelete(const std::shared_ptr<sw::redis::Redis> &redis, 
        const bitemq::MQClient::ptr &mq_client, 
        const bitemq::declare_settings &mqsettings): _redis(redis) {
        _publisher = std::make_shared<bitemq::Publisher>(mq_client, mqsettings);
        _subscriber = std::make_shared<bitemq::Subscriber>(mq_client, mqsettings);
        _subscriber->consume(std::bind(&CacheDelete::callback, this, std::placeholders::_1, std::placeholders::_2));
    }
    //2. 缓存同步接口，传入缓存key，内部发布删除消息
    void CacheDelete::sync(const std::string &key) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto r = rtx.redis();
            r.del(key);
            api_message::DeleteCacheMsg msg;
            msg.add_key(key);
            bool ret = _publisher->publish(msg.SerializeAsString());
            if (ret == false) {
                throw vp_error::VPException(vp_error::VpError::MQ_OP_FAILED);
            }
        }catch(const vp_error::VPException &e) {
            ERR("发布缓存删除延迟消息失败: {}", e.what());
            throw;
        }catch(const std::exception &e) {
            ERR("发布缓存删除延迟消息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
    }

    //3. 消息订阅回调函数，收到消息后，取出key对缓存进行删除
    void CacheDelete::callback(const char *body, size_t len) {
        // 1. 反序列化消息
        api_message::DeleteCacheMsg msg;
        bool ret = msg.ParseFromArray(body, len);
        if (ret == false) {
            ERR("收到删除缓存延迟消息，但是消息反序列化失败！");
            return;
        }
        auto rtx = _redis->transaction(false, false);
        auto r = rtx.redis();
        int sz = msg.key_size();
        for (int i = 0; i < sz; ++i) {
            r.del(msg.key(i));
            DBG("收到删除缓存延迟消息，删除缓存key: {}", msg.key(i).c_str());
        }
    }


}
