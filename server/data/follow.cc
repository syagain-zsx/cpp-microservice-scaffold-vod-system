#include "follow.h"

namespace vp_data{
    FollowData::FollowData(odb::transaction &mtx):_mysql(mtx.database()){}
    void FollowData::insert(const std::string &follower_uid, const std::string &followed_uid) {
        try {
            Follow follow(follower_uid, followed_uid);
            _mysql.persist(follow);
        }catch (const std::exception &e) {
            ERR("向数据库新增关注关系失败: {}-{}", follower_uid, followed_uid);
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    void FollowData::remove(const std::string &follower_uid, const std::string &followed_uid) {
        try {
            typedef odb::query<Follow> Query;
            _mysql.erase_query<Follow>(Query::follower_uid == follower_uid && Query::followed_uid == followed_uid);
        } catch (const std::exception &e) {
            ERR("从数据库删除关注关系失败: {}-{}", follower_uid, followed_uid);
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
    }
    bool FollowData::judge(const std::string &follower_uid, const std::string &followed_uid) {
        try {
            typedef odb::query<Follow> Query;
            typedef odb::result<Follow> Result;
            auto res = _mysql.query<Follow>(Query::follower_uid == follower_uid && Query::followed_uid == followed_uid);
            return res.size() != 0;
        }catch (const std::exception &e) {
            ERR("从数据库判断关注关系失败: {}-{}", follower_uid, followed_uid);
            throw vp_error::VPException(vp_error::VpError::DATABASE_OP_FAILED);
        }
        return false; // should never reach here, but just in case it doesn‘t
    }
}