#pragma once
#include <bite_scaffold/odb.h>
#include <bite_scaffold/redis.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include "../common/sync.h"
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>

namespace vp_data{
    class SessionData {
        public:
            using ptr = std::shared_ptr<SessionData>;
            //db 是database操作句柄创建事务对象所构造的db对象
            //redis是redis操作句柄创建的事务对象所构造的redis对象
            SessionData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis); //这种方式实例化的对象只能进行新增和获取
            SessionData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            // 1. 向数据库新增会话
            void insert(Session &session);
            // 2. 更新数据库会话，并删除会话缓存，发布删除消息
            void update(Session &session);
            // 3. 通过会话ID删除数据库会话，并删除会话缓存，发布删除消息
            void removeBySid(const std::string &session_id);
            // 4. 通过用户ID删除数据库会话，并删除会话缓存，发布删除消息
            void removeByUid(const std::string &user_id);
            // 5. 获取会话信息（优先从缓存获取，缓存未命中则从数据库获取，并添加缓存）
            Session::ptr selectBySid(const std::string &session_id);
        private:
            // 1. 向数据库添加会话信息
            void insertToDb(Session &session);
            // 2. 从数据库获取会话信息
            Session::ptr selectFromDb(const std::string &session_id);
            std::vector<Session::ptr> selectFromDbByUid(const std::string &user_id);
            // 3. 修改数据库会话信息
            void updateToDb(Session &session);
            // 4. 通过会话ID删除数据库会话
            void removeBySidFromDb(const std::string &session_id);
            // 5. 通过用户ID删除数据库会话
            void removeByUidFromDb(const std::string &user_id);
            // 6. 
            // 7. 向redis添加会话缓存
            void insertToRedis(Session &session);
            // 8. 从redis获取会话缓存
            Session::ptr selectFromRedis(const std::string &session_id);
            // 9. 通过会话ID删除缓存会话
            void removeFromRedis(const std::string &session_id);
            // 11. 通过会话ID获取缓存key
            std::string getCacheKey(const std::string &session_id);
        private:
            static const std::string _cache_prefix;
            static const int _cache_expire;
            static const std::string _field_session_id;
            static const std::string _field_user_id;
            odb::database &_db;
            std::shared_ptr<sw::redis::Redis> _redis;
            vp_sync::CacheSync::ptr _sync;
    };
}