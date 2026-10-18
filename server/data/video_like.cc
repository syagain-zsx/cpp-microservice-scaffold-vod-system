#include "video_like.h"

namespace vp_data{
    VideoLikeData::VideoLikeData(odb::transaction &mtx): _mysql(mtx.database()){}
    void VideoLikeData::insert(const std::string &vid, const std::string &uid) {
        try {
            VideoLike vl(uid, vid);
            _mysql.persist(vl);
        }catch(const odb::exception &e) {
            ERR("向数据库新增视频点赞关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoLikeData::cancel(const std::string &vid, const std::string &uid) {
        try {
            typedef odb::query<VideoLike> Query;
            _mysql.erase_query<VideoLike>(Query::video_id == vid && Query::user_id == uid);
        }catch(const odb::exception &e) {
            ERR("从数据库取消视频点赞关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoLikeData::remove(const std::string &vid) {
        try {
            typedef odb::query<VideoLike> Query;
            _mysql.erase_query<VideoLike>(Query::video_id == vid);
        }catch(const odb::exception &e) {
            ERR("从数据库删除视频点赞关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    bool VideoLikeData::judge(const std::string &vid, const std::string &uid) {
        try {
            typedef odb::query<VideoLike> Query;
            // typedef odb::result<VideoLike> Result;
            VideoLike::ptr result(_mysql.query_one<VideoLike>(Query::video_id == vid && Query::user_id == uid));
            return result.get() != nullptr;
        }catch(const odb::exception &e) {
            ERR("从数据库判断视频点赞关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
}