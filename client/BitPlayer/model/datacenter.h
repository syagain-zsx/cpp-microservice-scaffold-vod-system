#ifndef DATACENTER_H
#define DATACENTER_H

#include <QObject>
#include "data.h"
#include "../netclient/netclient.h"

namespace model {
class DataCenter : public QObject
{
    Q_OBJECT
public:
    ~DataCenter();

    // 获取服务器地址
    const QString& getServerUrl()const;

    // 获取DataCenter对象实例
    static DataCenter* getInstance();

    // 初始化数据文件
    void initDataFile();
    // 存储数据到文件
    void saveDataFile();
    // 从文件中加载数据到内存
    void loadDataFile();

    const KindAndTag* getKindAndTagClassPtr();

    // 设置和获取sessionId
    void setSessionId(const QString& sessionId);
    const QString& getLoginSessionId()const;

    // 获取所有视频列表
    VideoList* getVideoListPtr();
    // 解析响应中的所有视频对象，并保存到视频列表
    void setVideoList(const QJsonObject& videoListJsonObj);

    // 设置和获取弹幕数据
    void setBarrageData(const QJsonArray& barrageArray);
    QHash<int64_t, QList<BarrageInfo>>& getBarrageData();

    // 获取和设置当前用户个人信息
    void setMyselfInfo(const QJsonObject& myselfJson);
    UserInfo* getMyselfInfo();
    // 获取和设置其他用户的个人信息
    void setOtherUserInfo(const QJsonObject& myselfJson);
    UserInfo* getOtherUserInfo();
    // 构建临时用户UserInfo信息
    void buildTempUserInfo();
    // 清楚用户信息
    void clearUserInfo();

    // 设置用户头像id
    void setAvatar(const QString& fileId);

    // 获取 和 设置用户视频列表
    void setUserVideoList(const QJsonObject& videoListJson);
    VideoList* getUserVideoList();

    // 获取 和 设置状态视频列表
    void setStatusVideoList(const QJsonObject& videoListJson);
    VideoList* getStatusVideoList();

    // 获取和设置管理员列表
    void setAdminList(const QJsonObject& resultJson, bool isAdminStatus = true);
    AdminList* getAdminList();
private:
    explicit DataCenter(QObject *parent = nullptr);

private:
    static DataCenter* instance;
    KindAndTag* kindAndTag = nullptr;
    QString serverUrl;
    network::NetClient httpClient;

    // 当前客户端登录到服务器对应的登录会话ID
    QString loginSessionId = "";
    VideoList* videoListPtr = nullptr;

    // 弹幕信息
    // key： 发送弹幕的数据 value: 该时间下的所有弹幕数据
    QHash<int64_t, QList<BarrageInfo>> barrages;

    // 保存当前用户个人信息
    UserInfo* myselfInfo = nullptr;
    // 保存其他用户个人信息
    UserInfo* otherUserInfo = nullptr;
    // 保存指定用户的视频列表：我的视频列表 或 其他用户视频列表
    VideoList* userVideoList = nullptr;
    // 状态视频列表--给视频审核页面使用
    VideoList* statusVideoList = nullptr;
    // 管理员列表
    AdminList* adminList = nullptr;

public:
    // 定义所有异步请求方法
    void helloAsync();
    void pingAsync();

    // 临时用户登录
    void loginTempUserAsync();

    // 获取所有视频列表
    void getAllVideoListAsync();

    // 获取分类视频列表
    void getAllVideoListInKindAsync(int kindId);

    // 获取标签视频列表
    void getAllVideoListInTagAsync(int tagId);

    // 获取搜索视频列表
    void getAllVideoListSearchTextAsync(const QString& searchText);

    // 下载图片
    void downloadPhotoAsync(const QString& photoFileId);

    // 上传图片
    void uploadPhotoAsync(const QByteArray& photoData, QWidget* whichPage);

    // 下载视频
    void downloadVideoAsync(const QString& videoFileId);

    // 上传视频
    void uploadVideoAsync(const QString& videoPath);

    // 获取弹幕数据
    void getVideoBarrageAsync(const QString& videoId);

    // 更新视频播放数
    void setPlayNumberAsync(const QString& videoId);

    // 检测视频是否被点赞过
    void getIsLikeVdieoAsync(const QString& videoId);

    // 更新点赞数据
    void setLikeNumberAsync(const QString& videoId);

    // 新增弹幕
    void loadupBarrageAsync(const QString& videoId, const BarrageInfo& barrageInfo);

    // 获取当前用户个人信息
    void getMyselfInfoAsync();

    // 获取其他用户个人信息
    void getOtherUserInfoAsync(const QString& userId);

    // 设置用户头像id
    void setAvatarAsync(const QString& fileId);

    // 获取用户视频列表: 当前用户 或 其他用户
    // userId: 空表示获取当前用户视频列表  否则获取其他用户视频列表
    void getUserVideoListAsync(const QString& userId, int pageIndex, VideoStatus videoStatus, const QString& whichPage);

    // 删除视频
    void deleteVideoAsync(const QString& videoId);

    // 关注
    void newAttentionAsync(const QString& userId);
    // 取消关注
    void delAttentionAsync(const QString& userId);

    // 获取验证码
    void getAuthcodeAsync(const QString& email);

    // 邮箱登录
    void loginWithEmailAsync(const QString& email, const QString& authcode, const QString& authcodeId);

    // 密码登录
    void loginWithPasswordAsync(const QString& username, const QString& password);

    // session登录
    void loginSessionAsync();

    // 退出登录
    void logoutAsync();

    // 设置密码
    void setPasswordAsync(const QString& password);

    // 设置昵称
    void setNicknameAsync(const QString& nickname);

    // 新增视频信息
    void uploadVideoDescAsync(const VideoDesc& videoDesc);

    // 获取状态视频列表
    void getStatusVideoListAsync(model::VideoStatus videoStatus, int pageIndex);

    // 视频审核
    void checkVideoAsync(const QString& videoId, bool result);

    // 上架视频
    void putawayVideoAsync(const QString& videoId);

    // 下架视频
    void discardVideoAsync(const QString& videoId);

    // 获取管理员信息--通过邮箱
    void getAdminInfoByEmailAsync(const QString& email);

    // 获取管理员信息--通过状态
    void getAdminInfoByStatusAsync(int pageIndex, AdminStatus adminStatus);

    // 新增管理员
    void newAdminAsync(const AdminInfo& adminInfo);

    // 编辑管理员
    void editAdminAsync(const AdminInfo& adminInfo);

    // 修改管理员状态
    void setAdminStatusAsync(const AdminInfo& adminInfo);

    // 删除管理员
    void delAdminAsync(const QString& userId);

signals:
    void helloDone();
    void pingDone();

    // 临时用户登录成功
    void loginTempUserDone();
    // 获取所有视频列表完成
    void getAllVideoListDone();
    // 获取分类视频列表完成
    void getAllVideoListInKindDone();
    // 获取标签视频列表完成
    void getAllVideoListInTagDone();
    // 获取搜索视频列表完成
    void getAllVideoListSearchTextDone();
    // 下载图片完成
    void downloadPhotoDone(const QString& photoFileId, QByteArray imageData);
    // 上传图片
    void uploadPhotoDone(const QString& fileId, QWidget* whichPage);
    // 下载视频完成
    void downloadVideoDone(const QString& m3u8Path, const QString& videoFileId);
    // 上传视频完成
    void uploadVideoDone(const QString& videoId);
    // 获取弹幕数据完成
    void getVideoBarrageDone(const QString& videoId);
    // 检测视频是否被点赞过
    void getIsLikeVdieoDone(const QString& videoId, bool isLike);
    // 获取当前用户个人信息
    void getMyselfInfoDone();
    // 获取其他用户个人信息
    void getOtherUserInfoDone();
    // 设置用户头像id成功
    void setAvatarDone();
    // 获取用户视频列表
    void getUserVideoListDone(const QString& userId, const QString& whichPage);
    // 删除视频
    void deleteVideoDone(const QString& videoId);
    // 关注
    void newAttentionDone();
    // 取消关注
    void delAttentionDone();
    // 获取验证码成功
    void getAuthcodeDone(const QString& codeId);
    // 邮箱登录成功
    void loginWithEmailDone();
    // 邮箱登录失败
    void loginWithEmailFailed(const QString& errorInfo);
    // 密码登录成功
    void loginWithPasswordDone();
    // 密码登录失败
    void loginWithPasswordFailed(const QString& errorInfo);
    // session登录成功
    void loginSessionDone(bool isTempUser);
    // session登录失败
    void loginSessionFailed(const QString& errorInfo);
    // 退出登录成功
    void logoutDone();
    // 设置密码成功
    void setPasswordDone();
    // 设置昵称成功
    void setNicknameDone(const QString& nickname);
    // 设置昵称失败
    void setNicknameFailed(const QString& reason);
    // 新增视频信息完成
    void uploadVideoDescDone();
    // 获取状态视频列表完成
    void getStatusVideoListDone();
    // 视频审核完成
    void checkVideoDone();
    // 上架视频完成
    void putawayVideoDone();
    // 下架视频完成
    void discardVideoDone();
    // 获取管理员信息完成--通过邮箱
    void getAdminInfoByEmailDone();
    // 获取管理员信息失败--通过邮箱
    void getAdminInfoByEmailFailed();
    // 获取管理员信息完成--通过状态
    void getAdminInfoByStatusDone();
    // 新增管理员完成
    void newAdminDone();
    // 编辑管理员完成
    void editAdminDone(const QString& adminId);
    // 修改管理员状态完成
    void setAdminStatusDone();
    // 删除管理员完成
    void delAdminDone();
};

}// end model



#endif // DATACENTER_H
