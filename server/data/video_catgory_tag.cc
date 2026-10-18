#include "video_catgory_tag.h"


namespace vp_data{
    VideoCatgoryTagData::VideoCatgoryTagData(odb::transaction &mtx): _mysql(mtx.database()) {}
    void VideoCatgoryTagData::insert(const std::string &vid, unsigned int cid, unsigned int tid) {
        try {
            vp_data::VideoCatgoryTag vct(vid, cid, tid);
            _mysql.persist(vct);
        }catch(const odb::exception &e) {
            ERR("向数据库新增视频分类标签关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoCatgoryTagData::remove(const std::string &vid) {
        try {
            typedef odb::query<VideoCatgoryTag> Query;
            _mysql.erase_query<VideoCatgoryTag>(Query::video_id == vid);
        }catch(const odb::exception &e) {
            ERR("从数据库删除视频分类标签关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void VideoCatgoryTagData::getCatgoryTags(const std::string &vid, size_t &cid, std::vector<size_t> &tags) {
        try {
            typedef odb::query<VideoCatgoryTag> Query;
            typedef odb::result<VideoCatgoryTag> Result;
            Result result(_mysql.query<VideoCatgoryTag>(Query::video_id == vid));
            if (result.empty() == false) {
                cid = result.begin()->catgory_id();
            }else {
                WRN("未找到视频的分类标签信息: {}", vid);
                // throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
                return;
            }
            for(auto &r: result) {
                if (cid != r.catgory_id()) {
                    ERR("一个视频仅能有一个分类, 但数据库中存在多个分类: {}-{}-{}", vid, cid, r.catgory_id());
                    throw vp_error::VPException(vp_error::VpError::DATA_ERROR);
                }
                cid = r.catgory_id();
                tags.push_back(r.tag_id());
            }
        }catch (const odb::exception &e) {
            ERR("从数据库获取视频分类标签关系失败: {}-{}", vid, e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
}