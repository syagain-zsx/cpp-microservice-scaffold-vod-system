#include <video_catgory_tag.h>
#include <cache_sync.h>

int main()
{
    JianLiuLog::init();
    JianLiuOdb::mysql_settings msettings = {
        .host = "192.168.65.128",
        .passwd = "123456",
        .db = "vptest",
        .connection_pool_size = 5
    };
    auto mysql = JianLiuOdb::DBFactory::mysql(msettings);
    // {
    //     //新增视频分类标签
    //     odb::transaction mtx(mysql->begin());
    //     vp_data::VideoCatgoryTagData vct(mtx);
    //     vct.insert("video_1", 1, 1);
    //     vct.insert("video_1", 1, 2);
    //     vct.insert("video_1", 1, 3);
    //     vct.insert("video_2", 1, 1);
    //     vct.insert("video_2", 1, 2);
    //     vct.insert("video_3", 1, 3);
    //     mtx.commit();
    // }
    // {
    //     // 获取视频分类标签信息
    //     odb::transaction mtx(mysql->begin());
    //     vp_data::VideoCatgoryTagData vct(mtx);
    //     unsigned int cid = 0;
    //     std::vector<unsigned int> tags;
    //     vct.getCatgoryTags("video_1", cid, tags);
    //     std::cout << "catgory: " << cid << std::endl;
    //     for(auto &tag : tags) {
    //         std::cout << "tag: " << tag << std::endl;
    //     }
    //     mtx.commit();
    // }
    {
        // 删除视频分类标签
        odb::transaction mtx(mysql->begin());
        vp_data::VideoCatgoryTagData vct(mtx);
        vct.remove("video_2");
        mtx.commit();
    }
    return 0;
}