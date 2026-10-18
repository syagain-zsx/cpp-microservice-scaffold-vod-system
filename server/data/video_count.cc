#include "video_count.h"
#include <message.pb.h>

namespace vp_data{
    const std::string VideoCountData::_cache_prefix = "video_count_";
    const std::string VideoCountData::_key_play_count = "playCount";
    const std::string VideoCountData::_key_like_count = "likeCount";
    VideoCountData::VideoCountData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync)
        : _mysql(mtx.database()), _redis(redis), _cache_sync(sync) {}
    VideoCount::ptr VideoCountData::getCount(const std::string &vid) {
        auto count = getCountFromCache(vid);
        if (count) return count;
        count = getCountFromDB(vid);
        if (count)  {
            insertCountToCache(vid, count);
            syncCache(vid);
        }
        return count;
    }
    // count时要增加/减少的数据，而不是要设置的数据
    void VideoCountData::updateVideoPlayCount(const std::string &vid, int count) {
        //更新缓存中的数据
        updatePlayCountToCache(vid, count);
    }
    void VideoCountData::updateVideoLikeCount(const std::string &vid, int count) {
        //更新缓存中的数据
        updateLikeCountToCache(vid, count);
    }

    VideoCount::ptr VideoCountData::getCountFromDB(const std::string &vid) {
        try {
            VideoCount::ptr count = std::make_shared<VideoCount>();
            //从数据库获取视频点赞量
            typedef odb::query<VideoPlayCount> PlayQuery;
            auto play_res(_mysql.query_one<VideoPlayCount>(PlayQuery::video_id == vid));
            if (play_res) {
                count->play_count = play_res->count;
                // DBG("从数据库获取视频播放量 : {} - {}", count->play_count, play_res->count);
            }
            //从数据库获取视频播放量
            typedef odb::query<VideoLikeCount> LikeQuery;
            auto like_res(_mysql.query_one<VideoLikeCount>(LikeQuery::video_id == vid));
            if (like_res) {
                count->like_count = like_res->count;
                // DBG("从数据库获取视频点赞量 : {} - {}", count->like_count, like_res->count);
            }
            return count;
        } catch (const odb::exception &e) {
            ERR("从数据库取消视频点赞关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoCountData::removeCountFromCache(const std::string &vid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(vid);
            redis.del(key);
        }catch (const sw::redis::Error &e) {
            ERR("从redis中删除视频统计数据缓存失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    void VideoCountData::insertCountToCache(const std::string &vid, VideoCount::ptr &count) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 获取缓存key
            std::string key = getCacheKey(vid);
            //2. 根据video中的信息，组织unordered_map
            std::unordered_map<std::string, std::string> count_info;
            count_info[_key_play_count] = std::to_string(count->play_count);
            count_info[_key_like_count] = std::to_string(count->like_count);
            //3. 添加reids缓存
            redis.hmset(key, count_info.begin(), count_info.end());
        } catch (const sw::redis::Error &e) {
            ERR("向redis中添加视频统计数据缓存失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    VideoCount::ptr VideoCountData::getCountFromCache(const std::string &vid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 获取缓存key
            std::string key = getCacheKey(vid);
            //2. 从缓存中获取视频信息
            std::unordered_map<std::string, std::string> count_info;
            redis.hgetall(key, std::inserter(count_info, count_info.begin()));
            if (count_info.empty()) return nullptr;
            //3. 构造VideoCount::ptr对象，填充信息
            VideoCount::ptr count = std::make_shared<VideoCount>();
            count->play_count = std::stoi(count_info[_key_play_count]);
            count->like_count = std::stoi(count_info[_key_like_count]);
            return count;
        } catch (const sw::redis::Error &e) {
            ERR("从redis中获取视频统计数据缓存失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    std::string VideoCountData::getCacheKey(const std::string &vid) {
        return _cache_prefix + vid;
    }
    void VideoCountData::syncCache(const std::string &vid) {
        // 这里的缓存同步，不是双写删除同步
        // 而是发布延迟消息，在一定时间后，从缓存加载数据， 将播放量同步到数据库
        // 但是本质上，这里也只需要发布一条延迟消息即可
        _cache_sync->sync(vid);
    }
    void VideoCountData::updatePlayCountToCache(const std::string &vid, int count) {
        auto rtx = _redis->transaction(false, false);
        auto redis = rtx.redis();
        while(true) {
            try {
                // 获取缓存key
                std::string key = getCacheKey(vid);
                // 进行缓存监控
                redis.watch(key);
                // 判断缓存是否存在
                bool res = redis.exists(key);
                if (res == false) {
                    // 如果缓存不存在： 从数据库加载数据，修改后，添加缓存
                    // 从数据库加载数据
                    auto video_count = getCountFromDB(vid);
                    if (video_count == nullptr) {
                        ERR("从数据库加载视频统计数据失败: {}", vid);
                        throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
                    }
                    // 修改数据
                    // DBG("添加缓存视频播放量: {} - {}", video_count->play_count, count);
                    video_count->play_count += count;
                    
                    std::unordered_map<std::string, std::string> count_info;
                    count_info[_key_play_count] = std::to_string(video_count->play_count);
                    count_info[_key_like_count] = std::to_string(video_count->like_count);
                    //3. 添加reids缓存
                    rtx.hmset(key, count_info.begin(), count_info.end()).exec();
                    syncCache(vid); //数据添加到缓存后，不要忘了，发布延迟同步消息
                    return;
                }else {
                    // 如果缓存存在： 直接修改缓存
                    // 启动事务，进行缓存修改
                    // DBG("更新缓存视频播放量: {}", count);
                    rtx.hincrby(key, _key_play_count, count).exec();
                    return;
                }
            }catch(const sw::redis::WatchError &e) {
                //如果事务执行失败：原因可能会有很多，当前最可能的原因：key值被修改
                ERR("更新视频数据统计播放量缓存，事务期间，key值监控异常: {}-{}", vid, e.what());
            }catch (const std::exception& e) {
                ERR("向缓存更新视频统计数据播放量失败: {}-{}", vid, e.what());
                throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
            }
        }
    }
    void VideoCountData::updateLikeCountToCache(const std::string &vid, int count) {
        auto rtx = _redis->transaction(false, false);
        auto redis = rtx.redis();
        while(true) {
            try {
                // 获取缓存key
                std::string key = getCacheKey(vid);
                // 进行缓存监控
                redis.watch(key);
                // 判断缓存是否存在
                bool res = redis.exists(key);
                if (res == false) {
                    // 如果缓存不存在： 从数据库加载数据，修改后，添加缓存
                    // 从数据库加载数据
                    auto video_count = getCountFromDB(vid);
                    if (video_count == nullptr) {
                        ERR("从数据库加载视频统计数据失败: {}", vid);
                        throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
                    }
                    // 修改数据
                    // DBG("添加缓存视频点赞量: {} - {}", video_count->play_count, count);
                    video_count->like_count += count;
                    
                    std::unordered_map<std::string, std::string> count_info;
                    count_info[_key_play_count] = std::to_string(video_count->play_count);
                    count_info[_key_like_count] = std::to_string(video_count->like_count);
                    //3. 添加reids缓存
                    rtx.hmset(key, count_info.begin(), count_info.end()).exec();
                    syncCache(vid); //数据添加到缓存后，不要忘了，发布延迟同步消息
                    return;
                }else {
                    // 如果缓存存在： 直接修改缓存
                    // 启动事务，进行缓存修改
                    // DBG("更新缓存视频点赞量: {}", count);
                    rtx.hincrby(key, _key_like_count, count).exec();
                    return;
                }
            }catch(const sw::redis::WatchError &e) {
                //如果事务执行失败：原因可能会有很多，当前最可能的原因：key值被修改
                ERR("更新视频数据统计点赞量缓存，事务期间，key值监控异常: {}-{}", vid, e.what());
            }catch (const std::exception& e) {
                ERR("向缓存更新视频统计数据点赞量失败: {}-{}", vid, e.what());
                throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
            }
        }
    }

    const std::string VideoCountCache::_cache_prefix = "video_count_";
    const std::string VideoCountCache::_key_play_count = "playCount";
    const std::string VideoCountCache::_key_like_count = "likeCount";
    VideoCountCache::VideoCountCache(const std::shared_ptr<sw::redis::Redis> &redis): _redis(redis) {}
    std::string VideoCountCache::getCacheKey(const std::string &vid) {
        return  _cache_prefix + vid;
    }
    VideoCount::ptr VideoCountCache::getCountFromCache(const std::string &vid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 获取缓存key
            std::string key = getCacheKey(vid);
            //2. 从缓存中获取视频信息
            std::unordered_map<std::string, std::string> count_info;
            redis.hgetall(key, std::inserter(count_info, count_info.begin()));
            if (count_info.empty()) return nullptr;
            //3. 构造VideoCount::ptr对象，填充信息
            VideoCount::ptr count = std::make_shared<VideoCount>();
            count->play_count = std::stoi(count_info[_key_play_count]);
            count->like_count = std::stoi(count_info[_key_like_count]);
            return count;
        } catch (const sw::redis::Error &e) {
            ERR("从redis中获取视频统计数据缓存失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }


}