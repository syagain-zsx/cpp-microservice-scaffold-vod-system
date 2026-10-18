#pragma once
#include <string>
#include <cstddef> // std::size_t
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/nullable.hxx>
#include <odb/core.hxx>
#include <optional>

namespace vp_data{

enum class IdentifyType {
    UNKNOWN = 0,
    NORMAL = 1,
    ADMIN = 2
};
//超级管理员， 管理员，登录用户，未登录用户
enum class RoleType {
    UNKNOWN = 0,
    ADMIN_SUPER = 1,
    ADMIN_NORMAL = 2,
    USER_NORNAL = 3,
    USER_GUEST = 4
};
enum class UserStatus {
    UNKNOWN,
    ENABLE = 1,
    DISABLE = 2
};

//未知，审核中，审核通过上架中，审核驳回，下架中，转码中，转码失败
enum class VideoStatus {
    UNKNOWN = 0,
    AUDITING = 1,
    ONLINE = 2,
    REJECT = 3,
    OFFLINE = 4,
    TRANSCODING = 5,
    TRANSCODE_FAILED = 6
};

#pragma db object table("tbl_session_meta")
class Session {
    public:
        using ptr = std::shared_ptr<Session>;
        Session();
        Session(const std::string &session_id);
        Session(const std::string &session_id, const std::string &user_id);
        size_t id() const;
        void set_id(size_t id);
        const std::string &session_id() const;
        void set_session_id(const std::string &session_id);
        odb::nullable<std::string> user_id() const;
        void set_user_id(const odb::nullable<std::string> &user_id);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db unique type("VARCHAR(64)")
        std::string _session_id;
        #pragma db index type("VARCHAR(64)")
        odb::nullable<std::string> _user_id;
};

#pragma db view object(Session) \
                query((?))
struct SessionPtr {
    using ptr = std::shared_ptr<SessionPtr>;
    std::shared_ptr<Session> session;
};

#pragma db object table("tbl_file_meta")
class File {
    public:
        using ptr = std::shared_ptr<File>;
        File();
        File(const std::string &file_id, const std::string &uploader_uid, const std::string &path, size_t size, const std::string &mime);
        size_t id() const;
        void set_id(size_t id);
        const std::string &file_id() const;
        void set_file_id(const std::string &file_id);
        const std::string &uploader_uid() const;
        void set_uploader_uid(const std::string &uploader_uid);
        const std::string &path() const;
        void set_path(const std::string &path);
        size_t size() const;
        void set_size(size_t size);
        const std::string &mime() const;
        void set_mime(const std::string &mime);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db unique type("VARCHAR(64)")
        std::string _file_id;       //文件ID
        #pragma db index type("VARCHAR(64)")
        std::string _uploader_uid;  //上传用户ID
        #pragma db type("VARCHAR(64)")
        std::string _path;          //存储路径
        size_t _size;               //文件大小
        #pragma db type("VARCHAR(32)")
        std::string _mime;          //文件mime
};


#pragma db object table("tbl_identify_meta")
class Identify {
    public:
        using ptr = std::shared_ptr<Identify>;
        Identify();
        Identify(const std::string &identify_id, IdentifyType type, const std::string &name);
        size_t id() const;
        const std::string &identify_id() const;
        IdentifyType type() const; 
        const std::string &name() const;

        void set_id(size_t id);
        void set_identify_id(const std::string &identify_id);
        void set_type(IdentifyType type);
        void set_name(const std::string &name);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db unique type("VARCHAR(64)")
        std::string _identify_id;
        #pragma db unique type("TINYINT")
        IdentifyType _type;
        #pragma db type("VARCHAR(64)")
        std::string _name;
};


#pragma db object table("tbl_role_meta")
class Role {
    public:
        using ptr = std::shared_ptr<Role>;
        Role();
        Role(const std::string &role_id, RoleType type, const std::string &name);
        size_t id() const;
        const std::string &role_id() const;
        RoleType type() const;
        const std::string &name() const;

        void set_id(size_t id);
        void set_role_id(const std::string &role_id);
        void set_type(RoleType type);
        void set_name(const std::string &name);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db unique type("VARCHAR(64)")
        std::string _role_id;
        #pragma db unique type("TINYINT")
        RoleType _type;
        #pragma db type("VARCHAR(64)")
        std::string _name;
};


#pragma db object table("tbl_operator_meta")
class Operator {
    public:
        using ptr = std::shared_ptr<Operator>;
        Operator();
        Operator(const std::string &operator_id, const std::string &op_url, const std::string &name);
        size_t id() const;
        const std::string &operator_id() const;
        const std::string &op_url() const;
        const std::string &name() const;

        void set_id(size_t id);
        void set_operator_id(const std::string &operator_id);
        void set_op_url(const std::string &op_url);
        void set_name(const std::string &name);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db unique type("VARCHAR(64)")
        std::string _operator_id;
        #pragma db unique type("VARCHAR(64)")
        std::string _op_url;
        #pragma db type("VARCHAR(64)")
        std::string _name;
};


#pragma db object table("tbl_user_meta")
class User {
    public:
        using ptr = std::shared_ptr<User>;
        User();
        User(const std::string &user_id, const std::string &email);
        size_t id() const;
        const std::string &user_id() const;
        const std::string &email() const;
        const std::string &nickname() const;
        UserStatus status() const;
        const odb::nullable<std::string> &bakname() const;
        const odb::nullable<std::string> &phone() const;
        const odb::nullable<std::string> &password() const;
        const odb::nullable<std::string> &avatar() const;
        const odb::nullable<std::string> &remark() const;
        const boost::posix_time::ptime &reg_time() const;

        void set_id(size_t id);
        void set_user_id(const std::string &user_id);
        void set_email(const std::string &email);
        void set_nickname(const std::string &nickname);
        void set_status(UserStatus status);
        void set_bakname(const odb::nullable<std::string> &bakname);
        void set_phone(const odb::nullable<std::string> &phone);
        void set_password(const odb::nullable<std::string> &password);
        void set_avatar(const odb::nullable<std::string> &avatar);
        void set_remark(const odb::nullable<std::string> &remark);
        void set_reg_time(const boost::posix_time::ptime &reg_time);
    private:    
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        // 用户ID，手机号，邮箱，密码，昵称，备用昵称，状态，头像文件ID，备注信息，注册时间
        #pragma db unique type("VARCHAR(64)")
        std::string _user_id;
        #pragma db unique type("VARCHAR(64)")
        std::string _email;
        #pragma db unique type("VARCHAR(64)")
        std::string _nickname;
        #pragma db unique type("VARCHAR(64)")
        odb::nullable<std::string> _bakname;
        #pragma db unique type("VARCHAR(64)")
        odb::nullable<std::string> _phone;
        #pragma db type("VARCHAR(64)") 
        odb::nullable<std::string> _password;
        #pragma db index type("TINYINT")
        UserStatus _status;
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _avatar;
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _remark;
        #pragma db type("DATETIME")
        boost::posix_time::ptime _reg_time;
};





#pragma db object table("tbl_user_identify_role_meta")
class UserIdentifyRole {
    public:
        using ptr = std::shared_ptr<UserIdentifyRole>;
        UserIdentifyRole();
        UserIdentifyRole(const std::string &user_id, IdentifyType identify_type, RoleType role_type);
        size_t id() const;
        const std::string &user_id() const;
        IdentifyType identify_type() const;
        RoleType role_type() const;
        void set_id(size_t id);
        void set_user_id(const std::string &user_id);
        void set_identify_type(IdentifyType identify_type);
        void set_role_type(RoleType role_type);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        // 用户ID， 身份类型，角色类型
        #pragma db index type("VARCHAR(64)")
        std::string _user_id;
        #pragma db index type("TINYINT")
        IdentifyType _identify_type;
        #pragma db index type("TINYINT")
        RoleType _role_type;
};


#pragma db object table("tbl_operator_role_meta")
class OperatorRole {
    public:
        using ptr = std::shared_ptr<OperatorRole>;
        OperatorRole();
        OperatorRole(const std::string &operator_url, RoleType role_type);
        size_t id() const;
        const std::string &operator_url() const;
        RoleType role_type() const;
        void set_id(size_t id);
        void set_operator_url(const std::string &operator_url);
        void set_role_type(RoleType role_type);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db index type("VARCHAR(64)")
        std::string _operator_url;
        #pragma db index type("TINYINT")
        RoleType _role_type;
};


//用户关注关系表
#pragma db object table("tbl_follow_meta")
class Follow {
    public:
        using ptr = std::shared_ptr<Follow>;
        Follow();
        Follow(const std::string &follower_uid, const std::string &followed_uid);
        size_t id() const;
        const std::string &follower_uid() const;
        const std::string &followed_uid() const;

        void set_id(size_t id);
        void set_follower_uid(const std::string &follower_uid);
        void set_followed_uid(const std::string &followed_uid);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db index type("VARCHAR(64)")
        std::string _follower_uid;
        #pragma db index type("VARCHAR(64)")
        std::string _followed_uid;
};





#pragma db object table("tbl_video_meta")
class Video {
    public:
        using ptr = std::shared_ptr<Video>;
        Video();
        Video(const std::string &video_id, const std::string &title, 
            const std::string &summary, 
            const std::string &cover_fid, 
            const std::string &video_fid, 
            const std::string &uploader_uid, 
            size_t duration, size_t size);
            size_t id() const;
            const std::string &video_id() const;
            const std::string &title() const;
            const odb::nullable<std::string> &summary() const;
            const std::string &cover_fid() const;
            const std::string &video_fid() const;
            const std::string &uploader_uid() const;
            const odb::nullable<std::string> &auditor_uid() const;
            size_t play_count() const;
            size_t duration() const;
            size_t size() const;
            VideoStatus status() const;
            const boost::posix_time::ptime &upload_time() const;

            void set_id(size_t id);
            void set_video_id(const std::string &video_id);
            void set_title(const std::string &title);
            void set_summary(const odb::nullable<std::string> &summary);
            void set_cover_fid(const std::string &cover_fid);
            void set_video_fid(const std::string &video_fid);
            void set_uploader_uid(const std::string &uploader_uid);
            void set_auditor_uid(const odb::nullable<std::string> &auditor_uid);
            void set_play_count(size_t play_count);
            void set_duration(size_t duration);
            void set_size(size_t size);
            void set_status(VideoStatus status);
            void set_upload_time(const boost::posix_time::ptime &upload_time);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        //视频ID，视频标题，视频简介，视频封面文件ID，视频数据文件ID，视频上传用户ID， 审核源用户ID，播放量，视频时长，视频大小，视频状态，上传时间
        #pragma db unique type("VARCHAR(64)")
        std::string _video_id;
        #pragma db type("VARCHAR(64)")
        std::string _cover_fid;
        #pragma db type("VARCHAR(64)")
        std::string _video_fid;
        #pragma db index type("VARCHAR(64)")
        std::string _uploader_uid;
        #pragma db type("TEXT")
        std::string _title;
        #pragma db type("TEXT")
        odb::nullable<std::string> _summary;
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _auditor_uid;
        #pragma db type("INT")
        size_t _play_count;
        #pragma db type("INT")
        size_t _duration;
        #pragma db type("INT")
        size_t _size;
        #pragma db index type("TINYINT")
        VideoStatus _status;
        #pragma db type("DATETIME")
        boost::posix_time::ptime _upload_time;
};




//视频点赞关系表
#pragma db object table("tbl_video_like_meta")
class VideoLike {
    public:
        using ptr = std::shared_ptr<VideoLike>;
        VideoLike();
        VideoLike(const std::string &user_id, const std::string &video_id);
        size_t id() const;
        const std::string &user_id() const;
        const std::string &video_id() const;
        
        void set_id(size_t id);
        void set_user_id(const std::string &user_id);
        void set_video_id(const std::string &video_id);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db index type("VARCHAR(64)")
        std::string _user_id;
        #pragma db index type("VARCHAR(64)")
        std::string _video_id;
};

//定义获取用户关注总量视图结构: query = Follow::follower_uid='xxxx'
#pragma db view object(Follow) \
                query((?))
struct FollowedCount {
    using ptr = std::shared_ptr<FollowedCount>;
    #pragma db column("count(*)")    //select count(*) from tbl_follow_meta where (?)
    size_t count;
};
//定义获取用户关注总量视图结构: query = Follow::followed_uid='xxxx'
#pragma db view object(Follow) \
                query((?))
struct FollowerCount {
    using ptr = std::shared_ptr<FollowerCount>;
    #pragma db column("count(*)")
    size_t count;
};

// 定义用户视频播放总量统计视图结构： query: Query::Video::uploader_uid='xxxx'
#pragma db view object(Video) \
                query((?))
struct UserVPlayCount {
    using ptr = std::shared_ptr<UserVPlayCount>;
    #pragma db column("sum(" + Video::_play_count + ")")  // ”count(tbl_video_meta.play_count)“
    size_t count;
};
// 定义用户视频点赞总量统计视图结构： query: Query::Video::uploader_uid='xxxx'
#pragma db view object(VideoLike) \
                object(Video : VideoLike::_video_id == Video::_video_id) \
                query((?))
struct UserVLikeCount {
    using ptr = std::shared_ptr<UserVLikeCount>;
    #pragma db column("count(*)")
    size_t count;
};

struct UserCount {
    using ptr = std::shared_ptr<UserCount>;
    size_t followed_count = 0;
    size_t follower_count = 0;
    size_t vplay_count = 0;
    size_t vlike_count = 0;
};



#pragma db view object(User) \
                query((?))
struct UserPtr {
    using ptr = std::shared_ptr<UserPtr>;
    std::shared_ptr<User> user;
};

struct DetailUser {
    using ptr = std::shared_ptr<DetailUser>;
    User::ptr user;
    UserCount::ptr count;
    std::vector<UserIdentifyRole> uirs;
    std::optional<bool> isFollowing;
};

// 获取指定身份以及邮箱的用户信息： query=  Query::User::email==xxxx && Query::UserIdentifyRole::identify_type==xxxx
#pragma db view object(User) \
                object(UserIdentifyRole : User::_user_id == UserIdentifyRole::_user_id) \
                query((?))
struct EmailIdendifyUser {
    using ptr = std::shared_ptr<EmailIdendifyUser>;
    std::shared_ptr<User> user;
};


#define ASC_LIMIT_PAGE(page, size) \
    "ORDER BY order_field ASC LIMIT " + std::to_string(size) + " OFFSET " + std::to_string((page) * size)
#define DESC_LIMIT_PAGE(page, size) \
    "ORDER BY order_field DESC LIMIT " + std::to_string(size) + " OFFSET " + std::to_string((page) * size)
// 获取指定状态&身份的用户信息： query=  Query::User::status==xxxx && Query::UserIdentifyRole::identify_type==xxxx
#pragma db view object(User) \
                object(UserIdentifyRole : User::_user_id == UserIdentifyRole::_user_id) \
                query((?))
struct StatusIdentifyUser {
    using ptr = std::shared_ptr<StatusIdentifyUser>;
    std::shared_ptr<User> user;
    #pragma db column(UserIdentifyRole::_role_type)
    RoleType role_type;
    #pragma db column(User::_id + " AS order_field")
    size_t order_field;
};

#pragma db view object(User) \
                object(UserIdentifyRole : User::_user_id == UserIdentifyRole::_user_id) \
                query((?))
struct StatusIdentifyUserCount {
    using ptr = std::shared_ptr<StatusIdentifyUserCount>;
    #pragma db column("count(*)")
    size_t count;
};

struct StatusIdentifyUserList {
    using ptr = std::shared_ptr<StatusIdentifyUserList>;
    std::vector<StatusIdentifyUser> users;
    size_t total = 0;
};


struct VerifyCode {
    using ptr = std::shared_ptr<VerifyCode>;
    std::string session_id;
    std::string code_id;
    std::string code;
    std::string email;
};


//视频分类标签关系类设计
#pragma db object table("tbl_video_catgory_tag_meta")
class VideoCatgoryTag {
    public:
        using ptr = std::shared_ptr<VideoCatgoryTag>;
        VideoCatgoryTag();
        VideoCatgoryTag(const std::string &video_id, unsigned int catgory_id, unsigned int tag_id);
        size_t id() const;
        const std::string &video_id() const;
        unsigned int catgory_id() const;
        unsigned int tag_id() const;
        void set_id(size_t id);
        void set_video_id(const std::string &video_id);
        void set_catgory_id(unsigned int catgory_id);
        void set_tag_id(unsigned int tag_id);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db index type("VARCHAR(64)")
        std::string _video_id;
        #pragma db index type("INT")
        unsigned int _catgory_id;
        #pragma db index type("INT")
        unsigned int _tag_id;
};

//弹幕信息表映射类设计： 主键ID， 弹幕ID， 视频ID，用户ID， 弹幕内容，弹幕时间
#pragma db object table("tbl_barrage_meta")
class Barrage {
    public:
        using ptr = std::shared_ptr<Barrage>;
        Barrage();
        Barrage(const std::string &barrage_id, const std::string &video_id, const std::string &user_id, const std::string &content, size_t time);
        size_t id() const;
        const std::string &barrage_id() const;
        const std::string &video_id() const;
        const std::string &user_id() const;
        const std::string &content() const;
        size_t time() const;
        void set_id(size_t id);
        void set_barrage_id(const std::string &barrage_id);
        void set_video_id(const std::string &video_id);
        void set_user_id(const std::string &user_id);
        void set_content(const std::string &content);
        void set_time(size_t time);
    private:
        friend class odb::access;
        #pragma db id auto
        size_t _id;
        #pragma db unique type("VARCHAR(64)")
        std::string _barrage_id;
        #pragma db index type("VARCHAR(64)")
        std::string _video_id;
        #pragma db type("VARCHAR(64)")
        std::string _user_id;
        #pragma db type("VARCHAR(64)")
        std::string _content;
        #pragma db type("INT")
        size_t _time;
};

#pragma db view object(Video) \
                query((?))
struct VideoPtr {
    using ptr = std::shared_ptr<VideoPtr>;
    std::shared_ptr<Video> video;
};

#pragma db view object(Barrage) \
                query((?) + "ORDER BY " + Barrage::_time)
struct BarragePtr {
    using ptr = std::shared_ptr<BarragePtr>;
    std::shared_ptr<Barrage> barrage;
};

//视频点赞总量统计结构: Query::VideoLike::video_id='xxxx'
#pragma db view object(VideoLike) \
                query((?))
struct VideoLikeCount {
    using ptr = std::shared_ptr<VideoLikeCount>;
    #pragma db column("count(*)")
    size_t count;
};
//视频播放总量统计结构： Query::Video::video_id='xxxx'
#pragma db view object(Video) \
                query((?))
struct VideoPlayCount {
    using ptr = std::shared_ptr<VideoPlayCount>;
    #pragma db column(Video::_play_count)
    size_t count;
};

struct VideoCount {
    using ptr = std::shared_ptr<VideoCount>;
    size_t like_count = 0;
    size_t play_count = 0;
};

//主页视频ID列表： 根据时间进行排序，分页获取视频ID列表 : 视频状态必须是审核通过状态
//     query: Query::Video::status==xxxx order by order_field limit size offset page*size   DESC_LIMIT_PAGE(page, size)
#pragma db view object(Video) \
                query((?))
struct VideoMainIdList {
    using ptr = std::shared_ptr<VideoMainIdList>;
    #pragma db column(Video::_video_id)
    std::string video_id;
    #pragma db column(Video::_upload_time + " AS order_field")
    boost::posix_time::ptime order_field;
};

//获取用户视频ID列表：根据播放量进行排序，分页获取视频ID列表
//    query: Query::Video::uploader_uid==xxxx order by order_field limit size offset page*size
#pragma db view object(Video) \
                query((?))
struct VideoUserPlayIdList {
    using ptr = std::shared_ptr<VideoUserPlayIdList>;
    #pragma db column(Video::_video_id)
    std::string video_id;
    #pragma db column(Video::_play_count + " AS order_field")
    size_t order_field;
};
//获取用户指定状态的视频总量
#pragma db view object(Video) \
                query((?))
struct VideoUserPlayIdListCount {
    using ptr = std::shared_ptr<VideoUserPlayIdListCount>;
    #pragma db column("count(*)")
    size_t count;
};

struct VideoUserStatusIdList {
    using ptr = std::shared_ptr<VideoUserStatusIdList>;
    std::vector<std::string> videos;
    size_t total = 0;
};


//获取分类视频ID列表：根据播放量进行排序，分页获取视频ID列表
//    query: Query::VideoCatgoryTag::catgory_id==xxxx order by order_field limit size offset page*size
#pragma db view object(VideoCatgoryTag) \
                object(Video : Video::_video_id == VideoCatgoryTag::_video_id) \
                query((?))
struct VideoCatgoryPlayIdList {
    using ptr = std::shared_ptr<VideoCatgoryPlayIdList>;
    #pragma db column("DISTINCT " + Video::_video_id)
    std::string video_id;
    #pragma db column(Video::_play_count + " AS order_field")
    size_t order_field;
};
//获取标签视频ID列表：根据播放量进行排序，分页获取视频ID列表
//    query: Query::VideoCatgoryTag::tag_id==xxxx order by order_field limit size offset page*size
#pragma db view object(VideoCatgoryTag) \
                object(Video : Video::_video_id == VideoCatgoryTag::_video_id) \
                query((?))
struct VideoTagPlayIdList {
    using ptr = std::shared_ptr<VideoTagPlayIdList>;
    #pragma db column("DISTINCT " + Video::_video_id)
    std::string video_id;
    #pragma db column(Video::_play_count + " AS order_field")
    size_t order_field;
};

//获取指定状态的视频信息列表：根据上传时间进行排序，分页获取视频ID列表
//    query: Query::status==xxxx order by order_field limit size offset page*size
#pragma db view object(Video) \
                query((?))
struct VideoStatusIdList {
    using ptr = std::shared_ptr<VideoStatusIdList>;
    std::shared_ptr<Video> video;
    #pragma db column(Video::_upload_time + " AS order_field")
    boost::posix_time::ptime order_field;
};
//获取指定状态视频的总量
//    query: Query::status==xxxx
#pragma db view object(Video) \
                query((?))
struct VideoStatusCount {
    using ptr = std::shared_ptr<VideoStatusCount>;
    #pragma db column("count(*)")
    size_t count = 0;
};

//指定状态视频信息列表的整合结构
struct StatusVideoList {
    using ptr = std::shared_ptr<StatusVideoList>;
    std::vector<Video::ptr> videos;
    size_t total = 0;
};


}