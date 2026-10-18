#pragma once
#include <file.h>
#include <session.h>
#include <error.h>
#include <sync.h>

namespace svc_file {
    class SvcData {
        public:
            using ptr = std::shared_ptr<SvcData>;
            SvcData(const std::shared_ptr<sw::redis::Redis> &redis,
                const std::shared_ptr<odb::database> &mysql,
                const vp_sync::CacheSync::ptr &cache_sync);
            std::string getSessionUser(const std::string &session_id);
            void newFileMeta(vp_data::File &file);
            void removeFileMeta(const std::string &file_id);
            vp_data::File::ptr getFileMeta(const std::string &file_id);
        private:
        private:
            std::shared_ptr<sw::redis::Redis> _redis;
            std::shared_ptr<odb::database> _mysql;
            vp_sync::CacheSync::ptr _cache_sync;
    };
}