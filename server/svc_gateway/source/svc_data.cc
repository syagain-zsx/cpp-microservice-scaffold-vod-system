#include "svc_data.h"


namespace svc_gateway {
    SvcData::SvcData(const std::shared_ptr<sw::redis::Redis> &redis, const std::shared_ptr<odb::database> &mysql)
    : _redis(redis) , _mysql(mysql) {}
    void SvcData::judgePermission(const std::string &session_id, const std::string &url) {
        try {
            odb::transaction mtx(_mysql->begin());
            //2. 通过url获取授权角色信息
            vp_data::PermissionRoleData permission_role_data(mtx, _redis);
            auto roles = permission_role_data.list(url);
            //3. 如果url的授权角色是0，那么不需要授权，直接返回true
            if (roles.find(vp_data::RoleType::UNKNOWN) != roles.end()) {
                return;
            }
            //1. 通过会话ID获取用户ID（可能不存在，也可能存在）
            vp_data::SessionData session_data(mtx, _redis);
            auto ss = session_data.selectBySid(session_id);
            if (!ss) {
                throw vp_error::VPException(vp_error::VpError::SESSION_INVALID);
            }
            //4. 通过用户ID获取用户身份角色信息 （前提是用户存在）
            std::vector<vp_data::RoleType> user_roles;
            if (!ss->user_id()) {
                user_roles.push_back(vp_data::RoleType::USER_GUEST); //若会话用户不存在则会话角色就是访客
            }else {
                vp_data::UserIdentifyRoleData user_identify_role_data(mtx, _redis);
                auto user_identify_roles = user_identify_role_data.list(ss->user_id().get());
                for (auto &user_identify_role : user_identify_roles) {
                    user_roles.push_back(user_identify_role.role_type());
                }
            }
            //5. 判断用户身份角色是否在授权角色中
            for (auto &user_role : user_roles) {
                if (roles.find(user_role) != roles.end()) {
                    return;
                }
            }
            WRN("用户权限不足: {} - {}", session_id, url);
            throw vp_error::VPException(vp_error::VpError::PERMISSION_DENIED);
        } catch (const vp_error::VPException &e) {
            throw ;
        } catch (const std::exception &e) {
            throw vp_error::VPException(vp_error::VpError::SERVER_INERNAL_ERROR);
        }
    }
}