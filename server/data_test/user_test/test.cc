#include <user.h>

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
    auto sync = vp_data::RemoveCacheFactory::create(mq_client, redis);

    // {
    //     //测试当一个邮箱没有被注册的时候，注册用户，测试的时候测试两次：1-邮箱未注册过；2-邮箱已经注册过
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         // mtx.tracer(odb::stderr_full_tracer);
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         bool ret = tbl_ud.isExist("bite@qq.com");
    //         if (ret == false) {
    //             tbl_ud.insert("user_id_1", "bite@qq.com");
    //         }else {
    //             std::cout << "bite@qq.com 已经注册！\n";
    //         }
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     // 两个测试：1-修改密码； 2-用户名密码检测
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         tbl_ud.updatePasswd("user_id_1", "123456");
    //         tbl_ud.updateNickname("user_id_1", "zhangsan");
    //         tbl_ud.updateAvatar("user_id_1", "avatar_fid_1");
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         std::cout << tbl_ud.verify("zhangsan", "123456") << std::endl;
    //         std::cout << tbl_ud.verify("bite@qq.com", "123456") << std::endl;
    //         std::cout << tbl_ud.verify("bite@qq.com", "456789") << std::endl;
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         auto user = tbl_ud.getUser("user_id_1");
    //         if (user) {
    //             std::cout << user->user_id() << std::endl;
    //             std::cout << user->nickname() << std::endl;
    //             std::cout << user->email() << std::endl;
    //             std::cout << (int)user->status() << std::endl;
    //             if (user->bakname()) std::cout << user->bakname().get() << std::endl;
    //             if (user->phone()) std::cout << user->phone().get() << std::endl;
    //             if (user->avatar()) std::cout << user->avatar().get() << std::endl;
    //             if (user->remark()) std::cout << user->remark().get() << std::endl;
    //             std::cout << boost::posix_time::to_simple_string(user->reg_time()) << std::endl;
    //         }else {
    //             std::cout << "未获取到用户信息: " << "user_id_1" << std::endl;
    //         }
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         std::unordered_set<std::string> id_list = {
    //             "user_id_1",
    //             "user_id_2",
    //             "user_id_3"
    //         };
    //         auto users = tbl_ud.getListById(id_list);
    //         for (auto &user : users) {
    //             if (user) {
    //                 std::cout << user->user_id() << std::endl;
    //                 std::cout << user->nickname() << std::endl;
    //                 std::cout << user->email() << std::endl;
    //                 std::cout << (int)user->status() << std::endl;
    //                 if (user->bakname()) std::cout << user->bakname().get() << std::endl;
    //                 if (user->phone()) std::cout << user->phone().get() << std::endl;
    //                 if (user->avatar()) std::cout << user->avatar().get() << std::endl;
    //                 if (user->remark()) std::cout << user->remark().get() << std::endl;
    //                 std::cout << boost::posix_time::to_simple_string(user->reg_time()) << std::endl;
    //             }else {
    //                 std::cout << "未获取到用户信息: " << "user_id_1" << std::endl;
    //             }
    //             std::cout << "---------------------------------------\n";
    //         }
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         auto users = tbl_ud.getIdentifyUserList(vp_data::UserStatus::ENABLE, vp_data::IdentifyType::ADMIN, 1, 3);
    //         std::cout << "管理员总量：" << users->total << std::endl;
    //         for (auto &user : users->users) {
    //             if (user) {
    //                 std::cout << user->user_id() << std::endl;
    //                 std::cout << user->nickname() << std::endl;
    //                 std::cout << user->email() << std::endl;
    //                 std::cout << (int)user->status() << std::endl;
    //                 if (user->bakname()) std::cout << user->bakname().get() << std::endl;
    //                 if (user->phone()) std::cout << user->phone().get() << std::endl;
    //                 if (user->avatar()) std::cout << user->avatar().get() << std::endl;
    //                 if (user->remark()) std::cout << user->remark().get() << std::endl;
    //                 std::cout << boost::posix_time::to_simple_string(user->reg_time()) << std::endl;
    //             }else {
    //                 std::cout << "未获取到用户信息: " << "user_id_1" << std::endl;
    //             }
    //             std::cout << "---------------------------------------\n";
    //         }
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     //更新用户状态
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         tbl_ud.updateStatus("user_id_2", vp_data::UserStatus::DISABLE);
    //         tbl_ud.updateStatus("user_id_4", vp_data::UserStatus::DISABLE);
    //         tbl_ud.updateStatus("user_id_6", vp_data::UserStatus::DISABLE);
    //         tbl_ud.updateStatus("user_id_7", vp_data::UserStatus::DISABLE);
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     //判断用户状态
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         std::cout << tbl_ud.isEnable("user_id_1") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_2") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_3") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_4") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_5") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_6") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_7") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_8") << std::endl;
    //         std::cout << tbl_ud.isEnable("user_id_9") << std::endl;
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    // {
    //     //判断用户状态
    //     try {
    //         odb::transaction mtx(mysql->begin());
    //         auto rtx = redis->transaction(false, false);
    //         vp_data::UserData tbl_ud(mtx, rtx, sync);
    //         tbl_ud.updateAdmin("user_id_2", "lisisi", "我是李四四", vp_data::UserStatus::DISABLE);
    //         mtx.commit();
    //     }catch (const vp_error::VPException &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }
    {
        //通过邮箱获取用户信息
        try {
            odb::transaction mtx(mysql->begin());
            auto rtx = redis->transaction(false, false);
            vp_data::UserData tbl_ud(mtx, rtx, sync);
            auto user = tbl_ud.getUserByEmail("333@qq.com");
            if (user) {
                std::cout << user->user_id() << std::endl;
                std::cout << user->nickname() << std::endl;
                std::cout << user->email() << std::endl;
                std::cout << (int)user->status() << std::endl;
                if (user->bakname()) std::cout << user->bakname().get() << std::endl;
                if (user->phone()) std::cout << user->phone().get() << std::endl;
                if (user->avatar()) std::cout << user->avatar().get() << std::endl;
                if (user->remark()) std::cout << user->remark().get() << std::endl;
                std::cout << boost::posix_time::to_simple_string(user->reg_time()) << std::endl;
            }else {
                std::cout << "未获取到用户信息: " << "user_id_1" << std::endl;
            }
            mtx.commit();
        }catch (const vp_error::VPException &e) {
            std::cout << e.what() << std::endl;
        }
    }
    getchar();
    return 0;
}