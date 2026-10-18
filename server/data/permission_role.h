#pragma once
#include <bite_scaffold/odb.h>
#include <bite_scaffold/redis.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include <bite_scaffold/util.h>
#include "../common/sync.h"
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>

namespace vp_data{
    class PermissionRoleData{
        public:
            using ptr = std::shared_ptr<PermissionRoleData>;
            PermissionRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis);
            PermissionRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            // 1. 新增权限角色关系
            void insert(const std::string &url, RoleType roles);
            void insert(const std::string &url, std::vector<RoleType> &roles);
            // 2. 删除权限角色关系
            void remove(const std::string &url);//应用于删除操作时，删除操作所有的关联信息
            void remove(const std::string &url, RoleType roles);//应用于更新操作时，调整指定角色的操作权限
            //3. 获取权限角色关系
            std::unordered_set<RoleType> list(const std::string &url);
        private:
            //1.向数据库新增权限角色关系
            void insertToDb(const std::string &url, RoleType roles);
            //2.从数据库删除权限角色关系
            void removeFromDb(const std::string &url, RoleType roles);
            void removeFromDb(const std::string &url);
            //3.从数据库获取权限角色关系
            std::unordered_set<RoleType> listFromDb(const std::string &url);
            //4.向缓存添加权限角色信息
            void insertToCache(const std::string &url, const std::unordered_set<RoleType> &roles);
            //5.从缓存获取权限角色信息
            std::unordered_set<RoleType> listFromCache(const std::string &url);
            //6.从缓存删除权限角色信息
            void removeFromCache(const std::string &url);
            //7.获取缓存key
            std::string getCacheKey(const std::string &url);
        private:
            static const std::string _prefix;
            // 数据库操作句柄
            odb::database &_mysql;
            // 缓存操作句柄
            std::shared_ptr<sw::redis::Redis> _redis;
            // 缓存同步对象
            vp_sync::CacheSync::ptr _sync;
    };
}