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
    class UserData {
        public:
            using ptr = std::shared_ptr<UserData>;
            UserData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            //新增用户信息（用户ID，邮箱）
            void insert(const std::string &uid, const std::string email);
            //邮箱是否已注册指定身份
            bool isExist(const std::string &email, const IdentifyType type = vp_data::IdentifyType::NORMAL);
            //用户密码验证(注：用户名-邮箱/昵称/备用昵称/手机号&密码)
            User::ptr verify(const std::string &username, const std::string &passwd);
            //通过用户ID获取用户信息
            User::ptr getUser(const std::string &uid);
            //根据用户ID获取用户信息列表（注：用于视频列表获取的时候）
            std::unordered_map<std::string, User::ptr> getListById(const std::unordered_set<std::string> &uids);
            //获取指定状态及身份的用户信息列表及总量
            StatusIdentifyUserList::ptr getIdentifyUserList(const UserStatus status, const IdentifyType identify, 
                const size_t page, const size_t size);
            //修改头像(返回原来的头像文件ID，用于发布文件删除消息)
            std::optional<std::string> updateAvatar(const std::string &uid, const std::string &avatar_fid);
            //修改用户状态
            void updateStatus(const std::string &uid, const UserStatus status);
            //修改昵称
            void updateNickname(const std::string &uid, const std::string &nickname);
            //修改密码
            void updatePasswd(const std::string &uid, const std::string &passwd);
            //判断用户是否是启用状态（注：用于管理员各项操作）
            bool isEnable(const std::string &uid);
            bool hasStatus(const std::string &uid, UserStatus status);
            //修改管理员
            void updateAdmin(const std::string &uid, const std::string &nickname, const std::string &remark, UserStatus status);
            //通过邮箱获取用户（管理员）信息
            User::ptr getUserByEmail(const std::string &email);
        private:
            // 1. 向数据库新增用户信息
            void insertToDB(const std::string &uid, const std::string email);
            // 2. 通过身份类型和邮箱获取用户信息
            User::ptr getUserByTypeFromDB(const std::string &email, const IdentifyType type);
            // 2. 通过用户ID从数据库获取用户信息（注：定义视图结构）
            User::ptr getUserByIdFromDB(const std::string &uid);
            // 3. 通过邮箱从数据库获取用户信息
            User::ptr getUserByEmailFromDB(const std::string &email);
            // 4. 通过手机号从数据库获取用户信息
            User::ptr getUserByPhoneFromDB(const std::string &phone);
            // 5. 通过昵称从数据库获取用户信息
            User::ptr getUserByNicknameFromDB(const std::string &nickname);
            // 6. 通过备用名从数据库获取用户信息
            User::ptr getUserByBaknameFromDB(const std::string &bakname);
            // 7. 通过状态&身份获取用户信息列表(注：需定义视图结构)
            StatusIdentifyUserList::ptr getIdentifyUserListFromDB(const UserStatus status, const IdentifyType identify, 
                const size_t page, const size_t size);
            // 10. 修改数据库用户信息
            void updateToDB(const User::ptr &user);
            // 11. 向缓存添加用户信息
            void insertToCache(const User::ptr &user);
            // 12. 从缓存获取用户信息
            User::ptr getUserFromCache(const std::string &uid);
            // 13. 从缓存删除用户信息
            void removeFromCache(const std::string &uid);
            // 14. 获取缓存key
            std::string getCacheKey(const std::string &uid);
        private:
            static const size_t _expire_time;
            static const std::string _prefix;
            static const std::string _key_user_id;
            static const std::string _key_nickname;
            static const std::string _key_bak_nickname;
            static const std::string _key_phone;
            static const std::string _key_email;
            static const std::string _key_avatar;
            static const std::string _key_status;
            static const std::string _key_remark;
            static const std::string _key_reg_time;
            odb::database &_mysql;
            std::shared_ptr<sw::redis::Redis> _redis;
            vp_sync::CacheSync::ptr _sync;
    };
}