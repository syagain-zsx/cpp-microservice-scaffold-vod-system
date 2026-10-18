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
    class VideoIdListData {
        public:
            using ptr = std::shared_ptr<VideoIdListData>;
            VideoIdListData(odb::transaction &mtx, const std::shared_ptr<sw::redis::Redis> &redis, const vp_sync::CacheSync::ptr &sync);
            std::vector<std::string> getMainPageList(size_t page, size_t count);
            VideoUserStatusIdList::ptr getUserPageList(const std::string &uid, VideoStatus status, size_t page, size_t count);
            std::vector<std::string> getCatgoryPageList(size_t catgory_type, size_t page, size_t count);
            std::vector<std::string> getTagPageList(size_t tag_type, size_t page, size_t count);
            StatusVideoList::ptr getStatusPageList(VideoStatus status, size_t page, size_t count);
            void syncPageListCache(const std::string &vid, const std::string &uid, size_t catgory_type, std::vector<size_t> tag_type);
        private:
            std::string getMainPageKey();
            std::string getUserPageKey(const std::string &uid);
            std::string getCatgoryPageKey(size_t catgory_type);
            std::string getTagPageKey(size_t tag_type);
            void syncCache(const std::string &key); //删除缓存，并发布延迟删除消息

            std::vector<VideoMainIdList> getMainPageListFromDB();
            std::vector<VideoMainIdList> getMainPageListFromDB(size_t page, size_t count);
            void insertMainPageListToCache(const std::vector<VideoMainIdList> &list);
            std::vector<std::string> getMainPageListFromCache(size_t page, size_t count);
            void syncMainPageCache(const std::string &video_id); //缓存数据量小于1000，或者视频ID在列表中

            void syncUserPageCache(const std::string &uid, const std::string &video_id); //缓存数据量小于1000，或者视频ID在列表中
            void syncCatgoryPageCache(size_t catgory_type, const std::string &video_id); //缓存数据量小于1000，或者视频ID在列表中
            void syncTagPageCache(size_t tag_type, const std::string &video_id); //缓存数据量小于1000，或者视频ID在列表中

            std::vector<VideoUserPlayIdList> getUserPageListFromDB(const std::string &uid, VideoStatus status);
            size_t getUserPageListCountFromDB(const std::string &uid, VideoStatus status);
            std::vector<VideoCatgoryPlayIdList> getCatgoryPageListFromDB(size_t catgory_type);
            std::vector<VideoCatgoryPlayIdList> getCatgoryPageListFromDB(size_t catgory_type, size_t page, size_t count);
            std::vector<VideoTagPlayIdList> getTagPageListFromDB(size_t tag_type);
            std::vector<VideoTagPlayIdList> getTagPageListFromDB(size_t tag_type, size_t page, size_t count);
            StatusVideoList::ptr getStatusPageListFromDB(VideoStatus status, size_t page, size_t count);

            void insertUserPageListToCache(const std::string &uid, const std::vector<VideoUserPlayIdList> &list);
            void insertCatgoryPageListToCache(size_t catgory_type, const std::vector<VideoCatgoryPlayIdList> &list);
            void insertTagPageListToCache(size_t tag_type, const std::vector<VideoTagPlayIdList> &list);

            std::vector<std::string> getUserPageListFromCache(const std::string &uid, size_t page, size_t count);
            std::vector<std::string> getCatgoryPageListFromCache(size_t catgory_type, size_t page, size_t count);
            std::vector<std::string> getTagPageListFromCache(size_t tag_type, size_t page, size_t count);
        private:
            static const size_t _list_cache_size;
            static const size_t _expire_time;
            static const std::string _key_main_page_list;
            static const std::string _key_user_page_list_prefix;
            static const std::string _key_catgory_page_list_prefix;
            static const std::string _key_tag_page_list_prefix;
            odb::database& _mysql;
            std::shared_ptr<sw::redis::Redis> _redis;
            vp_sync::CacheSync::ptr _cache_sync;
    };
}