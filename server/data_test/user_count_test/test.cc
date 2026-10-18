#include <user_count.h>

int main()
{
    // 1. 初始化各项操作句柄：日志，mysql，reids
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
    // 2. 开始测试
    // {
    //     //获取用户统计数据（输出数据进行对比，再看看缓存中统计数据是否添加了）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::UserCountData ucd(mtx, rtx);
    //     auto uc = ucd.getCount("user_id_1");
    //     std::cout << uc->followed_count << std::endl;
    //     std::cout << uc->follower_count << std::endl;
    //     std::cout << uc->vplay_count << std::endl;
    //     std::cout << uc->vlike_count << std::endl;
    //     mtx.commit();
    // }
    // {
    //     //更新用户视频播放量/点赞量/关注数/粉丝数（再看看缓存中统计数据是否更新了）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::UserCountData ucd(mtx, rtx);
    //     ucd.updatePlayCount("user_id_1", -1);
    //     ucd.updateLikeCount("user_id_1", -1);
    //     ucd.updateFollowerCount("user_id_1", -1);
    //     ucd.updateFollowedCount("user_id_1", -1);
    //     mtx.commit();
    // }
    // {
    //     //获取用户统计数据（输出数据进行对比，再看看缓存中统计数据是否添加了）
    //     odb::transaction mtx(mysql->begin());
    //     auto rtx = redis->transaction(false, false);
    //     vp_data::UserCountData ucd(mtx, rtx);
    //     auto uc = ucd.getCount("user_id_1");
    //     std::cout << uc->followed_count << std::endl;
    //     std::cout << uc->follower_count << std::endl;
    //     std::cout << uc->vplay_count << std::endl;
    //     std::cout << uc->vlike_count << std::endl;
    //     mtx.commit();
    // }
    {
        //缓存中没有数据的情况下， 更新各项缓存数据，查看缓存中是否添加了错误数据
        odb::transaction mtx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        vp_data::UserCountData ucd(mtx, rtx);
        ucd.updatePlayCount("user_id_1", -1);
        ucd.updateLikeCount("user_id_1", -1);
        ucd.updateFollowerCount("user_id_1", -1);
        ucd.updateFollowedCount("user_id_1", -1);
        mtx.commit();
    }
    return 0;
}