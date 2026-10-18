#include "svc_sync.h"
#include <video.h>
#include <video_count.h>

namespace svc_video {
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


    CacheToDB::CacheToDB(const std::shared_ptr<odb::database> &mysql,
        const std::shared_ptr<sw::redis::Redis> &redis,
        const CacheDelete::ptr &delete_cache,
        const bitemq::MQClient::ptr &mq_client, 
        const bitemq::declare_settings &mqsettings) 
        : _mysql(mysql)
        , _redis(redis)
        , _delete_cache(delete_cache) {
        _publisher = std::make_shared<bitemq::Publisher>(mq_client, mqsettings);
        _subscriber = std::make_shared<bitemq::Subscriber>(mq_client, mqsettings);
        _subscriber->consume(std::bind(&CacheToDB::callback, this, std::placeholders::_1, std::placeholders::_2));
    }
    void CacheToDB::sync(const std::string &key) {
        api_message::CacheToDBMsg msg;
        msg.add_video_id(key);
        bool ret = _publisher->publish(msg.SerializeAsString());
        if (ret == false) {
            throw vp_error::VPException(vp_error::VpError::MQ_OP_FAILED);
        }
        DBG("向队列发布消息：延迟将缓存播放量同步到数据库中: {}", key);
    }

    void CacheToDB::callback(const char *body, size_t len) {
        DBG("收到向数据库同步播放量的消息");
        api_message::CacheToDBMsg msg;
        bool ret = msg.ParseFromArray(body, len);
        if (ret == false) {
            ERR("收到删除缓存延迟消息，但是消息反序列化失败！");
            return;
        }
        DBG("消息反序列化成功");
        auto rtx = _redis->transaction(false, false);
        auto redis_temp = rtx.redis();
        for (int i = 0; i < msg.video_id_size(); i++) {
            std::string vid = msg.video_id(i);
            DBG("要同步的视频播放量：{}", vid);
            while(true) {
                try {
                    std::string key = vp_data::VideoCountCache::getCacheKey(vid);
                    // 1. 缓存统计数据监控
                    redis_temp.watch(key); //监控处理期间缓存中的统计数据不能被改变

                    odb::transaction mtx(_mysql->begin());
                    auto &mysql_temp = mtx.database();
                    //获取缓存中的视频统计统计
                    vp_data::VideoCountCache vcc(_redis);
                    auto count = vcc.getCountFromCache(vid);
                    if (!count) {
                        WRN("收到向数据库同步播放量消息，但是未找到视频统计缓存: {}", vid);
                        break;
                    }
                    DBG("从缓存获取视频统计数据完毕：{} - {}", vid, count->play_count);
                    //更新数据库统计数据
                    vp_data::VideoData vd(mtx, _redis, _delete_cache);
                    auto video = vd.getFromDB(vid);
                    if (!video) {
                        WRN("收到向数据库同步播放量消息，但是未找到视频元信息: {}", vid);
                        break;
                    }
                    video->set_play_count(count->play_count);
                    vd.update(video);
                    DBG("向数据库更新视频统计数据完毕：{} - {}", vid, count->play_count);
                    // 5. 删除缓存中的统计数据
                    rtx.del(key).exec(); //这里必须使用事务删除缓存中的统计数据 --事务执行期间如果缓存发生了改变就会抛异常重新处理
                    // 6. 发布延迟的同步删除消息（对缓存中的统计数据进行二次延迟删除）
                    _delete_cache->sync(key);
                    DBG("向数据库同步播放量: {} - {}", vid, count->play_count);
                    mtx.commit();
                    break; // 不要忘了跳出while循环
                }catch(const sw::redis::WatchError &e) {
                    //如果事务执行失败：原因可能会有很多，当前最可能的原因：key值被修改
                    ERR("同步播放量统计数据到数据库, 事务期间, key值监控异常: {}-{}", vid, e.what());
                }catch (const std::exception& e) {
                    ERR("同步播放量统计数据到数据库失败: {}-{}", vid, e.what());
                    return;
                }
            }
        }
    }
}
