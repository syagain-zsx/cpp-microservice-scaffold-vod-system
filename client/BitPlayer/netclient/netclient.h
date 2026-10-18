#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include "../model/data.h"

namespace model {
class DataCenter;
}

namespace network {
class NetClient : public QObject
{
    Q_OBJECT
public:
    NetClient(model::DataCenter* dataCenter = nullptr);
    void setHTTPURL(const QString& HTTP_URL);

    // 给服务器发送一个hell的请求，服务器相应world结果
    void hello();

    // ping请求
    void ping();

    // 临时用户登录
    void loginTempUser();

    // 获取所有视频列表
    void getAllVideoList();

    // 获取分类视频列表
    void getAllVideoListInKind(int kindId);

    // 获取标签视频列表
    void getAllVideoListInTag(int tagId);

    // 获取搜索视频列表
    void getAllVideoListSearchText(const QString& searchText);

    // 下载图片
    void downloadPhoto(const QString& photoFileId);

    // 上传图片
    void uploadPhoto(const QByteArray& photoData, QWidget* whichPage);

    // 下载m3u8
    void downloadVideo(const QString& videoFileId);

    // 上传视频
    void uploadVideoAsync(const QString& videoPath);

    // 获取弹幕数据
    void getVideoBarrage(const QString& videoId);

    // 更新视频播放数
    void setPlayNumber(const QString& videoId);

    // 检测视频是否被点赞过
    void getIsLikeVdieo(const QString& videoId);

    // 更新点赞数据
    void setLikeNumber(const QString& videoId);

    // 新增弹幕
    void loadupBarrage(const QString& videoId, model::BarrageInfo barrageInfo);

    // 获取用户信息
    void getUserInfo(const QString& userId);

    // 设置用户头像id
    void setAvatar(const QString& fileId);

    // 获取用户视频列表
    void getUserVideoList(const QString& userId, int pageIndex, model::VideoStatus videoStatus, const QString& whichPage);

    // 删除视频
    void deleteVideo(const QString& videoId);

    // 关注
    void newAttention(const QString& userId);

    // 取消关注
    void delAttention(const QString& userId);

    // 获取验证码
    void getAuthcode(const QString& email);

    // 邮箱登录
    void loginWithEmail(const QString& email, const QString& authcode, const QString& authcodeId);

    // 密码登录
    void loginWithPassword(const QString& username, const QString& password);

    // session登录
    void loginSession();

    // 退出登录
    void logout();

    // 修改密码
    void setPassword(const QString& password);

    // 修改密码
    void setNickname(const QString& nickname);

    // 新增视频信息
    void uploadVideoDesc(const model::VideoDesc& videoDesc);

    // 获取状态视频列表
    void getStatusVideoList(model::VideoStatus videoStatus, int pageCount);

    // 视频审核
    void checkVideo(const QString& videoId, bool result);

    // 上架视频
    void putawayVideo(const QString& videoId);

    // 下架视频
    void discardVideo(const QString& videoId);

    // 获取管理员信息--通过邮箱
    void getAdminInfoByEmail(const QString& email);

    // 获取管理员信息--通过状态
    void getAdminInfoByStatus(int pageIndex, model::AdminStatus adminStatus);

    // 新增管理员
    void newAdmin(const model::AdminInfo& adminInfo);

    // 编辑管理员
    void editAdmin(const model::AdminInfo& adminInfo);

    // 修改管理员状态
    void setAdminStatus(const model::AdminInfo& adminInfo);

    // 删除管理员信息
    void delAdmin(const QString& userId);

private:
    QString makeRequestId();
    QNetworkReply*  sendHttpRequest(const QString& resourcePath, QJsonObject& reqBody);
    QJsonObject handleHttpResponse(QNetworkReply* httpReply, bool& ok, QString& reason);

private:
    QString HTTP_URL;
    QNetworkAccessManager httpClient;
    model::DataCenter* dataCenter = nullptr;
};

}// end network



#endif // NETCLIENT_H
