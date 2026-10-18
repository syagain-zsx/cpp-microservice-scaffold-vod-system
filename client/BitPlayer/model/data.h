#ifndef DATA_H
#define DATA_H
#include <QList>
#include <QString>
#include <QHash>
#include <QJsonObject>

// 该文件中，定义程序中各种数据对应的数据结构

namespace model {

/////////////////////////////////////////////////
/// 分类和标签
/////////////////////////////////////////////////
class KindAndTag
{
public:
    KindAndTag();

    // 获取所有的分类
    const QList<QString> getAllKinds()const;

    // 获取某个分类下所有的标签
    const QHash<QString, int> getTagsByKind(const QString& kind)const;

    // 获取kind对应的id
    int getKindId(const QString& kind)const;

    // 获取tag对应的id
    int getTagId(const QString& kind, const QString& tag)const;

private:
    QHash<QString, int> kindIds;
    // 分类-标签s  k-v
    QHash<QString, QHash<QString, int>> tagIds;
    static int id;
};

/////////////////////////////////////////////////
/// 视频信息结构
/////////////////////////////////////////////////

// 视频状态
enum VideoStatus{
    noStatus = 0,         // 无状态
    waitForChecking,      // 待审核
    putaway,              // 审核通过 or 上架
    reject,               // 审核驳回
    discard               // 已下架
};

class VideoInfo
{
public:
    QString videoId;          // 视频Id
    QString userId;           // 上传视频用户id
    QString userAvatarId;     // 上传视频用户头像id
    QString nickName;         // 上传视频用户昵称
    QString videoFileId;      // 视频文件id
    QString photoFileId;      // 视频封面id
    int64_t likeCount;        // 点赞量
    int64_t playCount;        // 播放量
    int64_t videoSize;        // 视频大小
    QString videoDesc;        // 视频描述信息
    QString videoTitle;       // 视频标题
    int64_t videoDuration;    // 视频持续市场
    QString videoUpTime;      // 视频上传时间
    int videoStatus;          // 视频状态
    QString checkerId;        // 审核者Id
    QString checkerName;      // 审核者昵称
    QString checkerAvatar;    // 审核者用户头像Id

    // 将JSON对象转换为视频信息
    void laodVideoInfo(const QJsonObject& videoInfoJson);
};

/////////////////////////////////////////////////
/// 视频列表
/////////////////////////////////////////////////
class VideoList
{
public:
    VideoList();
    // 设置或获取视频列表页号
    void setPageIndex(int pageIndex);
    int getPageIndex()const;

    // 获取视频列表中实际的视频个数
    int getVideoCount()const;

    // 设置或获取某条件下总的视频个数
    void setVideoTotalCount(int videoTotalCount);
    int64_t getVideoTotalCount()const;

    // 往视频列表中添加视频信息
    void addVideo(const VideoInfo& videoInfo);

    // 获取所有视频信息
    const QList<VideoInfo>& getVideoList()const;

    // 清空视频列表
    void clearVideoList();

    // 新增视频播放数
    void incrementPlayNum(const QString& videoId);

    // 更新点赞数
    void updateLikeNumber(const QString& videoId, int64_t likeCount);

    // 修改视频审核信息
    void updateVideoCheckInfo(const QString& videoId, VideoStatus videoStatus, const QString& nickname,
                              const QString& checkerId, const QString& checkerAvatarId);

    QList<VideoInfo> videoInofs;          // 目前从服务器上获取到的满足某条件的所有视频
    int pageIndex;                        // 页号
    int64_t videoTotalCount;              // 该条件下所包含的所有视频的总数
    const static int PAGE_COUNT = 20;     // 一页中包含的视频个数
};

/////////////////////////////////////////////////
/// 弹幕信息
/////////////////////////////////////////////////
class BarrageInfo
{
public:
    QString barrageId;      // 弹幕id
    QString userId;         // 发送弹幕的用户id
    int64_t playTime;       // 发送弹幕时视频的当前播放时间
    QString text;           // 弹幕文本内容

    void loadBarrageInfo(const QJsonObject& barrageJson);
};

/////////////////////////////////////////////////
/// 用户信息结构
/////////////////////////////////////////////////

// 角色类型：注意 项的初始化 以及 次序和课堂保持一致
enum RoleType{
    SuperAdmin = 1,     // 超级管理员
    Admin,              // 普通管理员
    User,               // 普通用户
    TempUser            // 临时用户
};

// 身份类型
enum identityType{
    CUser = 1,      // C 端用户
    BUser           // B 端用户
};

// 管理员状态
enum AdminStatus{
    noAdminStatus,   // 无状态
    eable,           // 启用
    disable          // 禁用
};

class UserInfo
{
public:
    QString userId;        // 用户Id
    QString email;         // 邮箱
    QString nickname;      // 用户昵称
    QList<int> roleType;   // 角色类型：普通用户 普通管理员 超级管理员 临时用户
    QList<int> identityType;  // 身份类型：B端用户 C端用户
    int64_t likeCount;        // 点赞数
    int64_t playCount;        // 播放数
    int64_t followedCount;    // 关注数：当前用户关注其他用户的数量
    int64_t followerCount;    // 粉丝数：其他用户管理当前用户的数量
    int userStatus;           // 用户状态：管理员页面需要用到--启用和禁止
    bool isFollowing;           // 是否被关注: 主要在其他用户信息中使用
    QString userMemo;          // 用户备注信息：新增管理员时需要用到
    QString userCTime;         // 用户创建的时间
    QString avatarFileId;      // 用户头像id

    // 保存用户头像数据，该字段发送给服务器，只是客户端自己使用
    QByteArray userAvatarData;  // 用户头像数据

    // Json对象转换为UserInfo结构
    void loadUserInfo(const QJsonObject& userInfoJson);
    // 检测用户是否为B端用户
    bool isBUser()const;
    // 检测用户是否为临时用户
    bool isTempUser()const;
    // 构建临时用户个人信息
    void buildTempUserInfo();
    // 检测用户是否被禁用
    bool isAdminDisable()const;
};

/////////////////////////////////////////////////
/// 上传视频时描述信息
/////////////////////////////////////////////////

class VideoDesc
{
public:
    QString videoFileId;     // 视频文件id
    QString photoFileId;     // 视频封面图文件id
    QString videoTitle;      // 视频标题
    QString videoDesc;       // 视频描述信息
    QString kind;            // 视频所属分类
    QList<QString> tags;            // 视频所属标签
    int64_t duration;        // 视频总时长
};

/////////////////////////////////////////////////
/// 管理员信息结构
/////////////////////////////////////////////////
class AdminInfo
{
public:
    QString userId;
    QString nickname;
    RoleType roleType;
    QString email;
    AdminStatus userStatu;
    QString userMemo;

    void loadAdminInfo(const QJsonObject& adminJson);
};

class AdminList
{
public:
    // 添加管理员
    void addAdminInfo(const AdminInfo& adminInfo);
    void setAdminStatus(const QString& userId, AdminStatus adminStatus);
    void clearAdminList();

    QList<AdminInfo> adminInfos;          // 保存管理员信息
    int64_t totalCount;                   // 系统中包含的管理员的总个数
    const static int PAGE_COUNT = 20;     // 默认一个页面显示20个管理员信息
};


} // end model

#endif // DATA_H







