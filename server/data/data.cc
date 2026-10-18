#include "data.h"


namespace vp_data{

    /* Sessoin Start----------------------------------- */
    Session::Session() {}
    Session::Session(const std::string &session_id) : _session_id(session_id) {}
    Session::Session(const std::string &session_id, const std::string &user_id)
        : _session_id(session_id), _user_id(user_id) {}
    size_t Session::id() const { return _id; }
    void Session::set_id(size_t id) { _id = id; }
    const std::string &Session::session_id() const { return _session_id; }
    void Session::set_session_id(const std::string &session_id) { _session_id = session_id; }
    odb::nullable<std::string> Session::user_id() const { return _user_id; }
    void Session::set_user_id(const odb::nullable<std::string> &user_id) { _user_id = user_id; }
    /* Sessoin End----------------------------------- */

    /* File Start--------------------------------------- */
    File::File() {}
    File::File(const std::string &file_id, const std::string &uploader_uid, const std::string &path, size_t size, const std::string &mime)
        : _file_id(file_id), _uploader_uid(uploader_uid), _path(path), _size(size), _mime(mime) {}
    size_t File::id() const { return _id; }
    void File::set_id(size_t id) { _id = id; }
    void File::set_file_id(const std::string &file_id) { _file_id = file_id; }
    void File::set_uploader_uid(const std::string &uploader_uid) { _uploader_uid = uploader_uid; }
    void File::set_path(const std::string &path) { _path = path;}
    void File::set_size(size_t size) { _size = size; }
    void File::set_mime(const std::string &mime) { _mime = mime; }
    const std::string &File::file_id() const { return _file_id; }
    const std::string &File::uploader_uid() const { return _uploader_uid; }
    const std::string &File::path() const { return _path; }
    const std::string &File::mime() const { return _mime; }
    size_t File::size() const { return _size; }
    /* File End----------------------------------- */

    
    /* Identify Start----------------------------------- */
    Identify::Identify() {}
    Identify::Identify(const std::string &identify_id, IdentifyType type, const std::string &name)
        : _identify_id(identify_id), _type(type), _name(name) {}
    size_t Identify::id() const { return _id;}
    const std::string &Identify::identify_id() const { return _identify_id;}
    IdentifyType Identify::type() const { return _type;}
    const std::string &Identify::name() const { return _name;}
    void Identify::set_id(size_t id) { _id = id;}
    void Identify::set_identify_id(const std::string &identify_id) { _identify_id = identify_id;}
    void Identify::set_type(IdentifyType type) { _type = type;}
    void Identify::set_name(const std::string &name) { _name = name;}
    /* Identify End----------------------------------- */

    /* Role Start----------------------------------- */
    Role::Role() {}
    Role::Role(const std::string &role_id, RoleType type, const std::string &name)
        : _role_id(role_id), _type(type), _name(name) {}
    size_t Role::id() const { return _id;}
    const std::string &Role::role_id() const { return _role_id;}
    RoleType Role::type() const { return _type;}
    const std::string &Role::name() const { return _name;}

    void Role::set_id(size_t id) { _id = id;}
    void Role::set_role_id(const std::string &role_id) { _role_id = role_id;}
    void Role::set_type(RoleType type) { _type = type;}
    void Role::set_name(const std::string &name) { _name = name;}
    /* Role End----------------------------------- */

    
    /* Operator Start----------------------------------- */
    Operator::Operator() {}
    Operator::Operator(const std::string &operator_id, const std::string &op_url, const std::string &name)
        : _operator_id(operator_id), _op_url(op_url), _name(name) {}
    size_t Operator::id() const { return _id;}
    const std::string &Operator::operator_id() const { return _operator_id;}
    const std::string &Operator::op_url() const { return _op_url;}
    const std::string &Operator::name() const { return _name;}

    void Operator::set_id(size_t id) { _id = id;}
    void Operator::set_operator_id(const std::string &operator_id) { _operator_id = operator_id;}
    void Operator::set_op_url(const std::string &op_url) { _op_url = op_url;}
    void Operator::set_name(const std::string &name) { _name = name;}
    /* Operator End----------------------------------- */

    
    /* User Start----------------------------------- */
    User::User() {}
    User::User(const std::string &user_id, const std::string &email) 
        : _user_id(user_id)
        , _email(email)
        , _nickname(user_id)
        , _status(UserStatus::ENABLE)
        , _reg_time(boost::posix_time::microsec_clock::local_time()) {}
    size_t User::id() const { return _id;}
    const std::string &User::user_id() const { return _user_id;}
    const std::string &User::email() const { return _email;}
    const std::string &User::nickname() const { return _nickname;}
    UserStatus User::status() const { return _status;}
    const odb::nullable<std::string> &User::bakname() const { return _bakname;}
    const odb::nullable<std::string> &User::phone() const { return _phone;}
    const odb::nullable<std::string> &User::password() const { return _password;}
    const odb::nullable<std::string> &User::avatar() const { return _avatar;}
    const odb::nullable<std::string> &User::remark() const { return _remark;}
    const boost::posix_time::ptime &User::reg_time() const { return _reg_time;}
    void User::set_id(size_t id) { _id = id;}
    void User::set_user_id(const std::string &user_id) { _user_id = user_id;}
    void User::set_email(const std::string &email) { _email = email;}
    void User::set_nickname(const std::string &nickname) { _nickname = nickname;}
    void User::set_status(UserStatus status) { _status = status;}
    void User::set_bakname(const odb::nullable<std::string> &bakname) { _bakname = bakname;}
    void User::set_phone(const odb::nullable<std::string> &phone) { _phone = phone;}
    void User::set_password(const odb::nullable<std::string> &password) { _password = password;}
    void User::set_avatar(const odb::nullable<std::string> &avatar) { _avatar = avatar;}
    void User::set_remark(const odb::nullable<std::string> &remark) { _remark = remark;}
    void User::set_reg_time(const boost::posix_time::ptime &reg_time) { _reg_time = reg_time;}
    /* User End----------------------------------- */

    
    /* UserIdentifyRole Start----------------------------------- */
    UserIdentifyRole::UserIdentifyRole() {}
    UserIdentifyRole::UserIdentifyRole(const std::string &user_id, IdentifyType identify_type, RoleType role_type)
        : _user_id(user_id), _identify_type(identify_type), _role_type(role_type) {}
    size_t UserIdentifyRole::id() const { return _id;}
    const std::string &UserIdentifyRole::user_id() const { return _user_id;}
    IdentifyType UserIdentifyRole::identify_type() const { return _identify_type;}
    RoleType UserIdentifyRole::role_type() const { return _role_type;}
    void UserIdentifyRole::set_id(size_t id) { _id = id;}
    void UserIdentifyRole::set_user_id(const std::string &user_id) { _user_id = user_id;}
    void UserIdentifyRole::set_identify_type(IdentifyType identify_type) { _identify_type = identify_type;}
    void UserIdentifyRole::set_role_type(RoleType role_type) { _role_type = role_type;}
    /* UserIdentifyRole End----------------------------------- */

    
    /* OperatorRole Start----------------------------------- */
    OperatorRole::OperatorRole() {}
    OperatorRole::OperatorRole(const std::string &operator_url, RoleType role_type)
        : _operator_url(operator_url), _role_type(role_type) {}
    size_t OperatorRole::id() const { return _id;}
    const std::string &OperatorRole::operator_url() const {   return _operator_url;}
    RoleType OperatorRole::role_type() const { return _role_type;}
    void OperatorRole::set_id(size_t id) { _id = id;}
    void OperatorRole::set_operator_url(const std::string &operator_url) { _operator_url = operator_url;}
    void OperatorRole::set_role_type(RoleType role_type) { _role_type = role_type;}
    /* OperatorRole End----------------------------------- */
    
    
    /* Follow Start----------------------------------- */
    Follow::Follow() {}
    Follow::Follow(const std::string &follower_uid, const std::string &followed_uid) 
        : _follower_uid(follower_uid), _followed_uid(followed_uid) {}
    size_t Follow::id() const { return _id;}
    const std::string &Follow::follower_uid() const { return _follower_uid;}
    const std::string &Follow::followed_uid() const { return _followed_uid;}

    void Follow::set_id(size_t id) { _id = id;}
    void Follow::set_follower_uid(const std::string &follower_uid) { _follower_uid = follower_uid;}
    void Follow::set_followed_uid(const std::string &followed_uid) { _followed_uid = followed_uid;}
    /* Follow End----------------------------------- */

    
    /* Video Start----------------------------------- */
    Video::Video()
    : _play_count(0) 
    , _duration(0)
    , _size(0)
    , _status(VideoStatus::TRANSCODING)
    , _upload_time(boost::posix_time::microsec_clock::local_time())
    {}
    Video::Video(const std::string &video_id, const std::string &title, 
        const std::string &summary, 
        const std::string &cover_fid, 
        const std::string &video_fid, 
        const std::string &uploader_uid, 
        size_t duration, size_t size)
        : _video_id(video_id)
        , _title(title)
        , _summary(summary)
        , _cover_fid(cover_fid)
        , _video_fid(video_fid)
        , _uploader_uid(uploader_uid)
        , _play_count(0) 
        , _duration(duration)
        , _size(size)
        , _status(VideoStatus::TRANSCODING)
        , _upload_time(boost::posix_time::microsec_clock::local_time())
        {}
    size_t Video::id() const { return _id;}
    const std::string &Video::video_id() const { return _video_id;}
    const std::string &Video::title() const { return _title;}
    const odb::nullable<std::string> &Video::summary() const { return _summary;}
    const std::string &Video::cover_fid() const { return _cover_fid;}
    const std::string &Video::video_fid() const { return _video_fid;}
    const std::string &Video::uploader_uid() const { return _uploader_uid;}
    const odb::nullable<std::string> &Video::auditor_uid() const { return _auditor_uid;}
    size_t Video::play_count() const { return _play_count;}
    size_t Video::duration() const { return _duration;}
    size_t Video::size() const { return _size;}
    VideoStatus Video::status() const { return _status;}
    const boost::posix_time::ptime &Video::upload_time() const { return _upload_time;}

    void Video::set_id(size_t id) { _id = id;}
    void Video::set_video_id(const std::string &video_id) { _video_id = video_id;}
    void Video::set_title(const std::string &title) { _title = title;}
    void Video::set_summary(const odb::nullable<std::string> &summary) { _summary = summary;}
    void Video::set_cover_fid(const std::string &cover_fid) { _cover_fid = cover_fid; }
    void Video::set_video_fid(const std::string &video_fid) { _video_fid = video_fid;}
    void Video::set_uploader_uid(const std::string &uploader_uid) { _uploader_uid = uploader_uid;}
    void Video::set_auditor_uid(const odb::nullable<std::string> &auditor_uid) { _auditor_uid = auditor_uid;}
    void Video::set_play_count(size_t play_count) { _play_count = play_count;}
    void Video::set_duration(size_t duration) { _duration = duration;}
    void Video::set_size(size_t size) { _size = size;}
    void Video::set_status(VideoStatus status) { _status = status;}
    void Video::set_upload_time(const boost::posix_time::ptime &upload_time) { _upload_time = upload_time;}
    /* Video End----------------------------------- */
    
    /* VideoLike Start----------------------------------- */
    VideoLike::VideoLike() {}
    VideoLike::VideoLike(const std::string &user_id, const std::string &video_id)
        : _user_id(user_id), _video_id(video_id) {}
    size_t VideoLike::id() const { return _id;}
    const std::string &VideoLike::user_id() const { return _user_id;}
    const std::string &VideoLike::video_id() const { return _video_id;}
    
    void VideoLike::set_id(size_t id) { _id = id;}
    void VideoLike::set_user_id(const std::string &user_id) { _user_id = user_id;}
    void VideoLike::set_video_id(const std::string &video_id) { _video_id = video_id;}
    /* VideoLike End----------------------------------- */


    
    /* VideoCatgoryTag Start----------------------------------- */
    VideoCatgoryTag::VideoCatgoryTag() {}
    VideoCatgoryTag::VideoCatgoryTag(const std::string &video_id, unsigned int catgory_id, unsigned int tag_id)
        : _video_id(video_id), _catgory_id(catgory_id), _tag_id(tag_id) {}
    size_t VideoCatgoryTag::id() const { return _id;}
    const std::string &VideoCatgoryTag::video_id() const { return _video_id;}
    unsigned int VideoCatgoryTag::catgory_id() const { return _catgory_id;}
    unsigned int VideoCatgoryTag::tag_id() const { return _tag_id;}
    void VideoCatgoryTag::set_id(size_t id) { _id = id;}
    void VideoCatgoryTag::set_video_id(const std::string &video_id) { _video_id = video_id;}
    void VideoCatgoryTag::set_catgory_id(unsigned int catgory_id) { _catgory_id = catgory_id;}
    void VideoCatgoryTag::set_tag_id(unsigned int tag_id) { _tag_id = tag_id;}
    /* VideoCatgoryTag End----------------------------------- */
    
    /* Barrage Start----------------------------------- */
    Barrage::Barrage() {}
    Barrage::Barrage(const std::string &barrage_id, const std::string &video_id, const std::string &user_id, const std::string &content, size_t time)
        : _barrage_id(barrage_id), _video_id(video_id), _user_id(user_id), _content(content), _time(time) {}
    size_t Barrage::id() const { return _id;}
    const std::string &Barrage::barrage_id() const { return _barrage_id; }
    const std::string &Barrage::video_id() const { return _video_id; }
    const std::string &Barrage::user_id() const { return _user_id; }
    const std::string &Barrage::content() const { return _content; }
    size_t Barrage::time() const { return _time; }
    void Barrage::set_id(size_t id) { _id = id;}
    void Barrage::set_barrage_id(const std::string &barrage_id) { _barrage_id = barrage_id;}
    void Barrage::set_video_id(const std::string &video_id) { _video_id = video_id;}
    void Barrage::set_user_id(const std::string &user_id) { _user_id = user_id; }
    void Barrage::set_content(const std::string &content) { _content = content;}
    void Barrage::set_time(size_t time) { _time = time;}
    /* Barrage End----------------------------------- */
}