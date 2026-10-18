#pragma once 
#include <follow.h>
#include <permission_role.h>
#include <session.h>
#include <user_count.h>
#include <user_identify_role.h>
#include <user.h>
#include <verify_code.h>

#include <base.pb.h>

namespace svc_user {
    class SvcData {
        public:
            using ptr = std::shared_ptr<SvcData>;
            SvcData(const std::shared_ptr<sw::redis::Redis> &redis,
                const std::shared_ptr<odb::database> &mysql,
                const vp_sync::CacheSync::ptr &cache_sync);
            //临时用户登录数据处理: 生成会话ID,向数据库新增临时会话信息
            std::string tempLogin();
            //会话用户登录: 获取会话信息, 返回会话是否是临时会话
            bool sessionLogin(const std::string &ssid);
            //获取验证码: 生成验证码和验证码ID,添加验证码信息缓存; 返回验证码ID
            void getVCode(const std::string &ssid, std::string &cid, std::string &code, const std::string &email);
            //注册或登录: 获取验证码信息进行比对;  判断email是否已注册; 未注册-新增用户,新增身份信息,更新会话信息; 已注册-更新会话信息
            void regOrLogin(const std::string &ssid, const std::string &cid, const std::string &code, const std::string &email);
            //用户名登录: 通过用户名获取用户信息; 进行密码比对; 更新会话信息
            void normalLogin(const std::string &ssid, const std::string &username, const std::string &passwd);
            //退出登录: 获取会话信息; 更新会话信息
            std::string logout(const std::string &ssid);
            //修改昵称: 获取会话信息, 更新昵称
            void setNickname(const std::string &ssid, const std::string &name);
            //修改密码: 获取会话信息, 更新密码
            void setPassword(const std::string &ssid, const std::string &passwd);
            //修改头像: 获取会话信息, 更新头像
            std::optional<std::string> setAvatar(const std::string &ssid, const std::string &avatar);
            //修改状态: 获取会话信息; 获取会话用户身份角色判断是否管理员,以及是否启用; 获取目标用户身份角色; 修改用户状态
            void setStatus(const std::string &ssid, const std::string &uid, vp_data::UserStatus status);
            //获取用户详细信息: 获取会话信息; 获取目标用户基础用户信息,获取目标用户身份角色;获取目标用户统计数据;判断会话用户是否关注了目标用户
            vp_data::DetailUser::ptr getUser(const std::string &ssid, const std::optional<std::string> &uid = std::optional<std::string>());
            //新增关注: 获取会话信息; 新增关注关系(判断是否已关注,判断目标是否是自己)
            void newFollow(const std::string &ssid, const std::string &uid);
            //取消关注: 获取会话信息; 删除关注关系
            void delFollow(const std::string &ssid, const std::string &uid);
            //新增管理员: 获取会话信息,获取会话用户身份角色(判断是否是管理员,是否启用, 目标与自己的管理等级); 新增管理员信息/更新普通用户信息,添加身份信息
            void newAdmin(const std::string &ssid, const api_message::AdminInfo &info);
            //编辑管理员: 获取会话信息,获取会话用户身份角色(判断是否是管理员,是否启用, 目标与自己的管理等级); 更新管理员信息
            // 如果更新的是自己的信息,则不能修改自己的状态
            void setAdmin(const std::string &ssid, const api_message::AdminInfo &info);
            //删除管理员: 获取会话信息,获取会话用户身份角色(判断是否是管理员,是否启用, 目标与自己的管理等级); 删除用户身份角色
            void delAdmin(const std::string &ssid, const std::string &uid);
            //获取管理员列表: 获取会话信息(判断是否是管理员,是否启用, 目标与自己的管理等级); 获取管理员列表
            vp_data::StatusIdentifyUserList::ptr getAdminList(const std::string &ssid, vp_data::UserStatus status, size_t page, size_t count);
            //通过邮箱获取管理员: 获取会话信息(判断是否是管理员,是否启用, 目标与自己的管理等级); 获取管理员信息
            vp_data::StatusIdentifyUser::ptr getAdmin(const std::string &ssid, const std::string &email);
        private:
            const int _code_length = 6;
            std::shared_ptr<sw::redis::Redis> _redis;
            std::shared_ptr<odb::database> _mysql;
            vp_sync::CacheSync::ptr _cache_sync;
    };
}