#pragma once
#include <string>

namespace vp_error {

enum class VpError {
    SUCCESS = 0,
    INVALID_SESSION,
    UPLOAD_FILE_TO_FDFS_FAILED,
    DATABASE_OP_FAILED,
    REDIS_OP_FAILED,
    MQ_OP_FAILED,
    FILE_NOT_EXIST,
    DOWNLOAD_FILE_FROM_FDFS_FAILED,
    USER_NOT_EXIST,
    SERVER_INERNAL_ERROR,
    SESSION_INVALID,
    VERIFY_CODE_ERROR,
    SESSION_ALREADY_LOGIN,
    EMAIL_USER_NOT_EXISTS,
    USERNAME_OR_PASSWD_ERROR,
    SESSION_NOT_LOGIN,
    PERMISSION_DENIED,
    USER_COUNT_NOT_EXISTS,
    USER_IDENTIFY_ROLE_NOT_EXISTS,
    USER_ALREADY_FOLLOWING,
    USER_NOT_FOLLOWING,
    MAIL_CODE_ERROR,
    USER_ALREADY_HAS_ADMIN_IDENTIFY,
    USER_ALREADY_HAS_NOT_ADMIN_IDENTIFY,
    DATA_ERROR,
    ELASTIC_OP_ERROR,
    VIDEO_NOT_EXISTS,
    TRANSCODE_FAILED,
    SERVICE_NOT_FOUND,
    RPC_REQUEST_FAILED,
    CLIENT_REQUEST_PARAM_ERROR,
    NICKNAME_IS_EXISTS,
    NEW_PASSWD_SAME_OLD,
};

extern const char *vp_error_to_string(VpError err);

class VPException : public std::exception {
    public:
        VPException(VpError err);
        const char *what() const noexcept override;
        VpError errCode() const noexcept ;
    private:
        VpError _err;
};
}