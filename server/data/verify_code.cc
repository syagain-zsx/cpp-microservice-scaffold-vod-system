#include "verify_code.h"


namespace vp_data{
    
    const size_t VerifyCodeData::_expire_time = 300;
    const std::string VerifyCodeData::_prefix = "verify_code_";
    const std::string VerifyCodeData::_key_ssid = "sessionId";
    const std::string VerifyCodeData::_key_cid = "codeId";
    const std::string VerifyCodeData::_key_code = "verifyCode";
    const std::string VerifyCodeData::_key_email = "email";
    VerifyCodeData::VerifyCodeData(const std::shared_ptr<sw::redis::Redis> &redis): _redis(redis) {}
    void VerifyCodeData::insert(const std::string &cid, const std::string &ssid, const std::string &code, const std::string &email) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 组织缓存key
            std::string key = getCacheKey(cid);
            //2. 根据数据对象，组织缓存操作map
            std::unordered_map<std::string, std::string> map;
            map[_key_ssid] = ssid;
            map[_key_cid] = cid;
            map[_key_code] = code;
            map[_key_email] = email;
            //3. 添加缓存
            redis.hmset(key, map.begin(), map.end());
            //4. 设置过期时间
            redis.expire(key, _expire_time + biteutil::Random::number(0, _expire_time));
        }catch (const std::exception &e) {
            ERR("向缓存添加验证码失败: {}-{}-{}", ssid, code, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    void VerifyCodeData::remove(const std::string &cid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(cid);
            redis.del(key);
        }catch (const std::exception &e) {
            ERR("从缓存删除验证码失败: {}-{}", cid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    VerifyCode::ptr VerifyCodeData::get(const std::string &cid) {
        VerifyCode::ptr code;
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(cid);
            std::unordered_map<std::string, std::string> map;
            redis.hgetall(key, std::inserter(map, map.begin()));
            if (map.empty()) return nullptr;
            code = std::make_shared<VerifyCode>();
            code->session_id = map[_key_ssid];
            code->code_id = map[_key_cid];
            code->code =  map[_key_code];
            code->email =  map[_key_email];
        }catch (const std::exception &e) {
            ERR("从缓存删除验证码失败: {}-{}", cid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
        return code;
    }

    std::string VerifyCodeData::getCacheKey(const std::string &cid) {
        return _prefix + cid;
    }
}