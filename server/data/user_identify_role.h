
#pragma once
/*
    用户身份角色关联关系信息的操作封装
        1. 新增用户身份角色关系（向数据库新增）
        2. 删除用户身份角色关系（删除数据库数据，以及缓存数据）
        3. 获取用户身份角色关系（优先从缓存获取，缓存未命中则从数据库获取）
        4. 判断用户是否具有指定身份（获取关系信息，进行判断即可）
        5. 判断用户是否具有指定角色（获取关系信息，进行判断即可）
*/

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
    class UserIdentifyRoleData{
        public:
            using ptr = std::shared_ptr<UserIdentifyRoleData>;
            UserIdentifyRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis);
            UserIdentifyRoleData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            // 1. 新增用户身份角色关系
            void insert(const std::string &uid, IdentifyType itype, RoleType rtype);
            // 2. 删除用户身份角色关系
            void remove(const std::string &uid, IdentifyType itype, RoleType rtype = RoleType::UNKNOWN);
            // 3. 获取用户身份角色关系
            std::vector<UserIdentifyRole> list(const std::string &uid);
            // 4. 判断用户是否具有指定身份
            bool hasIdentify(const std::string &uid, IdentifyType itype);
            // 5. 判断用户是否具有指定角色
            bool hasRole(const std::string &uid, RoleType rtype);
        private:
            //1.向数据库新增用户身份角色关系
            void insertToDb(const std::string &uid, IdentifyType itype, RoleType rtype);
            //2.从数据库删除用户身份角色关系
            void removeFromDb(const std::string &uid, IdentifyType itype, RoleType rtype);
            void removeFromDb(const std::string &uid, RoleType rtype);
            void removeFromDb(const std::string &uid, IdentifyType itype);
            //3.从数据库获取用户身份角色关系
            std::vector<UserIdentifyRole> listFromDb(const std::string &uid);
            //4.向缓存添加用户身份角色信息
            void insertToCache(const std::string &uid, const std::vector<UserIdentifyRole> &uirs);
            //5.从缓存获取用户身份角色信息
            std::vector<UserIdentifyRole> listFromCache(const std::string &uid);
            //6.从缓存删除用户身份角色信息
            void removeFromCache(const std::string &uid);
            //7.获取缓存key
            std::string getCacheKey(const std::string &uid);
        private:
            static const size_t _expire_time;
            static const std::string _prefix;
            // 数据库操作句柄
            odb::database &_mysql;
            // 缓存操作句柄
            std::shared_ptr<sw::redis::Redis> _redis;
            // 缓存同步对象
            vp_sync::CacheSync::ptr _sync;
    };
}