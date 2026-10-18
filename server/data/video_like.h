#pragma once
#include <JianLiuScaffold/odb.h>
#include <JianLiuScaffold/redis.h>
#include <JianLiuScaffold/mq.h>
#include <JianLiuScaffold/log.h>
#include <JianLiuScaffold/util.h>
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>

namespace vp_data{
    class VideoLikeData {
        public:
            using ptr = std::shared_ptr<VideoLikeData>;
            VideoLikeData(odb::transaction &mtx);
            void insert(const std::string &vid, const std::string &uid);
            void cancel(const std::string &vid, const std::string &uid);
            void remove(const std::string &vid);
            bool judge(const std::string &vid, const std::string &uid);
        private:
            odb::database &_mysql;
    };
}