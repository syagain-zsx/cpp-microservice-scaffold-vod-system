#include <follow.h>

int main()
{
    // 1. 初始化操作句柄：日志，数据库
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
    //     auto &m = mtx.database();
    //     vp_data::FollowData fd(m);
    //     fd.insert("user_id_1", "user_id_2");
    //     fd.insert("user_id_2", "user_id_1");
    //     fd.insert("user_id_1", "user_id_3");
    //     mtx.commit();
    // }
    {
        // 2. 判断关系
        odb::transaction mtx(mysql->begin());
        auto &m = mtx.database();
        vp_data::FollowData fd(m);
        std::cout << fd.judge("user_id_1", "user_id_2") << std::endl;
        std::cout << fd.judge("user_id_2", "user_id_1") << std::endl;
        std::cout << fd.judge("user_id_1", "user_id_3") << std::endl;
        std::cout << fd.judge("user_id_1", "user_id_4") << std::endl;
        mtx.commit();
    }
    {
        // 3. 删除关系
        odb::transaction mtx(mysql->begin());
        auto &m = mtx.database();
        vp_data::FollowData fd(m);
        fd.remove("user_id_1", "user_id_2");
        fd.remove("user_id_2", "user_id_1");
        mtx.commit();
    }
    return 0;
}