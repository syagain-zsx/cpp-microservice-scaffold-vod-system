#include <video_like.h>
#include <cache_sync.h>

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
    // {
    //     //新增视频点赞
    //     odb::transaction mtx(mysql->begin());
    //     vp_data::VideoLikeData vld(mtx);
    //     vld.insert("video_6", "user_id_1");
    //     vld.insert("video_6", "user_id_2");
    //     vld.insert("video_6", "user_id_4");
    //     mtx.commit();
    // }
    // {
    //     odb::transaction mtx(mysql->begin());
    //     vp_data::VideoLikeData vld(mtx);
    //     vld.cancel("video_6", "user_id_3");
    //     mtx.commit();
    // }
    // {
    //     odb::transaction mtx(mysql->begin());
    //     vp_data::VideoLikeData vld(mtx);
    //     std::cout << vld.judge("video_6", "user_id_1") << std::endl;
    //     std::cout << vld.judge("video_6", "user_id_2") << std::endl;
    //     std::cout << vld.judge("video_6", "user_id_3") << std::endl;
    //     std::cout << vld.judge("video_6", "user_id_4") << std::endl;
    //     mtx.commit();
    // }
    {
        odb::transaction mtx(mysql->begin());
        vp_data::VideoLikeData vld(mtx);
        vld.remove("video_6");
        mtx.commit();
    }
    return 0;
}