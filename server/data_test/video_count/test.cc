#include <video_count.h>
#include <message.pb.h>


vp_data::CacheSync::ptr create_del_sync(bitemq::MQClient::ptr &mq_client,
    const std::shared_ptr<sw::redis::Redis> &redis)  {
    bitemq::declare_settings mqsettings = {
        .exchange = "delete_cache_exchange",
        .exchange_type = "delayed",
        .queue = "delete_cache_queue",
        .binding_key = "delete_cache_queue",
        .delayed_ttl = 3000
    };

    auto cb = [redis](const char* body, size_t len) {
        api_message::DeleteCacheMsg message;
        if(!message.ParseFromArray(body, len)) {
            std::cout << "队列消息反序列化失败！\n";
            return;
        }
        int sz = message.key_size();
        for (int i = 0; i < sz; i++) {
            std::string key = message.key(i);
            redis->del(key);
            std::cout << "收到缓存同步删除的延迟消息:" << key << std::endl;
        }
    };
    vp_data::CacheSync::ptr sync = std::make_shared<vp_data::CacheSync>(mq_client, mqsettings, cb);
    return sync;
}

vp_data::CacheSync::ptr create_cache_sync(bitemq::MQClient::ptr &mq_client, 
    const std::shared_ptr<odb::database> &mysql,
    const std::shared_ptr<sw::redis::Redis> &redis,
    const vp_data::CacheSync::ptr& delete_sync)  {
    bitemq::declare_settings mqsettings = {
        .exchange = "cache_sync_to_db_exchange",
        .exchange_type = "delayed",
        .queue = "cache_sync_to_db_queue",
        .binding_key = "cache_sync_to_db_queue",
        .delayed_ttl = 300000
    };

    auto cb = [=](const char* body, size_t len) {
        api_message::CacheToDBMsg message;
        if(!message.ParseFromArray(body, len)) {
            std::cout << "队列消息反序列化失败！\n";
            return;
        }
        int sz = message.video_id_size();
        for (int i = 0; i < sz; i++) {
            std::string video_id = message.video_id(i);
            DBG("收到缓存同步到数据库的延迟消息: {}", video_id);
            vp_data::VideoCountData::syncCacheToDB(video_id, mysql, redis, delete_sync);
        }
    };
    vp_data::CacheSync::ptr sync = std::make_shared<vp_data::CacheSync>(mq_client, mqsettings, cb);
    return sync;
}

int main()
{
    bitelog::bitelog_init();
    biteodb::mysql_settings msettings = {
        .host = "192.168.65.128",
        .passwd = "123456",
        .db = "vptest",
        .connection_pool_size = 5
    };
    auto mysql = biteodb::DBFactory::mysql(msettings);
    biteredis::redis_settings rsettings = {
        .host = "192.168.65.128",
        .passwd = "123456",
        .connection_pool_size = 5
    };
    auto redis = biteredis::RedisFactory::create(rsettings);
    const std::string url = "amqp://admin:123456@192.168.65.128:5672/";
    auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(url);

    auto del_sync = create_del_sync(mq_client, redis);
    auto cache_sync = create_cache_sync(mq_client, mysql, redis, del_sync);

    {
        std::cout << "增加视频播放量开始\n";
        //增加视频播放量（查看缓存中是否已经有了播放量字段，并且对比数据库是否进行增加）
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoCountData vcd(mtx, rtx, cache_sync);
        // DBG("第一次更新播放量");
        vcd.updateVideoPlayCount("video_1", 1);
        // DBG("第二次更新播放量");
        vcd.updateVideoPlayCount("video_1", 1);
        // DBG("更新点赞量");
        vcd.updateVideoLikeCount("video_1", 1);
        mtx.commit();
        std::cout << "增加视频播放量完毕\n";
    }
    {
        //获取视频播放量（对比获取到的数据与缓存是否一致）
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoCountData vcd(mtx, rtx, cache_sync);
        auto count = vcd.getCount("video_1");
        mtx.commit();
        std::cout << "play count:" << count->play_count << std::endl; // 114
        std::cout << "like count:" << count->like_count << std::endl; // 5
        std::cout << "---------------------------------------------\n";
    }
    {
        //减少视频播放量 ()
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoCountData vcd(mtx, rtx, cache_sync);
        vcd.updateVideoPlayCount("video_1", -1); // 113
        vcd.updateVideoLikeCount("video_1", -1); // 4
        mtx.commit();
    }
    {
        //获取视频播放量（对比获取到的数据与缓存是否一致）
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoCountData vcd(mtx, rtx, cache_sync);
        auto count = vcd.getCount("video_1");
        mtx.commit();
        std::cout << "play count:" << count->play_count << std::endl;
        std::cout << "like count:" << count->like_count << std::endl;
    }
    //过上五分钟之后，查看数据库中的数据是否从缓存进行了同步
    //以及到时候，看看是否会收到3s的延迟同步删除消息

    getchar();
    return 0;
}