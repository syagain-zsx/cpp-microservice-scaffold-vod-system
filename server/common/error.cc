#include "error.h"
namespace vp_error {
    
const char *vp_error_to_string(VpError err) {
    switch (err) {
        case VpError::SUCCESS: return "处理成功";
        case VpError::INVALID_SESSION: return "会话无效";
        case VpError::UPLOAD_FILE_TO_FDFS_FAILED: return "文件上传失败";
        case VpError::DATABASE_OP_FAILED: return "数据操作失败";
        case VpError::FILE_NOT_EXIST: return "文件不存在";
        case VpError::DOWNLOAD_FILE_FROM_FDFS_FAILED: return "文件下载失败";
        case VpError::REDIS_OP_FAILED: return "redis操作失败";
        case VpError::MQ_OP_FAILED: return "mq操作失败";
        case VpError::USER_NOT_EXIST: return "用户不存在";
        case VpError::SERVER_INERNAL_ERROR: return "服务器内部错误";
        case VpError::SESSION_INVALID: return "无效会话";
        case VpError::VERIFY_CODE_ERROR: return "验证码错误";
        case VpError::SESSION_ALREADY_LOGIN: return "会话已登录";
        case VpError::EMAIL_USER_NOT_EXISTS: return "未找到邮箱用户";
        case VpError::USERNAME_OR_PASSWD_ERROR: return "用户名密码错误";
        case VpError::SESSION_NOT_LOGIN: return "会话未登录";
        case VpError::PERMISSION_DENIED: return "权限不足";
        case VpError::USER_COUNT_NOT_EXISTS: return "用户统计数据不存在";
        case VpError::USER_IDENTIFY_ROLE_NOT_EXISTS: return "用户身份角色不存在";
        case VpError::USER_ALREADY_FOLLOWING: return "目标用户已关注";
        case VpError::USER_NOT_FOLLOWING: return "目标用户未关注";
        case VpError::MAIL_CODE_ERROR: return "邮箱验证码发送失败";
        case VpError::USER_ALREADY_HAS_ADMIN_IDENTIFY: return "用户已有管理员身份";
        case VpError::USER_ALREADY_HAS_NOT_ADMIN_IDENTIFY: return "用户没有管理员身份";
        case VpError::DATA_ERROR: return "数据错误";
        case VpError::ELASTIC_OP_ERROR: return "搜索引擎操作失败";
        case VpError::VIDEO_NOT_EXISTS: return "视频不存在";
        case VpError::TRANSCODE_FAILED: return "转码失败";
        case VpError::SERVICE_NOT_FOUND: return "服务未找到";
        case VpError::RPC_REQUEST_FAILED: return "RPC请求失败";
        case VpError::CLIENT_REQUEST_PARAM_ERROR: return "客户端请求参数错误";
        case VpError::NICKNAME_IS_EXISTS: return "昵称已存在";
        case VpError::NEW_PASSWD_SAME_OLD: return "新密码与旧密码相同";
    }
    return "未知错误";
}

VPException::VPException(VpError err) : _err(err) {}
const char *VPException::what() const noexcept {
    return vp_error_to_string(_err);
}
VpError VPException::errCode() const noexcept {
    return _err;
}
}