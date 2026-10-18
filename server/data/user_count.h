#pragma once
#include <bite_scaffold/odb.h>
#include <bite_scaffold/redis.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include <bite_scaffold/util.h>
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>


namespace vp_data{
    class UserCountData {
        public:
            using ptr = std::shared_ptr<UserCountData>;
            UserCountData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis);
            UserCount::ptr getCount(const std::string &uid);
            void updatePlayCount(const std::string &uid, int count);
            void updateLikeCount(const std::string &uid, int count);
            void updateFollowerCount(const std::string &uid, int count);
            void updateFollowedCount(const std::string &uid, int count);
        private:
            UserCount::ptr getCountFromDb(const std::string &uid);
            UserCount::ptr getCountFromCache(const std::string &uid);
            void insertToCache(const std::string &uid, const UserCount::ptr &uc);
            void updateCountToCache(const std::string &uid, const std::string &ckey, int count);
            std::string getCacheKey(const std::string &uid);
        private:
            static const size_t _expire_time;
            static const std::string _prefix;
            static const std::string _play_ckey;
            static const std::string _like_ckey;
            static const std::string _follower_ckey;
            static const std::string _followed_ckey;
            odb::database &_mysql;
            std::shared_ptr<sw::redis::Redis> _redis;
    };
}