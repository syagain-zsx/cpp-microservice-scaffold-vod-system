#include "permission_role.h"

namespace vp_data{
    const std::string PermissionRoleData::_prefix = "permission_role_";
    PermissionRoleData::PermissionRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis)
        : _mysql(mtx.database()), _redis(redis) {}
    PermissionRoleData::PermissionRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync)
        : _mysql(mtx.database()), _redis(redis), _sync(sync) {}
    // 1. 新增权限角色关系
    void PermissionRoleData::insert(const std::string &url, RoleType role) {
        try {
            insertToDb(url, role);
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    void PermissionRoleData::insert(const std::string &url, std::vector<RoleType> &roles) {
        try {
            for (auto &r : roles) {
                insertToDb(url, r);
            }
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    // 2. 删除权限角色关系,//应用于删除操作时，删除操作所有的关联信息
    void PermissionRoleData::remove(const std::string &url) {
        try {
            //2. 删除数据库数据
            removeFromDb(url);
            //1. 删除缓存
            removeFromCache(url);
            //3. 发布延迟同步消息
            _sync->sync(getCacheKey(url));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //应用于更新操作时，调整指定角色的操作权限
    void PermissionRoleData::remove(const std::string &url, RoleType role) {
        try {
            removeFromDb(url, role);
            removeFromCache(url);
            _sync->sync(getCacheKey(url));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //3. 获取权限角色关系
    std::unordered_set<RoleType> PermissionRoleData::list(const std::string &url) {
        try {
            std::unordered_set<RoleType> roles;
            //1. 从缓存获取
            roles = listFromCache(url);
            if (roles.empty()) {
                //2. 从数据库获取
                roles = listFromDb(url);
                //3. 向缓存添加
                if (roles.empty() == false) insertToCache(url, roles);
            }
            return roles;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }

    //1.向数据库新增权限角色关系
    void PermissionRoleData::insertToDb(const std::string &url, RoleType role) {
        try {
            OperatorRole orole(url, role);
            _mysql.persist(orole);
        } catch (const odb::exception &e) {
            ERR("向数据库新增权限角色关系失败: {}-{}-{}", url, (int)role, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    //2.从数据库删除权限角色关系
    void PermissionRoleData::removeFromDb(const std::string &url, RoleType role) {
        try {
            typedef odb::query<OperatorRole> Query;
            _mysql.erase_query<OperatorRole>(Query::operator_url == url && Query::role_type == role);
        } catch(const odb::exception &e) {
            ERR("从数据库删除权限角色关系失败: {}-{}-{}", url, (int)role, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void PermissionRoleData::removeFromDb(const std::string &url) {
        try {
            typedef odb::query<OperatorRole> Query;
            _mysql.erase_query<OperatorRole>(Query::operator_url == url);
        } catch(const odb::exception &e) {
            ERR("从数据库删除权限所有角色关系失败: {}-{}", url, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    //3.从数据库获取权限角色关系
    std::unordered_set<RoleType> PermissionRoleData::listFromDb(const std::string &url) {
        std::unordered_set<RoleType> roles;
        try {
            typedef odb::query<OperatorRole> Query;
            typedef odb::result<OperatorRole> Result;
            Result result(_mysql.query<OperatorRole>(Query::operator_url == url));
            for (auto &r : result) {
                roles.insert(r.role_type());
            }
        }catch (const odb::exception &e) {
            ERR("从数据库获取权限角色关系失败: {}-{}", url, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return roles;
    }
    //4.向缓存添加权限角色信息
    void PermissionRoleData::insertToCache(const std::string &url, const std::unordered_set<RoleType> &roles) {
        std::unordered_set<std::string> roles_str;
        for (auto &r : roles) {
            roles_str.insert(std::to_string((int)r));
        }
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(url);
            redis.sadd(key, roles_str.begin(), roles_str.end());
        } catch (const std::exception &e) {
            ERR("向缓存添加权限角色信息失败: {}-{}", url, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    //5.从缓存获取权限角色信息
    std::unordered_set<RoleType> PermissionRoleData::listFromCache(const std::string &url) {
        std::unordered_set<RoleType> result;
        std::unordered_set<std::string> roles;
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(url);
            redis.smembers(key, std::inserter(roles, roles.begin()));
            for (auto &r : roles) {
                result.insert((RoleType)std::stoi(r));
            }
        }catch (const std::exception &e) {
            ERR("从缓存获取权限角色信息失败: {}-{}", url, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
        return result;
    }
    //6.从缓存删除权限角色信息
    void PermissionRoleData::removeFromCache(const std::string &url) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(url);
            redis.del(key);
        } catch (const std::exception &e) {
            ERR("从缓存删除权限角色信息失败: {}-{}", url, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    //7.获取缓存key
    std::string PermissionRoleData::getCacheKey(const std::string &url) {
        return _prefix + url;
    }
}