#pragma once 
#include <barrage.h>
#include <video_catgory_tag.h>
#include <video_count.h>
#include <video_idlist.h>
#include <video_like.h>
#include <video_search.h>
#include <video.h>
#include <session.h>
#include <user_identify_role.h>
#include <user_count.h>
#include <user.h>

#include <base.pb.h>
#include <video.pb.h>
#include "svc_sync.h"

namespace svc_video {
    class SvcData {
        public:
            using ptr = std::shared_ptr<SvcData>;
            SvcData(const std::shared_ptr<sw::redis::Redis> &redis, 
                const std::shared_ptr<odb::database> &mysql,
                const biteics::ESClient::ptr &es_client,
                const vp_sync::CacheSync::ptr &cache_to_db_sync,
                const vp_sync::CacheSync::ptr &cache_delete_sync,
                const bitemq::Publisher::ptr &del_file_publisher,
                const bitemq::Publisher::ptr &transcode_publisher);
            void newVideo(const std::string &ssid, const api_message::NewVideoInfo &info);
            void delVideo(const std::string &ssid, const std::string &vid);
            bool judgeLike(const std::string &ssid, const std::string &vid);
            void likeVideo(const std::string &ssid, const std::string &vid);
            void playVideo(const std::string &ssid, const std::string &vid);
            void newBarrage(const std::string &ssid, const api_message::BarrageInfo &info);
            std::vector<api_message::BarrageInfo> getBarrage(const std::string &ssid, const std::string &vid);
            void checkVideo(const std::string &ssid, const std::string &vid, bool result);
            void onlineVideo(const std::string &ssid, const std::string &vid);
            void offlineVideo(const std::string &ssid, const std::string &vid);
            api_message::userVideoListResult getUserPageList(const std::string &ssid, 
                const std::optional<std::string> &uid, 
                vp_data::VideoStatus status,
                size_t page, size_t count);
            std::vector<api_message::VideoInfo> getStatusPageList(vp_data::VideoStatus status, size_t page, size_t count, size_t *total);
            std::vector<api_message::VideoInfo> getMainPageList(size_t page, size_t count);
            std::vector<api_message::VideoInfo> getCatgoryPageList(size_t catgory_id, size_t page, size_t count);
            std::vector<api_message::VideoInfo> getTagPageList(size_t tag_id, size_t page, size_t count);
            std::vector<api_message::VideoInfo> getSearchPageList(const std::string &search_key, size_t page, size_t count);
        private:
            std::string getSessionUser(const std::string &ssid, odb::transaction &mtx);
            std::vector<api_message::VideoInfo> getDetailVideoList(std::vector<std::string> &vid_list, odb::transaction &mtx);
        private:
            const int _uuid_length = 16;
            std::shared_ptr<sw::redis::Redis> _redis;
            std::shared_ptr<odb::database> _mysql;
            biteics::ESClient::ptr _es_client;
            vp_sync::CacheSync::ptr _cache_to_db_sync;
            vp_sync::CacheSync::ptr _cache_delete_sync;
            bitemq::Publisher::ptr _del_file_publisher;
            bitemq::Publisher::ptr _transcode_publisher;
    };
}
