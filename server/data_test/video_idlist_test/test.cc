#include <video_idlist.h>
#include <message.pb.h>

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
            std::cout << "收到缓存同步的延迟消息:" << key << std::endl;
        }
    };
    vp_data::CacheSync::ptr sync = std::make_shared<vp_data::CacheSync>(mq_client, mqsettings, cb);

    {
        // 获取主页视频列表测试
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoIdListData vild(mtx, rtx, sync);
        auto list = vild.getMainPageList(0, 5);
        mtx.commit();
        for (auto &i : list) {
            std::cout << i << std::endl;
        }
        std::cout << "main page test over---------------------\n";
    }
    {
        // 获取用户视频列表测试
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoIdListData vild(mtx, rtx, sync);
        auto list = vild.getUserPageList("user_id_1", 0, 5);
        mtx.commit();
        for (auto &i : list) {
            std::cout << i << std::endl;
        }
        std::cout << "user page test over---------------------\n";
    }
    {
        // 获取用户视频列表测试
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoIdListData vild(mtx, rtx, sync);
        auto list = vild.getCatgoryPageList(1, 0, 5);
        mtx.commit();
        for (auto &i : list) {
            std::cout << i << std::endl;
        }
        std::cout << "catgory page test over---------------------\n";
    }
    {
        // 获取用户视频列表测试
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoIdListData vild(mtx, rtx, sync);
        auto list = vild.getTagPageList(3, 0, 5);
        mtx.commit();
        for (auto &i : list) {
            std::cout << i << std::endl;
        }
        std::cout << "tag page test over---------------------\n";
    }
    {
        // 获取用户视频列表测试
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoIdListData vild(mtx, rtx, sync);
        auto list = vild.getStatusPageList(vp_data::VideoStatus::ONLINE, 0, 5);
        mtx.commit();
        std::cout << "total:" << list->total << std::endl;
        for (auto &i : list->videos) {
            std::cout << i->video_id() << std::endl;
            std::cout << i->uploader_uid() << std::endl;
        }
        std::cout << "status page test over---------------------\n";
    }
    {
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoIdListData vild(mtx, rtx, sync);
        vild.syncPageListCache("video_5", "user_id_1", 1, {3,4});
        mtx.commit();
    }
    getchar();
    return 0;
}