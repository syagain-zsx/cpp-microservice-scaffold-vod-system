#include <user_identify_role.h>

int main()
{
    // 1. 初始化构造各项数据操作句柄（数据库操作句柄，缓存操作句柄，缓存同步操作句柄）
    bitelog::bitelog_init();
    //1. 实例化消息队列客户端对象
    const std::string url = "amqp://admin:123456@192.168.65.128:5672/";
    auto mq_client = bitemq::MQFactory::create<bitemq::MQClient>(url);
    //2. 实例化数据库操作对象
    biteodb::mysql_settings msettings = {
        .host = "192.168.65.128",
        .passwd = "123456",
        .db = "vptest",
        .connection_pool_size = 5
    };
    auto mysql = biteodb::DBFactory::mysql(msettings);
    //3. 实例化缓存操作对象
    biteredis::redis_settings rsettings = {
        .host = "192.168.65.128",
        .passwd = "123456",
        .connection_pool_size = 5
    };
    auto redis = biteredis::RedisFactory::create(rsettings);
    //4. 实例化缓存同步删除对象
    auto sync = vp_data::RemoveCacheFactory::create(mq_client, redis);
    // 2. 开始测试
    // {
    //     // 0. 构造用户身份角色操作对象
    //     odb::transaction mtx(mysql->begin());
    //     auto &m = mtx.database();
    //     auto rtx = redis->transaction(false, false);
    //     auto r = rtx.redis();
    //     vp_data::UserIdentifyRoleData uir_data(m, r, sync);
    //     // 1. 新增用户身份角色关系（向数据库新增）
    //     uir_data.insert("user_id_1", vp_data::IdentifyType::ADMIN, vp_data::RoleType::ADMIN_NORMAL);
    //     uir_data.insert("user_id_1", vp_data::IdentifyType::NORMAL, vp_data::RoleType::USER_NORMAL);
    //     mtx.commit();
    // }
    // {
    //     odb::transaction mtx(mysql->begin());
    //     auto &m = mtx.database();
    //     auto rtx = redis->transaction(false, false);
    //     auto r = rtx.redis();
    //     vp_data::UserIdentifyRoleData uir_data(m, r, sync);
    //     // 2. 获取用户身份角色关系（对比获取到的数据与数据库是否一致，以及查看缓存是否已添加数据）
    //     auto uris_list = uir_data.list("user_id_1");
    //     std::cout << "list size: " << uris_list.size() << std::endl;
    //     for (auto &uri : uris_list) {
    //         std::cout << "identify_type: " << (int)uri.identify_type() << ", role_type: " << (int)uri.role_type() << std::endl;
    //     }
    //     mtx.commit();
    // }
    // {
    //     odb::transaction mtx(mysql->begin());
    //     auto &m = mtx.database();
    //     auto rtx = redis->transaction(false, false);
    //     auto r = rtx.redis();
    //     vp_data::UserIdentifyRoleData uir_data(m, r, sync);
    //     //3. 判断指定用户是否含有某个身份/角色
    //     bool ret1 = uir_data.hasIdentify("user_id_1", vp_data::IdentifyType::ADMIN);
    //     bool ret2 = uir_data.hasIdentify("user_id_1", vp_data::IdentifyType::NORMAL);
    //     bool ret3 = uir_data.hasRole("user_id_1", vp_data::RoleType::ADMIN_SUPER);
    //     bool ret4 = uir_data.hasRole("user_id_1", vp_data::RoleType::ADMIN_NORMAL);
    //     bool ret5 = uir_data.hasRole("user_id_1", vp_data::RoleType::USER_NORMAL);
    //     bool ret6 = uir_data.hasRole("user_id_1", vp_data::RoleType::USER_GUEST);
    //     mtx.commit();
    //     std::cout << "hasIdentify: " << ret1 << ret2 << std::endl;
    //     std::cout << "hasRole: " << ret3 << ret4 << ret5 << ret6 << std::endl;
    // }
    {
        odb::transaction mtx(mysql->begin());
        auto &m = mtx.database();
        auto rtx = redis->transaction(false, false);
        auto r = rtx.redis();
        vp_data::UserIdentifyRoleData uir_data(m, r, sync);
        //4. 删除用户身份角色关系（查看是否数据库与缓存数据都被删除，并且是否收到延迟同步消息）
        uir_data.remove("user_id_1", vp_data::IdentifyType::ADMIN, vp_data::RoleType::UNKNOWN);
        mtx.commit();
    }
    std::cout << "回车后程序退出...\n";
    getchar();
    return 0;
}