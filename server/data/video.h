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
    class VideoData{
        public:
            VideoData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            void insert(Video &info);
            Video::ptr remove(const std::string &vid);
            Video::ptr get(const std::string &vid);
            Video::ptr getFromDB(const std::string &vid);
            void update(Video::ptr &video);
            std::unordered_map<std::string, Video::ptr> list(std::vector<std::string> &vids);
        private:
            void insertVideoToDB(Video &video);
            Video::ptr getVideoFromDB(const std::string &vid);
            void updateVideoToDB(Video::ptr &video);
            void removeVideoFromDB(const std::string &vid);
            void insertVideoToCache(Video::ptr &video);
            Video::ptr getVideoFromCache(const std::string &vid);
            void removeVideoFromCache(const std::string &vid);
            std::string getCacheKey(const std::string &vid);
            void syncCache(const std::string &vid);
        private:
            static const size_t _expire_time;
            static const std::string _cache_prefix;
            static const std::string _key_video_id;
            static const std::string _key_video_file_id;
            static const std::string _key_cover_file_id;
            static const std::string _key_upload_user_id;
            static const std::string _key_audit_user_id;
            static const std::string _key_video_title;
            static const std::string _key_video_desc;
            static const std::string _key_play_count;
            static const std::string _key_video_duration;
            static const std::string _key_video_size;
            static const std::string _key_online_time;
            static const std::string _key_video_status;
            odb::database& _mysql;
            std::shared_ptr<sw::redis::Redis> _redis;
            vp_sync::CacheSync::ptr _cache_sync;
    };

    class VideoDBData {
        public:
            using ptr = std::shared_ptr<VideoDBData>;
            VideoDBData(odb::transaction &mtx);
            Video::ptr get(const std::string &vid);
            void set(Video::ptr &video);
        private:
            odb::database &_mysql;
    };
}