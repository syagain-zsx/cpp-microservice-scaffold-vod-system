#include "user_count.h"

namespace vp_data{
    const size_t UserCountData::_expire_time = 1800;
    const std::string UserCountData::_prefix = "user_count_";
    const std::string UserCountData::_play_ckey = "playCount";
    const std::string UserCountData::_like_ckey = "likeCount";
    const std::string UserCountData::_follower_ckey = "followerCount";
    const std::string UserCountData::_followed_ckey = "followedCount";
    UserCountData::UserCountData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis)
        :_mysql(mtx.database()), _redis(redis){}

    UserCount::ptr UserCountData::getCount(const std::string &uid) {
        try {
            //1. 从缓存获取用户统计数据
            UserCount::ptr uc = getCountFromCache(uid);
            if (uc) return uc;
            //2. 从数据库获取用户统计数据
            uc = getCountFromDb(uid);
            if (uc) {
                //3. 将用户统计数据添加到缓存中
                insertToCache(uid, uc);
            }
            return uc;
        }catch (const vp_error::VPException& e) {
            ERR("获取用户统计数据失败: {}-{}", uid, e.what());
            throw ;
        }
    }
    void UserCountData::updatePlayCount(const std::string &uid, int count) {
        try {
            DBG("更新用户视频播放量: {} {}", uid, count);
            updateCountToCache(uid, _play_ckey, count);
        }catch (const vp_error::VPException& e) {
            ERR("更新用户视频播放量失败: {}-{}", uid, e.what());
            throw ;
        }
    }
    void UserCountData::updateLikeCount(const std::string &uid, int count) {
        try {
            updateCountToCache(uid, _like_ckey, count);
        }catch (const vp_error::VPException& e) {
            ERR("更新用户视频点赞量失败: {}-{}", uid, e.what());
            throw ;
        }
    }
    void UserCountData::updateFollowerCount(const std::string &uid, int count) {
        try {
            updateCountToCache(uid, _follower_ckey, count);
        }catch (const vp_error::VPException& e) {
            ERR("更新用户粉丝量失败: {}-{}", uid, e.what());
            throw ;
        }
    }
    void UserCountData::updateFollowedCount(const std::string &uid, int count) {
        try {
            updateCountToCache(uid, _followed_ckey, count);
        }catch (const vp_error::VPException& e) {
            ERR("更新用户关注量失败: {}-{}", uid, e.what());
            throw ;
        }
    }

    std::string UserCountData::getCacheKey(const std::string &uid) {
        return _prefix + uid;
    }
    UserCount::ptr UserCountData::getCountFromDb(const std::string &uid) {
        try {
            //1. 获取粉丝总量
            typedef odb::query<FollowerCount> FollowerQuery;
            FollowerCount::ptr follower_count(_mysql.query_one<FollowerCount>(FollowerQuery::followed_uid == uid));
            //2. 获取关注总量
            typedef odb::query<FollowedCount> FollowedQuery;
            FollowedCount::ptr followed_count(_mysql.query_one<FollowedCount>(FollowedQuery::follower_uid == uid));
            //3. 获取视频播放总量
            typedef odb::query<UserVPlayCount> PlayQuery;
            UserVPlayCount::ptr play_count(_mysql.query_one<UserVPlayCount>(PlayQuery::uploader_uid == uid));
            //4. 获取视频点赞总量
            typedef odb::query<UserVLikeCount> LikeQuery;
            UserVLikeCount::ptr like_count(_mysql.query_one<UserVLikeCount>(LikeQuery::Video::uploader_uid == uid));
            //5. 封装成UserCount对象返回
            UserCount::ptr uc = std::make_shared<UserCount>();
            if (follower_count) uc->follower_count = follower_count->count;
            if (followed_count) uc->followed_count = followed_count->count;
            if (play_count) uc->vplay_count = play_count->count;
            if (like_count) uc->vlike_count = like_count->count;
            return uc;
        }catch (const std::exception& e) {
            ERR("从数据库获取用户统计数据失败: {}-{}", uid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return nullptr;
    }
    void UserCountData::insertToCache(const std::string &uid, const UserCount::ptr &uc) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::unordered_map<std::string, std::string> map;
            map[_play_ckey] = std::to_string(uc->vplay_count);
            map[_like_ckey] = std::to_string(uc->vlike_count);
            map[_follower_ckey] = std::to_string(uc->follower_count);
            map[_followed_ckey] = std::to_string(uc->followed_count);
            std::string key = getCacheKey(uid);
            int timeout = _expire_time + biteutil::Random::number(0, _expire_time);
            redis.hmset(key, map.begin(), map.end());
            redis.expire(key, timeout);
        }catch (const std::exception& e) {
            ERR("向缓存添加用户统计数据失败: {}-{}", uid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    UserCount::ptr UserCountData::getCountFromCache(const std::string &uid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(uid);
            std::unordered_map<std::string, std::string> map;
            redis.hgetall(key, std::inserter(map, map.end()));
            if (map.empty()) return nullptr;
            UserCount::ptr uc = std::make_shared<UserCount>();
            if (map.find(_play_ckey) != map.end()) uc->vplay_count = std::stoi(map[_play_ckey]);
            if (map.find(_like_ckey) != map.end()) uc->vlike_count = std::stoi(map[_like_ckey]);
            if (map.find(_follower_ckey)!= map.end()) uc->follower_count = std::stoi(map[_follower_ckey]);
            if (map.find(_followed_ckey)!= map.end()) uc->followed_count = std::stoi(map[_followed_ckey]);
            return uc;
        } catch (const std::exception& e) {
            ERR("从缓存获取用户统计数据失败: {}-{}", uid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
        return nullptr;
    }
    void UserCountData::updateCountToCache(const std::string &uid, const std::string &ckey, int count) {
        //1. 获取缓存key
        auto rtx = _redis->transaction(false, false);
        auto redis = rtx.redis();
        std::string key = getCacheKey(uid);
        while(true) {
            try {
                //2. 启动缓存事务，进行key值监控，在事务期间一旦key值发生改变，则报异常，重新进行操作
                redis.watch(key);
                bool res = redis.exists(key);
                // 如果用户统计数据缓存不存在，则不需要进行数值更新，直接返回即可
                if(!res){  
                    auto uc = getCountFromDb(uid);
                    if (uc) {
                        //3. 将用户统计数据添加到缓存中
                        insertToCache(uid, uc);
                    }
                }
                DBG("更新用户统计数据: {} {} {}", uid, ckey, count);
                rtx.hincrby(key, ckey, count);
                rtx.exec();
                break;
            }catch(const sw::redis::WatchError &e) {
                //如果事务执行失败：原因可能会有很多，当前最可能的原因：key值被修改
                ERR("更新用户数据统计量事务期间，key值监控异常: {}", e.what());
            }catch (const std::exception& e) {
                ERR("向缓存更新用户统计数据失败: {}-{}", uid, e.what());
                throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
            }
        }
    }
}