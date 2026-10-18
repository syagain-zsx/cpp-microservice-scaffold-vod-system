#pragma once
#include <bite_scaffold/log.h>
#include <bite_scaffold/util.h>
#include <bite_scaffold/elastic.h>
#include "../common/error.h"

#include "data.h"
#include <data-odb.hxx>

namespace vp_data{
    class VideoSearchData {
        public:
            using ptr = std::shared_ptr<VideoSearchData>;
            //构造函数中不要忘了创建索引
            VideoSearchData(const biteics::ESClient::ptr &es_client);
            static void init(const biteics::ESClient::ptr &es_client);
            void insert(const std::string &vid, const std::string &title, const std::string &desc, bool status);
            void update(const std::string &vid, const std::string &title, const std::string &desc, bool status);
            void remove(const std::string &vid);
            void remove();
            std::vector<std::string> search(const std::string &key, size_t page, size_t count);
        private:
            static const std::string _index_name;
            static const float _title_boost;
            static const float _desc_boost;
            static const std::string _key_video_id;
            static const std::string _key_video_title;
            static const std::string _key_video_desc;
            static const std::string _key_video_status;
            // Elasticsearch 不可用时置为 false：搜索功能降级，其余业务不受影响
            static bool _enabled;
            biteics::ESClient::ptr _es_client;
    };
}
