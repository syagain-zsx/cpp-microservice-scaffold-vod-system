#include <session.h>

int main()
{
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
    try {
        //5. 实例化会话数据操作对象
        //6. 进行功能测试
        // {
        //     odb::transaction mtx(mysql->begin());
        //     auto &m = mtx.database();
        //     auto rtx = redis->transaction(false, false);
        //     auto r = rtx.redis();
        //     vp_data::SessionData tbl_session(m, r, sync);
        //     // 新增会话数据（看看数据库中数据是否新增正常）
        //     vp_data::Session ss("session_id_1");
        //     tbl_session.insert(ss);
        //     mtx.commit(); // 提交数据库事务，否则会回滚操作
        // }
        // {
        //     odb::transaction mtx(mysql->begin());
        //     auto &m = mtx.database();
        //     auto rtx = redis->transaction(false, false);
        //     auto r = rtx.redis();
        //     vp_data::SessionData tbl_session(m, r, sync);
        //     // 获取会话数据（先看获取的数据是否正确，再看缓存中是否已有数据）
        //     auto ss = tbl_session.selectBySid("session_id_1");
        //     if (ss) {
        //         std::cout << "session_id: " << ss->session_id() << std::endl;
        //         if (ss->user_id())std::cout << "user_id: " << *ss->user_id() << std::endl;
        //     }
        //     mtx.commit(); // 提交数据库事务，否则会回滚操作
        // }
        // {
        //     odb::transaction mtx(mysql->begin());
        //     auto &m = mtx.database();
        //     auto rtx = redis->transaction(false, false);
        //     auto r = rtx.redis();
        //     vp_data::SessionData tbl_session(m, r, sync);
        //     // 更新会话数据（设置会话用户ID，清空会话用户ID； 看看缓冲中的数据是否被删除，以及是否会收到延迟删除消息）
        //     auto ss = tbl_session.selectBySid("session_id_1");

        //     if (ss) {
        //         // ss->set_user_id(std::string("user_id_1"));
        //         ss->set_user_id(odb::nullable<std::string>());
        //         tbl_session.update(*ss);
        //     }
        //     mtx.commit();
        //     DBG("等待3s后，查看是否能收到延迟删除消息");
        // }
        // {
        //     odb::transaction mtx(mysql->begin());
        //     auto &m = mtx.database();
        //     auto rtx = redis->transaction(false, false);
        //     auto r = rtx.redis();
        //     vp_data::SessionData tbl_session(m, r, sync);
        //     // 删除会话数据（通过会话ID进行删除）
        //     tbl_session.removeBySid("session_id_1");
        //     mtx.commit();
        //     DBG("等待3s后，查看是否能收到延迟删除消息");
        // }
        {
            odb::transaction mtx(mysql->begin());
            auto &m = mtx.database();
            auto rtx = redis->transaction(false, false);
            auto r = rtx.redis();
            vp_data::SessionData tbl_session(m, r, sync);
            // 删除会话数据（通过用户ID进行删除）
            auto ss1 = tbl_session.selectBySid("session_id_1");
            auto ss2 = tbl_session.selectBySid("session_id_2");
            tbl_session.removeByUid("user_id_1");
            mtx.commit();
            DBG("等待3s后，查看是否能收到延迟删除消息");
        }
    }catch(const std::exception &e){
        std::cerr << e.what() << '\n';
    }
    std::cout << "回车退出程序\n";
    getchar();
    return 0;
}