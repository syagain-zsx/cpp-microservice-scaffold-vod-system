#include "user.h"

namespace vp_data{
    const size_t UserData::_expire_time = 3600;
    const std::string UserData::_prefix = "key_user_";
    const std::string UserData::_key_user_id = "userId";
    const std::string UserData::_key_nickname = "nickname";
    const std::string UserData::_key_bak_nickname = "bakname";
    const std::string UserData::_key_phone = "phone";
    const std::string UserData::_key_email = "email";
    const std::string UserData::_key_avatar = "avatarFileId";
    const std::string UserData::_key_status = "userStatus";
    const std::string UserData::_key_remark = "userMemo";
    const std::string UserData::_key_reg_time = "userCTime";

    UserData::UserData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync)
        : _mysql(mtx.database()), _redis(redis), _sync(sync) {}
    //新增用户信息（用户ID，邮箱）
    void UserData::insert(const std::string &uid, const std::string email) {
        try {
            insertToDB(uid, email);
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //邮箱是否已注册指定身份
    // 1. 新增用户的时候判断，这个邮箱是否已经注册过普通用户
    // 2. 新增管理员的时候判断，当前邮箱是否已经注册过管理员用户
    bool UserData::isExist(const std::string &email, const IdentifyType type) {
        try {
            auto user = getUserByTypeFromDB(email, type);
            return user? true : false;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
        return false; //这一步肯定走不到
    }
    //用户密码验证(注：用户名-邮箱/昵称/备用昵称/手机号&密码)
    User::ptr UserData::verify(const std::string &username, const std::string &passwd) {
        User::ptr user;
        try {
            user = getUserByEmailFromDB(username);
            if (user && user->password()) return user->password().get() == passwd ? user : nullptr;
            user = getUserByPhoneFromDB(username);
            if (user && user->password()) return user->password().get() == passwd ? user : nullptr;
            user = getUserByNicknameFromDB(username);
            if (user && user->password()) return user->password().get() == passwd ? user : nullptr;
            user = getUserByBaknameFromDB(username);
            if (user && user->password()) return user->password().get() == passwd ? user : nullptr;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
        return nullptr;
    }
    //通过用户ID获取用户信息，优先从缓存获取，缓存未命中则从数据库获取
    User::ptr UserData::getUser(const std::string &uid) {
        //1. 从缓存获取用户信息
        User::ptr user;
        try {
            user = getUserFromCache(uid);
            if (user) return user;
            //2. 从数据库获取用户信息
            user = getUserByIdFromDB(uid);
            if (user) {
                //3. 向缓存添加用户信息
                insertToCache(user);
            }
        } catch (const vp_error::VPException &e) {
            throw ;
        }
        return user;
    }
    //根据用户ID获取用户信息列表（注：用于视频列表获取的时候）
    std::unordered_map<std::string, User::ptr> UserData::getListById(const std::unordered_set<std::string> &uids) {
        std::unordered_map<std::string, User::ptr> users;
        try {
            for (auto &uid : uids) {
                User::ptr user = getUser(uid);
                if (user) users.insert(std::make_pair(uid, user));
            }
        } catch (const vp_error::VPException &e) {
            throw ;
        }
        return users;
    }
    //修改头像(返回原来的头像文件ID，用于发布文件删除消息)
    std::optional<std::string> UserData::updateAvatar(const std::string &uid, const std::string &avatar_fid) {
        try {
            //1. 从数据库通过用户ID获取用户信息
            User::ptr user = getUserByIdFromDB(uid);
            if (!user) {
                ERR("用户信息不存在: {}", uid);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            //2. 更新头像信息
            auto old_avatar = user->avatar();
            if (old_avatar && *old_avatar == avatar_fid) return std::optional<std::string>();
            user->set_avatar(avatar_fid);
            updateToDB(user);
            //4. 进行缓存同步
            _sync->sync(getCacheKey(uid));
            //5. 返回原来的头像文件ID
            if (old_avatar) return *old_avatar;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
        return std::optional<std::string>();
    }
    //修改用户状态
    void UserData::updateStatus(const std::string &uid, const UserStatus status) {
        try {
            //1. 从数据库通过用户ID获取用户信息
            User::ptr user = getUserByIdFromDB(uid);
            if (!user) {
                ERR("用户信息不存在: {}", uid);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            //2. 更新用户状态
            user->set_status(status);
            updateToDB(user);
            //4. 进行缓存同步
            _sync->sync(getCacheKey(uid));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //修改昵称
    void UserData::updateNickname(const std::string &uid, const std::string &nickname) {
        try {
            User::ptr user = getUserByNicknameFromDB(nickname);
            if (user && uid != user->user_id()) {
                ERR("昵称已经被占用: {}", nickname);
                throw vp_error::VPException(vp_error::VpError::NICKNAME_IS_EXISTS);
            }
            //1. 从数据库通过用户ID获取用户信息
            user = getUserByIdFromDB(uid);
            if (!user) {
                ERR("用户信息不存在: {}", uid);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            //2. 更新用户昵称
            user->set_nickname(nickname);
            updateToDB(user);
            //4. 进行缓存同步
            _sync->sync(getCacheKey(uid));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //修改密码
    void UserData::updatePasswd(const std::string &uid, const std::string &passwd) {
        try {
            //1. 从数据库通过用户ID获取用户信息
            User::ptr user = getUserByIdFromDB(uid);
            if (!user) {
                ERR("用户信息不存在: {}", uid);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            if (user->password() && user->password().get() == passwd) {
                ERR("新密码不能与原密码相同: {}", passwd);
                throw vp_error::VPException(vp_error::VpError::NEW_PASSWD_SAME_OLD);
            }
            //2. 更新用户密码
            user->set_password(passwd);
            updateToDB(user);
            //因为缓存中没有保存密码信息，因此不需要进行缓存同步
        } catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //判断用户是否是启用状态（注：用于管理员各项操作）
    bool UserData::isEnable(const std::string &uid) {
        return hasStatus(uid, UserStatus::ENABLE);
    }
    bool UserData::hasStatus(const std::string &uid, UserStatus status) {
        try {
            //1. 获取用户信息
            auto user = getUser(uid);
            if (!user) {
                ERR("用户信息不存在: {}", uid);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            //2. 判断用户是否是启用状态
            return user->status() == status;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
        return false; //这一步肯定走不到
    }
    //修改管理员
    void UserData::updateAdmin(const std::string &uid, const std::string &nickname, const std::string &remark, UserStatus status) {
        try {
            User::ptr user = getUserByBaknameFromDB(nickname);
            if (user && user->user_id() != uid) {
                ERR("昵称已经被占用: {}", nickname);
                throw vp_error::VPException(vp_error::VpError::NICKNAME_IS_EXISTS);
            }
            //1. 从数据库获取用户信息
            user = getUserByIdFromDB(uid);
            if (!user) {
                ERR("用户信息不存在: {}", uid);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            //2. 更新用户信息
            user->set_bakname(nickname); //对于管理员来说，昵称就是备用名
            user->set_remark(remark);
            user->set_status(status);
            updateToDB(user);
            //3. 同步缓存
            _sync->sync(getCacheKey(uid));
        }catch (const vp_error::VPException &e) {
            throw ;
        }
    }
    //获取指定状态及身份的用户信息列表及总量
    StatusIdentifyUserList::ptr UserData::getIdentifyUserList(const UserStatus status, const IdentifyType identify, 
        const size_t page, const size_t size) {
        StatusIdentifyUserList::ptr list;
        try {
            list = getIdentifyUserListFromDB(status, identify, page, size);
        }catch (const vp_error::VPException &e) {
            throw ;
        }
        return list;
    }
    //通过邮箱获取用户（管理员）信息
    User::ptr UserData::getUserByEmail(const std::string &email) {
        try {
            auto user = getUserByEmailFromDB(email);
            if (!user) {
                ERR("用户信息不存在: {}", email);
                throw vp_error::VPException(vp_error::VpError::USER_NOT_EXIST);
            }
            return user;
        }catch (const vp_error::VPException &e) {
            throw ;
        }
        return nullptr;
    }


    // 1. 向数据库新增用户信息
    void UserData::insertToDB(const std::string &uid, const std::string email) {
        try {
            User user(uid, email);
            _mysql.persist(user);
        } catch (const odb::exception &e) {
            ERR("向数据库新增用户信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    User::ptr UserData::getUserByTypeFromDB(const std::string &email, const IdentifyType type) {
        try {
            typedef odb::query<EmailIdendifyUser> Query;
            Query cond(Query::User::email==email && Query::UserIdentifyRole::identify_type==type);
            EmailIdendifyUser::ptr res(_mysql.query_one<EmailIdendifyUser>(cond));
            if (res) return res->user;
            else DBG("未找到邮箱信息: {}-{}", email, (int)type);
        }catch (const odb::exception &e) {
            ERR("从数据库通过邮箱及身份获取用户信息失败: {}-{}", email, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 2. 通过用户ID从数据库获取用户信息（注：定义视图结构）
    User::ptr UserData::getUserByIdFromDB(const std::string &uid) {
        try {
            typedef odb::query<User> Query;
            User::ptr user(_mysql.query_one<User>(Query::user_id == uid));
            return user;
        }catch (const odb::exception &e) {
            ERR("从数据库通过用户ID获取用户信息失败: {}-{}", uid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 3. 通过邮箱从数据库获取用户信息
    User::ptr UserData::getUserByEmailFromDB(const std::string &email) {
        try {
            typedef odb::query<User> Query;
            User::ptr user(_mysql.query_one<User>(Query::email == email));
            return user;
        }catch (const odb::exception &e) {
            ERR("从数据库通过邮箱获取用户信息失败: {}-{}", email, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 4. 通过手机号从数据库获取用户信息
    User::ptr UserData::getUserByPhoneFromDB(const std::string &phone) {
        try {
            typedef odb::query<User> Query;
            User::ptr user(_mysql.query_one<User>(Query::phone == phone));
            return user;
        }catch (const odb::exception &e) {
            ERR("从数据库通过手机号获取用户信息失败: {}-{}", phone, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 5. 通过昵称从数据库获取用户信息
    User::ptr UserData::getUserByNicknameFromDB(const std::string &nickname) {
        try {
            typedef odb::query<User> Query;
            User::ptr user(_mysql.query_one<User>(Query::nickname == nickname));
            return user;
        }catch (const odb::exception &e) {
            ERR("从数据库通过昵称获取用户信息失败: {}-{}", nickname, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 6. 通过备用名从数据库获取用户信息
    User::ptr UserData::getUserByBaknameFromDB(const std::string &bakname) {
        try {
            typedef odb::query<User> Query;
            User::ptr user(_mysql.query_one<User>(Query::bakname == bakname));
            return user;
        }catch (const odb::exception &e) {
            ERR("从数据库通过备用名获取用户信息失败: {}-{}", bakname, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 7. 通过状态&身份获取用户信息列表(注：需定义视图结构)
    StatusIdentifyUserList::ptr UserData::getIdentifyUserListFromDB(const UserStatus status, 
        const IdentifyType identify, 
        const size_t page, const size_t size) {
        try {
            StatusIdentifyUserList::ptr list = std::make_shared<StatusIdentifyUserList>();
            //获取指定身份的用户信息列表
            std::string limit = ASC_LIMIT_PAGE(page, size); //定义分页查询字符串: order by id limit n offset s;
            typedef odb::query<StatusIdentifyUser> Query;
            typedef odb::result<StatusIdentifyUser> Result;
            Query cond;
            if (status == vp_data::UserStatus::UNKNOWN) {
                cond = Query(Query::UserIdentifyRole::identify_type==identify);
            }else {
                cond = Query(Query::User::status==status && Query::UserIdentifyRole::identify_type==identify);
            }
            // where user.status=xx and uir.identify_type=xx order by id limit n offset s;
            Result res(_mysql.query<StatusIdentifyUser>(cond + limit));
            for (auto &r : res) {
                list->users.push_back(r);
            }
            //获取指定身份的用户总量
            StatusIdentifyUserCount::ptr count(_mysql.query_one<StatusIdentifyUserCount>(cond));
            if (count) list->total = count->count;
            return list;
        }catch (const odb::exception &e) {
            ERR("获取指定身份用户列表失败: {}-{}-{}", (int)status, (int)identify, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    // 10. 修改数据库用户信息:前提是传入的对象必须是从数据库中获取到的对象
    void UserData::updateToDB(const User::ptr &user) {
        try {
            _mysql.update(*user);
        }catch (const odb::exception &e) {
            ERR("修改数据库用户信息失败: {}-{}", user->user_id(), e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    // 11. 向缓存添加用户信息
    void UserData::insertToCache(const User::ptr &user) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 获取缓存key
            std::string key = getCacheKey(user->user_id());
            //2. 组织unordered_map信息
            std::unordered_map<std::string, std::string> map;
            map[_key_user_id] = user->user_id();
            map[_key_nickname] = user->nickname();
            map[_key_email] = user->email();
            map[_key_status] = std::to_string((int)user->status());
            map[_key_reg_time] = boost::posix_time::to_simple_string(user->reg_time());
            if (user->bakname()) map[_key_bak_nickname] = user->bakname().get();
            if (user->phone()) map[_key_phone] = user->phone().get();
            if (user->avatar()) map[_key_avatar] = user->avatar().get();
            if (user->remark()) map[_key_remark] = user->remark().get();
            //3. 向缓存添加用户信息
            redis.hmset(key, map.begin(), map.end());
            //4. 设置缓存key的过期时间
            redis.expire(key, _expire_time + biteutil::Random::number(0, _expire_time));
        }catch (const std::exception &e) {
            ERR("向缓存添加用户信息失败: {}-{}", user->user_id(), e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    // 12. 从缓存获取用户信息
    User::ptr UserData::getUserFromCache(const std::string &uid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 获取缓存key
            std::string key = getCacheKey(uid);
            //2. 获取缓存信息到map中
            std::unordered_map<std::string, std::string> map;
            redis.hgetall(key, std::inserter(map, map.begin()));
            if (map.empty()) return nullptr;
            //3. 组织User对象
            User::ptr user = std::make_shared<User>();
            user->set_user_id(map[_key_user_id]);
            user->set_nickname(map[_key_nickname]);
            user->set_email(map[_key_email]);
            user->set_status((UserStatus)std::stoi(map[_key_status]));
            user->set_reg_time(boost::posix_time::time_from_string(map[_key_reg_time]));
            if (map.find(_key_bak_nickname) != map.end()) user->set_bakname(map[_key_bak_nickname]);
            if (map.find(_key_phone) != map.end()) user->set_phone(map[_key_phone]);
            if (map.find(_key_avatar)!= map.end()) user->set_avatar(map[_key_avatar]);
            if (map.find(_key_remark)!= map.end()) user->set_remark(map[_key_remark]);
            return user;
        }catch (const std::exception &e) {
            ERR("从缓存获取用户信息失败: {}-{}", uid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
        return nullptr;
    }
    // 13. 从缓存删除用户信息
    void UserData::removeFromCache(const std::string &uid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(uid);
            redis.del(key);
        }catch (const std::exception &e) {
            ERR("从缓存删除用户信息失败: {}-{}", uid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    // 14. 获取缓存key
    std::string UserData::getCacheKey(const std::string &uid) {
        return _prefix + uid;
    }


}