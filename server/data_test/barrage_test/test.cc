#include "barrage.h"

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
    //     odb::transaction mtx(mysql->begin());
    //     //新增弹幕
    //     vp_data::BarrageData bd(mtx);
    //     vp_data::Barrage b1("barrage_id_1", "video_1", "user_id_1", "content-1", 22);
    //     vp_data::Barrage b2("barrage_id_2", "video_1", "user_id_2", "content-2", 11);
    //     vp_data::Barrage b3("barrage_id_3", "video_1", "user_id_3", "content-3", 33);
    //     vp_data::Barrage b4("barrage_id_4", "video_2", "user_id_1", "content-4", 5);
    //     vp_data::Barrage b5("barrage_id_5", "video_2", "user_id_2", "content-5", 7);
    //     vp_data::Barrage b6("barrage_id_6", "video_2", "user_id_3", "content-6", 9);
    //     bd.insert(b1);
    //     bd.insert(b2);
    //     bd.insert(b3);
    //     bd.insert(b4);
    //     bd.insert(b5);
    //     bd.insert(b6);
    //     mtx.commit();
    // }
    {
        //获取视频弹幕列表
        odb::transaction mtx(mysql->begin());
        vp_data::BarrageData bd(mtx);
        auto blist = bd.list("video_1");
        mtx.commit();
        for (auto &b : blist) {
            std::cout << "barrage_id: " << b->barrage_id() << std::endl;
            std::cout << "video_id: " << b->video_id() << std::endl;
            std::cout << "user_id: " << b->user_id() << std::endl;
            std::cout << "content: " << b->content() << std::endl;
            std::cout << "time: " << b->time() << std::endl;
            std::cout << "----------------------------------------\n";
        }
    }
    {
        //删除视频弹幕
        odb::transaction mtx(mysql->begin());
        vp_data::BarrageData bd(mtx);
        bd.remove("video_2");
        mtx.commit();
    }
    return 0;
}