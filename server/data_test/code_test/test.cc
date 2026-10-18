#include <verify_code.h>

int main()
{
    //初始化：日志模块，redis操作句柄
    bitelog::bitelog_init();
    biteredis::redis_settings rsettings = {
        .host = "192.168.65.128",
        .passwd = "123456",
        .connection_pool_size = 5
    };
    auto redis = biteredis::RedisFactory::create(rsettings);
    {
        //新增验证码缓存
        try {
            auto rtx = redis->transaction(false, false);
            vp_data::VerifyCodeData cache_vcd(rtx);
            cache_vcd.insert("code_id_1", "session_id_1", "1357");
        }catch (const vp_error::VPException &e) {
            std::cout << e.what() << std::endl;
        }
    }
    {
        //获取验证码缓存
        try {
            auto rtx = redis->transaction(false, false);
            vp_data::VerifyCodeData cache_vcd(rtx);
            auto code = cache_vcd.get("code_id_1");
            if (!code) {
                std::cout << "没找到验证码\n";
                return -1;
            }
            std::cout << code->session_id << std::endl;
            std::cout << code->code_id << std::endl;
            std::cout << code->code << std::endl;
        }catch (const vp_error::VPException &e) {
            std::cout << e.what() << std::endl;
        }
    }
    {
        //删除验证码缓存
        try {
            auto rtx = redis->transaction(false, false);
            vp_data::VerifyCodeData cache_vcd(rtx);
            cache_vcd.remove("code_id_1");
        }catch (const vp_error::VPException &e) {
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}