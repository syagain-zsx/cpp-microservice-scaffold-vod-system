#include <permission_role.h>

int main()
{
    // 1. 初始化所有基础操作句柄-log，myql，redis，sync
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
    // 2. 开始功能测试
    // {
    //     odb::transaction mtx(mysql->begin());
    //     auto &m = mtx.database();
    //     auto rtx = redis->transaction(false, false);
    //     auto r = rtx.redis();
    //     vp_data::PermissionRoleData tbl_prd(m, r, sync);
    //     //1. 新增权限角色关系（查看数据库是否正常新增）
    //     std::vector<vp_data::RoleType> roles = {
    //         vp_data::RoleType::ADMIN_SUPER, 
    //         vp_data::RoleType::ADMIN_NORMAL,
    //         vp_data::RoleType::USER_NORNAL
    //     };
    //     tbl_prd.insert("/HttpService/getCode", vp_data::RoleType::UNKNOWN);
    //     tbl_prd.insert("/HttpService/vcodeLogin", vp_data::RoleType::USER_GUEST);
    //     tbl_prd.insert("/HttpService/logout", roles);
    //     mtx.commit();
    // }
    {
        odb::transaction mtx(mysql->begin());
        auto &m = mtx.database();
        auto rtx = redis->transaction(false, false);
        auto r = rtx.redis();
        vp_data::PermissionRoleData tbl_prd(m, r, sync);
        //2. 获取指定url所有的角色（查看缓存是否正常设置，获取的数据是否与数据库一致）
        auto roles = tbl_prd.list("/HttpService/logout");
        for (auto &r : roles) {
            std::cout << (int)r << std::endl;
        }
        mtx.commit();
    }
    {
        odb::transaction mtx(mysql->begin());
        auto &m = mtx.database();
        auto rtx = redis->transaction(false, false);
        auto r = rtx.redis();
        vp_data::PermissionRoleData tbl_prd(m, r, sync);
        //3. 删除指定url的所有角色（查看缓存是否正常删除，数据库是否正常删除）
        // tbl_prd.remove("/HttpService/logout", vp_data::RoleType::ADMIN_SUPER);
        tbl_prd.remove("/HttpService/logout");
        mtx.commit();
    }
    getchar();
    return 0;
}