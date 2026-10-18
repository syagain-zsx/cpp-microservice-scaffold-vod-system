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
    class VerifyCodeData {
        public:
            using ptr = std::shared_ptr<VerifyCodeData>;
            VerifyCodeData(const std::shared_ptr<sw::redis::Redis> &redis);
            void insert(const std::string &cid, const std::string &ssid, const std::string &code, const std::string &email);
            void remove(const std::string &cid);
            VerifyCode::ptr get(const std::string &cid);
        private:
            std::string getCacheKey(const std::string &cid);
        private:
            static const size_t _expire_time;
            static const std::string _prefix;
            static const std::string _key_ssid;
            static const std::string _key_cid;
            static const std::string _key_code;
            static const std::string _key_email;
            std::shared_ptr<sw::redis::Redis> _redis;
    };
}