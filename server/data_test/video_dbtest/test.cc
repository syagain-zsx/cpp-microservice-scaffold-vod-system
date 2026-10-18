#include <video.h>

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
    try {
        odb::transaction mtx(mysql->begin());
        vp_data::VideoDBData vdd(mtx);
        //1. 获取视频元信息
        auto video = vdd.get("E6n4Z3BFWFdAcW470002");
        std::cout << video->video_id() << std::endl;
        std::cout << video->title() << std::endl;
        std::cout << video->cover_fid() << std::endl;
        std::cout << video->video_fid() << std::endl;
        std::cout << video->uploader_uid() << std::endl;
        std::cout << video->play_count() << std::endl;
        //2. 设置视频元信息
        video->set_status(vp_data::VideoStatus::AUDITING);
        vdd.set(video);
        mtx.commit();
    }catch(const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}