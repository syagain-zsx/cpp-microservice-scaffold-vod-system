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
    class VideoCatgoryTagData {
        public:
            using ptr = std::shared_ptr<VideoCatgoryTagData>;
            VideoCatgoryTagData(odb::transaction &mtx);
            void insert(const std::string &vid, unsigned int cid, unsigned int tid);
            void remove(const std::string &vid);
            void getCatgoryTags(const std::string &vid, size_t &cid, std::vector<size_t> &tags);
        private:
            odb::database &_mysql;
    };
}