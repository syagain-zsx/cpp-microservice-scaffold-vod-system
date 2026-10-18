#include "user_identify_role.h"


namespace vp_data{
    const size_t UserIdentifyRoleData::_expire_time = 3600;
    const std::string UserIdentifyRoleData::_prefix = "user_identify_role_";
    UserIdentifyRoleData::UserIdentifyRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis)
        : _mysql(mtx.database()), _redis(redis) {}
    UserIdentifyRoleData::UserIdentifyRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync)
        : _mysql(mtx.database()), _redis(redis), _sync(sync) {}
    // 1. 新增用户身份角色关系
    void UserIdentifyRoleData::insert(const std::string &uid, IdentifyType itype, RoleType rtype) {
        try {
            insertToDb(uid, itype, rtype);
            // 1. 从缓存删除数据
            removeFromCache(uid);
            // 3. 进行缓存数据延迟同步删除
            _sync->sync(getCacheKey(uid));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 2. 删除用户身份角色关系
    void UserIdentifyRoleData::remove(const std::string &uid, IdentifyType itype, RoleType rtype) {
        try {
            // 2. 从数据库删除数据
            if (rtype == RoleType::UNKNOWN) {
                removeFromDb(uid, itype);
            }else if (itype == IdentifyType::UNKNOWN) {
                removeFromDb(uid, rtype);
            }else {
                removeFromDb(uid, itype, rtype);
            }
            // 1. 从缓存删除数据
            removeFromCache(uid);
            // 3. 进行缓存数据延迟同步删除
            _sync->sync(getCacheKey(uid));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 3. 获取用户身份角色关系
    std::vector<UserIdentifyRole> UserIdentifyRoleData::list(const std::string &uid) {
        try {
            // 1. 从缓存获取数据
            std::vector<UserIdentifyRole> uirs = listFromCache(uid);
            if (uirs.size() > 0) { return uirs; }
            uirs = listFromDb(uid);
            if (uirs.size() > 0) {
                // 2. 从数据库获取数据成功，将数据添加到缓存中
                insertToCache(uid, uirs);
            }else {
                WRN("从数据库没有获取到用户的身份角色关系: {}", uid);
            }
            return uirs;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 4. 判断用户是否具有指定身份
    bool UserIdentifyRoleData::hasIdentify(const std::string &uid, IdentifyType itype) {
        try {
            // 1. 获取用户身份角色关系
            std::vector<UserIdentifyRole> uirs = list(uid);
            // 2. 判断用户是否具有指定身份
            for (auto &uir : uirs) {
                // DBG("用户{} 身份信息: {}", uir.user_id(), (int)uir.identify_type());
                if (uir.identify_type() == itype) {
                    return true;
                }
            }
            return false;
        }catch (const vp_error::VPException &e) {
            throw ;
        }

    }
    // 5. 判断用户是否具有指定角色
    bool UserIdentifyRoleData::hasRole(const std::string &uid, RoleType rtype) {
        try {
            // 1. 获取用户身份角色关系
            std::vector<UserIdentifyRole> uirs = list(uid);
            // 2. 判断用户是否具有指定角色
            for (auto &uir : uirs) {
                if (uir.role_type() == rtype) {
                    return true;
                }
            }
            return false;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }

    
    //1.向数据库新增用户身份角色关系
    void UserIdentifyRoleData::insertToDb(const std::string &uid, IdentifyType itype, RoleType rtype) {
        try {
            UserIdentifyRole uir(uid, itype, rtype);
            _mysql.persist(uir);
        } catch (const odb::exception &e) {
            ERR("向数据库新增用户身份角色关系失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    //2.从数据库删除用户身份角色关系
    void UserIdentifyRoleData::removeFromDb(const std::string &uid, IdentifyType itype, RoleType rtype) {
        try {
            typedef odb::query<UserIdentifyRole> Query;
            // typedef odb::result<UserIdentifyRole> Result;
            _mysql.erase_query<UserIdentifyRole>(Query::user_id == uid && 
                Query::identify_type == itype && 
                Query::role_type == rtype);
        } catch (const odb::exception &e) {
            ERR("从数据库删除用户身份角色关系失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void UserIdentifyRoleData::removeFromDb(const std::string &uid, RoleType rtype) {
        try {
            typedef odb::query<UserIdentifyRole> Query;
            // typedef odb::result<UserIdentifyRole> Result;
            _mysql.erase_query<UserIdentifyRole>(Query::user_id == uid && Query::role_type == rtype);
        } catch (const odb::exception &e) {
            ERR("从数据库删除用户身份角色关系失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void UserIdentifyRoleData::removeFromDb(const std::string &uid, IdentifyType itype) {
        try {
            typedef odb::query<UserIdentifyRole> Query;
            // typedef odb::result<UserIdentifyRole> Result;
            _mysql.erase_query<UserIdentifyRole>(Query::user_id == uid && Query::identify_type == itype);
        } catch (const odb::exception &e) {
            ERR("从数据库删除用户身份角色关系失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    //3.从数据库获取用户身份角色关系
    std::vector<UserIdentifyRole> UserIdentifyRoleData::listFromDb(const std::string &uid) {
        try {
            typedef odb::query<UserIdentifyRole> Query;
            typedef odb::result<UserIdentifyRole> Result;
            Result uirs(_mysql.query<UserIdentifyRole>(Query::user_id == uid));
            std::vector<UserIdentifyRole> list;
            for (auto it = uirs.begin(); it != uirs.end(); ++it) {
                list.push_back(*it);
            }
            return list;
        } catch (const odb::exception &e) {
            ERR("从数据库获取用户身份角色关系失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return {};
    }
    //4.向缓存添加用户身份角色信息
    void UserIdentifyRoleData::insertToCache(const std::string &uid, const std::vector<UserIdentifyRole> &uirs) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            // 1. 组装缓存key
            std::string key = getCacheKey(uid);
            // 2. 关系信息缓存方式是hash，因此需要针对身份和角色，组成unordered_map
            std::unordered_map<int, int> uir_map;
            for (auto &uir : uirs) {
                uir_map[(int)uir.identify_type()] = (int)uir.role_type();
            }
            // 3. 缓存数据
            redis.hmset(key, uir_map.begin(), uir_map.end());
            // 4. 设置缓存过期时间
            redis.expire(key, _expire_time + biteutil::Random::number(0, _expire_time));
        } catch (const sw::redis::Error &e) {
            ERR("向缓存添加用户身份角色信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    //5.从缓存获取用户身份角色信息
    std::vector<UserIdentifyRole> UserIdentifyRoleData::listFromCache(const std::string &uid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            // 1. 组装缓存key
            std::string key = getCacheKey(uid);
            // 2. 从缓存获取数据
            std::vector<UserIdentifyRole> uirs;
            std::unordered_map<std::string, std::string> uir_map;
            redis.hgetall(key, std::inserter(uir_map, uir_map.begin()));
            for (auto &uir : uir_map) {
                uirs.emplace_back(uid, (IdentifyType)std::stoi(uir.first), (RoleType)std::stoi(uir.second));
            }
            return uirs;
        } catch (const sw::redis::Error &e) {
            ERR("从缓存获取用户身份角色信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
        return {};
    }
    //6.从缓存删除用户身份角色信息
    void UserIdentifyRoleData::removeFromCache(const std::string &uid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            // 1. 组装缓存key
            std::string key = getCacheKey(uid);
            // 2. 从缓存删除数据
            redis.del(key);
        } catch (const sw::redis::Error &e) {
            ERR("从缓存获取用户身份角色信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    //7.获取缓存key
    std::string UserIdentifyRoleData::getCacheKey(const std::string &uid) {
        return _prefix + uid;
    }
}