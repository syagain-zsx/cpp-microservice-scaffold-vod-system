#include <video.h>
#include <cache_sync.h>

#include <message.pb.h>

int main()
{
    //初始化：日志模块，odb句柄，redis句柄，sync对象
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
    // {
    //     //新增视频元信息（查看数据库是否成功插入了数据）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::VideoData video_data(mtx, rtx, sync);
    //     vp_data::Video video;
    //     video.set_video_id("video_7");
    //     video.set_title("七号视频标题");
    //     video.set_summary(std::string("七号视频简介"));
    //     video.set_cover_fid("cover_7");
    //     video.set_video_fid("video_7");
    //     video.set_uploader_uid("user_id_1");
    //     video.set_size(1024);
    //     video.set_duration(10);
    //     video_data.insert(video);
    //     mtx.commit();
    // }
    // {
    //     //获取视频元信息（针对数据库进行数据对比，并且查看缓存是否成功添加数据）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::VideoData video_data(mtx, rtx, sync);
    //     auto video = video_data.get("video_7");
    //     mtx.commit();
    //     if (!video) return -1;
    //     std::cout << "video_id: " << video->video_id() << std::endl;
    //     std::cout << "title: " << video->title() << std::endl;
    //     std::cout << "summary: " << video->summary().get() << std::endl;
    //     std::cout << "cover_fid: " << video->cover_fid() << std::endl;
    //     std::cout << "video_fid: " << video->video_fid() << std::endl;
    //     std::cout << "uploader_uid: " << video->uploader_uid() << std::endl;
    //     std::cout << "size: " << video->size() << std::endl;
    //     std::cout << "duration: " << video->duration() << std::endl;
    //     std::cout << "upload_time:" << boost::posix_time::to_simple_string(video->upload_time()) << std::endl;
    // }
    // {
    //     //删除视频元信息（查看数据库是否删除了数据，以及缓存是否删除了数据，以及是否会收到延迟删除同步消息）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::VideoData video_data(mtx, rtx, sync);
    //     auto video = video_data.remove("video_7");
    //     mtx.commit();
    //     if (!video) return -1;
    //     std::cout << "video_id: " << video->video_id() << std::endl;
    //     std::cout << "title: " << video->title() << std::endl;
    //     std::cout << "summary: " << video->summary().get() << std::endl;
    //     std::cout << "cover_fid: " << video->cover_fid() << std::endl;
    //     std::cout << "video_fid: " << video->video_fid() << std::endl;
    //     std::cout << "uploader_uid: " << video->uploader_uid() << std::endl;
    //     std::cout << "size: " << video->size() << std::endl;
    //     std::cout << "duration: " << video->duration() << std::endl;
    //     std::cout << "upload_time:" << boost::posix_time::to_simple_string(video->upload_time()) << std::endl;
    // }
    // {
    //     //更新视频元信息（从数据库获取，然后修改更新， 查看缓存数据是否被删除，查看数据库数据是否被更新）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::VideoData video_data(mtx, rtx, sync);
    //     auto video = video_data.getFromDB("video_1");
    //     video->set_title("一号视频标题");
    //     video->set_auditor_uid(std::string("user_id_8"));
    //     video->set_status(vp_data::VideoStatus::OFFLINE);
    //     video->set_play_count(video->play_count() + 100);
    //     video_data.update(video);
    //     mtx.commit();
    // }
    {
        //通过ID列表获取视频元信息（对比获取到的数据是否一致即可）
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::VideoData video_data(mtx, rtx, sync);
        std::vector<std::string> ids = {"video_1", "video_2", "video_3", "video_4", "video_5", "video_6"};
        auto videos = video_data.list(ids);
        mtx.commit();
        if (videos.empty()) return -1;
        for (auto &video_it : videos) {
            std::cout << "------------video_id:" << video_it.first << std::endl;
            auto video = video_it.second;
            std::cout << "video_id: " << video->video_id() << std::endl;
            std::cout << "title: " << video->title() << std::endl;
            if (video->summary()) std::cout << "summary: " << video->summary().get() << std::endl;
            std::cout << "cover_fid: " << video->cover_fid() << std::endl;
            std::cout << "video_fid: " << video->video_fid() << std::endl;
            std::cout << "uploader_uid: " << video->uploader_uid() << std::endl;
            std::cout << "size: " << video->size() << std::endl;
            std::cout << "duration: " << video->duration() << std::endl;
            std::cout << "upload_time:" << boost::posix_time::to_simple_string(video->upload_time()) << std::endl;
            std::cout << "play_count: " << video->play_count() << std::endl;
            std::cout << "status: " << (int)video->status() << std::endl;
            if (video->auditor_uid()) std::cout << "auditor_uid: " << video->auditor_uid().get() << std::endl;
            std::cout << "---------------------------------------\n";
        }
    }
    getchar();
    return 0;
}