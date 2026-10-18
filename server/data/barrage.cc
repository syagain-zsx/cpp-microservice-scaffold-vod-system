#include "barrage.h"

namespace vp_data {
    BarrageData::BarrageData(odb::transaction &mtx) :_mysql(mtx.database()) {}
    void BarrageData::insert(Barrage &barrage) {
        try {
            _mysql.persist(barrage);
        }catch (const odb::exception &e) {
            ERR("插入弹幕信息到数据库失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void BarrageData::remove(const std::string &video_id) {
        try {
            typedef odb::query<Barrage> Query;
            _mysql.erase_query<Barrage>(Query::video_id == video_id);
        }catch (const odb::exception &e) {
            ERR("从数据库删除视频弹幕信息失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    std::vector<Barrage::ptr> BarrageData::list(const std::string &video_id) {
        try {
            typedef odb::query<BarragePtr> Query;
            typedef odb::result<BarragePtr> Result;
            Result barrages(_mysql.query<BarragePtr>(Query::video_id == video_id));
            std::vector<Barrage::ptr>  result;
            for (auto &b : barrages) {
                result.push_back(b.barrage);
            }
            return result;
        }catch (const odb::exception &e) {
            ERR("从数据库获取视频弹幕信列表失败: {}", e.what());
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
}