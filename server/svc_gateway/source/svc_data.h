#pragma once 
#include <session.h>
#include <user_identify_role.h>
#include <permission_role.h>

namespace svc_gateway {
    class SvcData {
        public:
            using ptr = std::shared_ptr<SvcData>;
            SvcData(const std::shared_ptr<sw::redis::Redis> &redis, const std::shared_ptr<odb::database> &mysql);
            void judgePermission(const std::string &session_id, const std::string &url);
        private:
            std::shared_ptr<sw::redis::Redis> _redis;
            std::shared_ptr<odb::database> _mysql;
    };
}