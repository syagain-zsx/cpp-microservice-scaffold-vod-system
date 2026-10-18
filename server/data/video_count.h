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
    class VideoCountData{
        public:
            using ptr = std::shared_ptr<VideoCountData>;
            VideoCountData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            VideoCount::ptr getCount(const std::string &vid);
            // count时要增加/减少的数据，而不是要设置的数据
            void updateVideoPlayCount(const std::string &vid, int count);
            void updateVideoLikeCount(const std::string &vid, int count);
        private:
            void updatePlayCountToCache(const std::string &vid, int count);
            void updateLikeCountToCache(const std::string &vid, int count);
            VideoCount::ptr getCountFromDB(const std::string &vid);
            VideoCount::ptr getCountFromCache(const std::string &vid);
            void insertCountToCache(const std::string &vid, VideoCount::ptr &count);
            void removeCountFromCache(const std::string &vid);
            void syncCache(const std::string &vid);
            std::string getCacheKey(const std::string &vid);
        private:
            static const std::string _cache_prefix;
            static const std::string _key_play_count;
            static const std::string _key_like_count;
            odb::database& _mysql;
            std::shared_ptr<sw::redis::Redis> _redis;
            vp_sync::CacheSync::ptr _cache_sync;
    };

    class VideoCountCache {
        public:
            using ptr = std::shared_ptr<VideoCountCache>;
            VideoCountCache(const std::shared_ptr<sw::redis::Redis> &redis);
            VideoCount::ptr getCountFromCache(const std::string &vid);
            static std::string getCacheKey(const std::string &vid);
        private:
            static const std::string _cache_prefix;
            static const std::string _key_play_count;
            static const std::string _key_like_count;
            std::shared_ptr<sw::redis::Redis> _redis;
    };

}