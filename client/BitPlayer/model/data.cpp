#include "data.h"
#include <QJsonArray>

namespace model {
int KindAndTag::id = 10000;

KindAndTag::KindAndTag()
{
    // 构建分类和id
    QList<QString> kinds = {"历史", "美食", "游戏", "科技", "运动", "动物", "旅行", "电影"};
    for(auto& kind : kinds){
        kindIds.insert(kind, id++);
    }

    // 构建标签和id
    QHash<QString, QList<QString>> kindTags = {
            {"历史", {"中国史", "世界史", "历史人物", "艺术", "文化", "奇闻"}},
            {"美食", {"美食测评", "美食制作", "美食攻略", "美食记录", "探店", "水果", "海鲜"}},
            {"游戏", {"游戏攻略", "单机游戏", "电子竞技", "手机游戏", "网络游戏", "游戏赛事", "桌游棋牌"}},
            {"科技", {"数码", "软件应用", "智能家居", "手机", "电脑", "人工智能", "基础建设"}},
            {"运动", {"篮球", "足球", "乒乓球", "羽毛球", "健身", "竞技体育", "运动装备"}},
            {"动物", {"喵星人", "汪星人", "宠物知识", "动物资讯", "野生动物", "动物世界", "萌宠"}},
            {"旅行", {"旅游攻略", "旅行Vlog", "自驾游", "交通", "环球旅行", "露营", "野外生存"}},
            {"电影", {"电影解说", "电影推荐", "电影剪辑", "搞笑", "吐槽", "悬疑", "经典"}}};
    for(auto& kind : kinds){
        // 获取kind对应的所有标签
        QList<QString>& tags = kindTags[kind];

        // QHash<QString, QHash<QString, int> tagIds;
        QHash<QString, int> tagIdOfKind;
        for(auto& tag : tags){
            tagIdOfKind.insert(tag, id++);
        }

        tagIds.insert(kind, tagIdOfKind);
    }
}

const QList<QString> KindAndTag::getAllKinds() const
{
    return kindIds.keys();
}

const QHash<QString, int> KindAndTag::getTagsByKind(const QString &kind) const
{
    return tagIds[kind];
}

int KindAndTag::getKindId(const QString &kind) const
{
    return kindIds[kind];
}

int KindAndTag::getTagId(const QString &kind, const QString &tag) const
{
    return tagIds[kind][tag];
}

//////////////////////////////////////////////////////////////////////
/// 视频信息结构
//////////////////////////////////////////////////////////////////////
void VideoInfo::laodVideoInfo(const QJsonObject &videoInfoJson)
{
    videoId = videoInfoJson["videoId"].toString();
    userId = videoInfoJson["userId"].toString();
    userAvatarId = videoInfoJson["userAvatarId"].toString();
    nickName = videoInfoJson["nickname"].toString();
    videoFileId = videoInfoJson["videoFileId"].toString();
    photoFileId = videoInfoJson["photoFileId"].toString();
    likeCount = videoInfoJson["likeCount"].toInteger();
    playCount = videoInfoJson["playCount"].toInteger();
    videoSize = videoInfoJson["videoSize"].toInteger();
    videoDesc = videoInfoJson["videoDesc"].toString();
    videoTitle = videoInfoJson["videoTitle"].toString();
    videoDuration = videoInfoJson["videoDuration"].toInteger();
    videoUpTime = videoInfoJson["videoUpTime"].toString();
    videoUpTime = videoUpTime.left(videoUpTime.indexOf(' '));
    // 视频状态
    videoStatus = videoInfoJson["videoStatus"].toInt();
    // 审核者id
    checkerId = videoInfoJson["checkerId"].toString();
    // 审核者昵称
    checkerName = videoInfoJson["checkerName"].toString();
    // 审核者用户头像Id
    checkerAvatar = videoInfoJson["checkerAvatar"].toString();
}

/////////////////////////////////////////////////
/// 视频列表
/////////////////////////////////////////////////
VideoList::VideoList()
    :pageIndex(1)
    , videoTotalCount(0)
{}

void VideoList::setPageIndex(int pageIndex)
{
    this->pageIndex = pageIndex;
}

int VideoList::getPageIndex() const
{
    return pageIndex;
}

int VideoList::getVideoCount() const
{
    return videoInofs.size();
}

void VideoList::setVideoTotalCount(int videoTotalCount)
{
    this->videoTotalCount = videoTotalCount;
}

int64_t VideoList::getVideoTotalCount() const
{
    return videoTotalCount;
}

void VideoList::addVideo(const VideoInfo &videoInfo)
{
    videoInofs.append(videoInfo);
}

const QList<VideoInfo> &VideoList::getVideoList() const
{
    return videoInofs;
}

void VideoList::clearVideoList()
{
    videoInofs.clear();
    pageIndex = 1;
    videoTotalCount = 0;
}

void VideoList::incrementPlayNum(const QString &videoId)
{
    for(auto& videoInfo : videoInofs){
        if(videoInfo.videoId == videoId){
            videoInfo.playCount++;
            return;
        }
    }
}

void VideoList::updateLikeNumber(const QString &videoId, int64_t likeCount)
{
    for(auto& videoInfo : videoInofs){
        if(videoInfo.videoId == videoId){
            videoInfo.likeCount = likeCount;
            return;
        }
    }
}

void VideoList::updateVideoCheckInfo(const QString &videoId, VideoStatus videoStatus, const QString &nickname,
                                     const QString &checkerId, const QString &checkerAvatarId)
{
    for(auto& videoInfo : videoInofs){
        if(videoInfo.videoId == videoId){
            videoInfo.videoStatus = videoStatus;
            videoInfo.checkerId = checkerId;
            videoInfo.checkerName = nickname;
            videoInfo.checkerAvatar = checkerAvatarId;
            return;
        }
    }
}

/////////////////////////////////////////////////
/// 弹幕信息
/////////////////////////////////////////////////
void BarrageInfo::loadBarrageInfo(const QJsonObject &barrageJson)
{
    barrageId = barrageJson["barrageId"].toString();
    userId = barrageJson["userId"].toString();
    playTime = barrageJson["barrageTime"].toInteger();
    text = barrageJson["barrageContent"].toString();
}

/////////////////////////////////////////////////
/// 用户信息
/////////////////////////////////////////////////
void UserInfo::loadUserInfo(const QJsonObject &userInfoJson)
{
    userId = userInfoJson["userId"].toString();        // 用户Id
    email = userInfoJson["email"].toString();          // 邮箱
    nickname = userInfoJson["nickname"].toString();      // 用户昵称
    // 角色类型：普通用户 普通管理员 超级管理员 临时用户
    roleType.clear();
    QJsonArray roleTypeArray = userInfoJson["roleType"].toArray();
    for(int i = 0; i < roleTypeArray.size(); ++i){
        roleType.append(roleTypeArray[i].toInt());
    }

    // 身份类型：B端用户 C端用户
    identityType.clear();
    QJsonArray identityTypeArray = userInfoJson["identifyType"].toArray();
    for(int i = 0; i < identityTypeArray.size(); ++i){
        identityType.append(identityTypeArray[i].toInt());
    }

    likeCount = userInfoJson["likeCount"].toInteger();        // 点赞数
    playCount = userInfoJson["playCount"].toInteger();        // 播放数
    followedCount = userInfoJson["followedCount"].toInteger();    // 关注数：当前用户关注其他用户的数量
    followerCount = userInfoJson["followerCount"].toInteger();    // 粉丝数：其他用户管理当前用户的数量
    userStatus = userInfoJson["userStatus"].toInt();           // 用户状态：管理员页面需要用到--启用和禁止
    isFollowing = userInfoJson["isFollowing"].toBool();           // 是否被关注: 主要在其他用户信息中使用
    userMemo = userInfoJson["userMemo"].toString();          // 用户备注信息：新增管理员时需要用到
    userCTime = userInfoJson["userCTime"].toString();         // 用户创建的时间
    avatarFileId = userInfoJson["avatarFileId"].toString();      // 用户头像id
}

bool UserInfo::isBUser() const
{
    for(auto idType : identityType){
        if(idType == BUser){
            return true;
        }
    }

    return false;
}

bool UserInfo::isTempUser() const
{
    for(auto role : roleType){
        if(role == TempUser){
            return true;
        }
    }
    return false;
}

void UserInfo::buildTempUserInfo()
{
    userId = "";               // 用户Id
    email = "";                // 邮箱
    nickname = "临时用户";      // 用户昵称
    roleType.clear();
    roleType.append(TempUser); // 角色类型：普通用户 普通管理员 超级管理员 临时用户
    //roleType.append(User);
    identityType.clear();
    identityType.append(CUser);// 身份类型：B端用户 C端用户
    likeCount = 0;             // 点赞数
    playCount = 0;             // 播放数
    followedCount = 0;         // 关注数：当前用户关注其他用户的数量
    followerCount = 0;         // 粉丝数：其他用户管理当前用户的数量
    userStatus = 0;            // 用户状态：管理员页面需要用到--启用和禁止
    isFollowing = false;       // 是否被关注: 主要在其他用户信息中使用
    userMemo = "";             // 用户备注信息：新增管理员时需要用到
    userCTime = "";            // 用户创建的时间
    avatarFileId = "";         // 用户头像id
}

bool UserInfo::isAdminDisable() const
{
    return AdminStatus::disable == userStatus;
}

void AdminInfo::loadAdminInfo(const QJsonObject &adminJson)
{
    userId = adminJson["userId"].toString();
    nickname = adminJson["nickname"].toString();
    roleType = static_cast<RoleType>(adminJson["roleType"].toInt());
    email = adminJson["email"].toString();
    userStatu = static_cast<AdminStatus>(adminJson["userStatus"].toInt());
    userMemo = adminJson["userMemo"].toString();
}

void AdminList::addAdminInfo(const AdminInfo &adminInfo)
{
    adminInfos.append(adminInfo);
}

void AdminList::setAdminStatus(const QString &userId, AdminStatus adminStatus)
{
    for(auto& adminInfo : adminInfos){
        if(adminInfo.userId == userId){
            adminInfo.userStatu = adminStatus;
            return;
        }
    }
}

void AdminList::clearAdminList()
{
    adminInfos.clear();
    totalCount = 0;
}

} // end model
