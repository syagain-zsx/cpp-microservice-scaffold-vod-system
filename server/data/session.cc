#include "session.h"

namespace vp_data{
    
    const std::string SessionData::_cache_prefix = "vp_session_";
    const int SessionData::_cache_expire = 3600;
    const std::string SessionData::_field_session_id = "sessionId";
    const std::string SessionData::_field_user_id = "userId";
    SessionData::SessionData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis)
        : _db(mtx.database()), _redis(redis) {}
    SessionData::SessionData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync)
        : _db(mtx.database()), _redis(redis), _sync(sync) {}
    // 1. 向数据库新增会话
    void SessionData::insert(Session &session) {
        try {
            insertToDb(session);
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 2. 更新数据库会话，并删除会话缓存，发布删除消息
    void SessionData::update(Session &session) {
        try {
            auto ss = this->selectFromDb(session.session_id());
            if (!ss) {
                insertToDb(session);
                return;
            }
            ss->set_user_id(session.user_id());
            updateToDb(*ss);
            _sync->sync(getCacheKey(session.session_id()));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 3. 通过会话ID删除数据库会话，并删除会话缓存，发布删除消息
    void SessionData::removeBySid(const std::string &session_id) {
        try {
            removeBySidFromDb(session_id);
            _sync->sync(getCacheKey(session_id));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 4. 通过用户ID删除数据库会话，并删除会话缓存，发布删除消息
    void SessionData::removeByUid(const std::string &user_id) {
        try {
            // 1. 从数据库通过用户ID，获取会话ID列表
            auto list = selectFromDbByUid(user_id);
            if (list.empty()) {
                return;
            }
            // 4. 删除数据库会话
            removeByUidFromDb(user_id);
            // 2. 删除所有会话缓存
            for (auto &session : list) {
                DBG("删除用户 {} 原有会话: {}", user_id, session->session_id());
                _sync->sync(getCacheKey(session->session_id()));
            }
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 5. 获取会话信息（优先从缓存获取，缓存未命中则从数据库获取，并添加缓存）
    Session::ptr SessionData::selectBySid(const std::string &session_id) {
        try {
            auto session = selectFromRedis(session_id);
            if (session) {
                return session;
            }
            session = selectFromDb(session_id);
            if (session) {
                insertToRedis(*session);
            }
            return session;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    
    // 11. 通过会话ID获取缓存key
    std::string SessionData::getCacheKey(const std::string &session_id) {
        return _cache_prefix + session_id;
    }
    // 1. 向数据库添加会话信息
    void SessionData::insertToDb(Session &session) {
        try {
            _db.persist(session);
        }catch (const std::exception &e) {
            ERR("向数据库新增会话信息失败:{}-{}", session.session_id(), e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    // 2. 从数据库获取会话信息
    Session::ptr SessionData::selectFromDb(const std::string &session_id) {
        Session::ptr res;
        try {
            typedef odb::query<Session> Query;
            res.reset(_db.query_one<Session>(Query::session_id == session_id));
        }catch (const std::exception &e) {
            ERR("通过会话ID从数据库获取会话信息失败:{}-{}", session_id, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return res;
    }
    std::vector<Session::ptr> SessionData::selectFromDbByUid(const std::string &user_id) {
        std::vector<Session::ptr> list;
        try {
            typedef odb::query<SessionPtr> Query;
            typedef odb::result<SessionPtr> Result;
            Result res(_db.query<SessionPtr>(Query::user_id == user_id));
            for (auto it = res.begin(); it != res.end(); ++it) {
                list.push_back(it->session);
            }
        }catch (const std::exception &e) {
            ERR("通过用户ID从数据库获取会话信息失败:{}-{}", user_id, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return list;
    }
    // 3. 修改数据库会话信息--传入的sessoin对象，必须是selectFromDb获取到的对象
    void SessionData::updateToDb(Session &session) {
        try {
            _db.update(session);
        }catch (const std::exception &e) {
            ERR("更新数据库会话信息失败:{}-{}", session.session_id(), e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    // 4. 通过会话ID删除数据库会话
    void SessionData::removeBySidFromDb(const std::string &session_id) {
        try {
            typedef odb::query<Session> Query;
            _db.erase_query<Session>(Query::session_id == session_id);
        }catch (const std::exception &e) {
            ERR("通过会话ID删除数据库会话信息失败:{}-{}", session_id, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    // 5. 通过用户ID删除数据库会话
    void SessionData::removeByUidFromDb(const std::string &user_id) {
        try {
            typedef odb::query<Session> Query;
            _db.erase_query<Session>(Query::user_id == user_id);
        }catch (const std::exception &e) {
            ERR("通过用户ID删除数据库会话信息失败:{}-{}", user_id, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    // 7. 向redis添加会话缓存
    void SessionData::insertToRedis(Session &session) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(session.session_id());
            std::unordered_map<std::string, std::string> data;
            data[_field_session_id] = session.session_id();
            if (session.user_id()) {
                data[_field_user_id] = session.user_id().get();
            }
            redis.hmset(key, data.begin(), data.end());
            redis.expire(key, std::chrono::seconds(_cache_expire));
        }catch (const std::exception &e) {
            ERR("向缓存添加会话信息失败:{}-{}", session.session_id(), e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    // 8. 从redis获取会话缓存
    Session::ptr SessionData::selectFromRedis(const std::string &session_id) {
        Session::ptr session;
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            // 1. 构造缓存key
            std::string key = getCacheKey(session_id);
            // 2. 通过key获取缓存数据
            std::unordered_map<std::string, std::string> results;
            redis.hgetall(key, std::inserter(results, results.begin()));
            if (results.empty()) {
                //缓存未命中
                return nullptr;
            }
            // 3. 构造Session对象
            session = std::make_shared<Session>();
            if (results.find(_field_session_id) != results.end()) {
                session->set_session_id(results[_field_session_id]);
            }
            if (results.find(_field_user_id) != results.end()) {
                session->set_user_id(results[_field_user_id]);
            }
        }catch (const std::exception &e) {
            ERR("从缓存获取会话信息失败:{}-{}", session_id, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
        // 4. 返回Session对象
        return session;
    }
    // 9. 通过会话ID删除缓存会话
    void SessionData::removeFromRedis(const std::string &session_id) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(session_id);
            redis.del(key);
        }catch (const std::exception &e) {
            ERR("从缓存删除会话信息失败:{}-{}", session_id, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
}