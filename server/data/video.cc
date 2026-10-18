#include "video.h"

namespace vp_data{
    const size_t VideoData::_expire_time = 3600;
    const std::string VideoData::_cache_prefix = "cahce_video_";
    const std::string VideoData::_key_video_id = "videoId";
    const std::string VideoData::_key_video_file_id = "videoFileId";
    const std::string VideoData::_key_cover_file_id = "photoFileId";
    const std::string VideoData::_key_upload_user_id = "userId";
    const std::string VideoData::_key_audit_user_id = "checkerId";
    const std::string VideoData::_key_video_title = "videoTitle";
    const std::string VideoData::_key_video_desc = "videoDesc";
    const std::string VideoData::_key_video_duration = "videoDuration";
    const std::string VideoData::_key_video_size = "videoSize";
    const std::string VideoData::_key_online_time = "videoUpTime";
    const std::string VideoData::_key_video_status = "videoStatus";

    VideoData::VideoData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync)
        : _mysql(mtx.database()), _redis(redis), _cache_sync(sync) {}
    void VideoData::insert(Video &info) {
        insertVideoToDB(info);
    }
    Video::ptr VideoData::remove(const std::string &vid) {
        Video::ptr video = get(vid);
        DBG("获取信息完毕");
        if(video) {
            removeVideoFromDB(vid);
            DBG("删除数据库数据完毕");
            removeVideoFromCache(vid);
            DBG("删除缓存数据完毕");
            syncCache(vid);
            DBG("缓存同步完毕");
        }
        return video;
    }
    Video::ptr VideoData::get(const std::string &vid) {
        Video::ptr video = getVideoFromCache(vid);
        if (video) return video;
        video = getVideoFromDB(vid);
        if (video) {
            insertVideoToCache(video);
        }else {
            WRN("未找到指定ID的视频信息: {}", vid);
        }
        return video;
    }
    Video::ptr VideoData::getFromDB(const std::string &vid) {
        return getVideoFromDB(vid);
    }
    void VideoData::update(Video::ptr &video) {
        updateVideoToDB(video);
        removeVideoFromCache(video->video_id());
        syncCache(video->video_id());
    }
    std::unordered_map<std::string, Video::ptr> VideoData::list(std::vector<std::string> &vids) {
        std::unordered_map<std::string, Video::ptr> result;
        std::vector<std::string> cache_vids;
        for (auto &vid : vids) {
            Video::ptr video = get(vid);
            if (video) {
                result[vid] = video;
            } else {
                WRN("未找到指定ID的视频信息: {}", vid);
            }
        }
        return result;
    }

    void VideoData::insertVideoToDB(Video &video) {
        try {
            _mysql.persist(video);
        }catch (const odb::exception &e) {
            ERR("插入视频信息到数据库失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    Video::ptr VideoData::getVideoFromDB(const std::string &vid) {
        try {
            typedef odb::query<Video> Query;
            Video::ptr result(_mysql.query_one<Video>(Query::video_id == vid));
            return result;
        }catch (const odb::exception &e) {
            ERR("从数据库获取视频信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoData::updateVideoToDB(Video::ptr &video) {
        try {
            _mysql.update(*video);
        }catch (const odb::exception &e) {
            ERR("更新视频信息到数据库失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoData::removeVideoFromDB(const std::string &vid) {
        try {
            typedef odb::query<Video> Query;
            _mysql.erase_query<Video>(Query::video_id == vid);
        }catch (const odb::exception &e) {
            ERR("从数据库删除视频信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoData::insertVideoToCache(Video::ptr &video) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            // 1. 获取缓存key
            std::string key = getCacheKey(video->video_id());
            // 2. 根据video中的信息，组织unordered_map
            std::unordered_map<std::string, std::string> video_info;
            video_info[_key_video_id] = video->video_id();
            video_info[_key_video_file_id] = video->video_fid();
            video_info[_key_cover_file_id] = video->cover_fid();
            video_info[_key_upload_user_id] = video->uploader_uid();
            video_info[_key_video_title] = video->title();
            video_info[_key_video_duration] = std::to_string(video->duration());
            video_info[_key_video_size] = std::to_string(video->size());
            video_info[_key_online_time] = boost::posix_time::to_simple_string(video->upload_time());
            video_info[_key_video_status] = std::to_string((int)video->status());
            if (video->auditor_uid()) video_info[_key_audit_user_id] = video->auditor_uid().get();
            if (video->summary()) video_info[_key_video_desc] = video->summary().get();
            // 3. 添加reids缓存
            redis.hmset(key, video_info.begin(), video_info.end());
            // 4. 设置缓存过期时间
            redis.expire(key, _expire_time + biteutil::Random::number(0, _expire_time));
        }catch (const sw::redis::Error &e) {
            ERR("插入视频信息到缓存失败: {}-{}", video->video_id(), e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    Video::ptr VideoData::getVideoFromCache(const std::string &vid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            //1. 获取缓存key
            std::string key = getCacheKey(vid);
            //2. 从缓存中获取视频信息
            std::unordered_map<std::string, std::string> video_info;
            redis.hgetall(key, std::inserter(video_info, video_info.begin()));
            if (video_info.empty()) return nullptr;
            //3. 构造Video::ptr对象，填充信息
            Video::ptr video = std::make_shared<Video>();
            video->set_video_id(video_info[_key_video_id]);
            video->set_video_fid(video_info[_key_video_file_id]);
            video->set_cover_fid(video_info[_key_cover_file_id]);
            video->set_uploader_uid(video_info[_key_upload_user_id]);
            video->set_title(video_info[_key_video_title]);
            video->set_duration(std::stoi(video_info[_key_video_duration]));
            video->set_size(std::stoi(video_info[_key_video_size]));
            video->set_upload_time(boost::posix_time::time_from_string(video_info[_key_online_time]));
            video->set_status((VideoStatus)std::stoi(video_info[_key_video_status]));
            if (video_info.find(_key_audit_user_id) != video_info.end()) 
                video->set_auditor_uid(video_info[_key_audit_user_id]);
            if (video_info.find(_key_video_desc) != video_info.end()) 
                video->set_summary(video_info[_key_video_desc]);
            //4. 返回对象
            return video;
        }catch (const sw::redis::Error &e) {
            ERR("从缓存获取视频信息失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    void VideoData::removeVideoFromCache(const std::string &vid) {
        try {
            auto rtx = _redis->transaction(false, false);
            auto redis = rtx.redis();
            std::string key = getCacheKey(vid);
            redis.del(key);
        }catch (const sw::redis::Error &e) {
            ERR("从缓存删除视频信息失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::REDIS_OP_FAILED);
        }
    }
    std::string VideoData::getCacheKey(const std::string &vid) {
        return _cache_prefix + vid;
    }
    void VideoData::syncCache(const std::string &vid) {
        //1. 组织缓存key
        std::string key = getCacheKey(vid);
        //3. 发布延迟同步删除消息
        _cache_sync->sync(key);
    }



    VideoDBData::VideoDBData(odb::transaction &mtx): _mysql(mtx.database()) {}
    Video::ptr VideoDBData::get(const std::string &vid) {
        try {
            typedef odb::query<Video> Query;
            Video::ptr result(_mysql.query_one<Video>(Query::video_id == vid));
            if (!result) {
                ERR("从数据库获取视频信息失败: {}", vid);
                throw vp_error::VPException(vp_error::VpError::VIDEO_NOT_EXISTS);
            }
            return result;
        }catch (const odb::exception &e) {
            ERR("从数据库获取视频信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoDBData::set(Video::ptr &video) {
        try {
            _mysql.update(*video);
        }catch (const odb::exception &e) {
            ERR("更新视频信息到数据库失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
}