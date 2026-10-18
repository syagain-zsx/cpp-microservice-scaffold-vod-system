#pragma once
#include <bite_scaffold/odb.h>
#include <bite_scaffold/redis.h>
#include <bite_scaffold/mq.h>
#include <bite_scaffold/log.h>
#include <bite_scaffold/util.h>
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>

namespace vp_data {
    class FollowData {
        public:
            using ptr = std::shared_ptr<FollowData>;
            FollowData(odb::transaction &mtx);
            void insert(const std::string &follower_uid, const std::string &followed_uid);
            void remove(const std::string &follower_uid, const std::string &followed_uid);
            bool judge(const std::string &follower_uid, const std::string &followed_uid);
        private:
            odb::database &_mysql;
    };
}