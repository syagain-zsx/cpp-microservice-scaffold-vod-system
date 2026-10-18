#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>

// 角色类型
enum RoleTyep{
    SuperAdmin = 1,    // 超级管理员
    Admin = 2,         // 普通管理员
    User = 3,          // 普通用户
    TempUser = 4       // 临时用户
};

class BarrageInfo
{
public:
    QString userId;
    QString barrageId;
    int64_t playTime;
    QString text;

    BarrageInfo(const QString& userId = "", int64_t playTime = 0, const QString& text = "")
        : userId(userId)
        , playTime(playTime)
        , text(text)
    {}
};

class HttpServer : public QObject
{
    Q_OBJECT
public:
    static HttpServer* getInstance();
    bool init();
    void buildResponseData();
private:
    HttpServer();
    // hello的响应
    QHttpServerResponse hello(const QHttpServerRequest& request);
    // ping请求的响应
    QHttpServerResponse ping(const QHttpServerRequest& request);

    // tempLogin请求的响应
    QHttpServerResponse tempLogin(const QHttpServerRequest& request);

    // 获取所有视频列表
    QHttpServerResponse allVideoList(const QHttpServerRequest& request);

    // 获取分类视频列表
    QHttpServerResponse typeVideoList(const QHttpServerRequest& request);

    // 获取标签视频列表
    QHttpServerResponse tagVideoList(const QHttpServerRequest& request);

    // 获取搜索视频列表
    QHttpServerResponse keyVideoList(const QHttpServerRequest& request);

    // 下载图片
    QHttpServerResponse downloadPhoto(const QHttpServerRequest& request);

    // 上传图片
    QHttpServerResponse uploadPhoto(const QHttpServerRequest& request);

    // 下载视频
    QHttpServerResponse downloadVideo(const QHttpServerRequest& request);
    // 下载视频分片
    QHttpServerResponse downloadVideoSegmentation(const QString& fileName);

    // 上传视频
    QHttpServerResponse uploadVideo(const QHttpServerRequest& request);

    // 获取弹幕数据
    QHttpServerResponse getBarrage(const QHttpServerRequest& request);

    // 更新视频播放数
    QHttpServerResponse setPlay(const QHttpServerRequest& request);

    // 检测视频是否被点赞过
    QHttpServerResponse judgeLike(const QHttpServerRequest& request);

    // 更新点赞数据
    QHttpServerResponse setLike(const QHttpServerRequest& request);

    // 新增弹幕
    QHttpServerResponse newBarrage(const QHttpServerRequest& request);

    // 获取用户信息
    QHttpServerResponse getUserInfo(const QHttpServerRequest& request);

    // 设置用户头像id
    QHttpServerResponse setAvatar(const QHttpServerRequest& request);

    // 获取用户视频列表
    QHttpServerResponse userVideoList(const QHttpServerRequest& request);

    // 删除视频
    QHttpServerResponse removeVideo(const QHttpServerRequest& request);

    // 关注
    QHttpServerResponse newAttention(const QHttpServerRequest& request);

    // 取消关注
    QHttpServerResponse delAttention(const QHttpServerRequest& request);

    // 获取验证码
    QHttpServerResponse getCode(const QHttpServerRequest& request);

    // 短信登录
    QHttpServerResponse vcodeLogin(const QHttpServerRequest& request);

    // 密码登录
    QHttpServerResponse passwdLogin(const QHttpServerRequest& request);

    // session登录
    QHttpServerResponse sessionLogin(const QHttpServerRequest& request);

    // 退出登录
    QHttpServerResponse logout(const QHttpServerRequest& request);

    // 修改密码
    QHttpServerResponse setPassword(const QHttpServerRequest& request);

    // 修改昵称
    QHttpServerResponse setNickname(const QHttpServerRequest& request);

    // 新增视频信息
    QHttpServerResponse newVideo(const QHttpServerRequest& request);

    // 获取状态视频列表
    QHttpServerResponse statusVideoList(const QHttpServerRequest& request);

    // 视频审核
    QHttpServerResponse checkVideo(const QHttpServerRequest& request);

    // 上架视频
    QHttpServerResponse saleVideo(const QHttpServerRequest& request);

    // 下架视频
    QHttpServerResponse haltVideo(const QHttpServerRequest& request);

    // 获取管理员信息--通过手机号
    QHttpServerResponse getAdminByPhone(const QHttpServerRequest& request);

    // 获取管理员信息--通过状态
    QHttpServerResponse getAdminListByStatus(const QHttpServerRequest& request);

    // 新增管理员
    QHttpServerResponse newAdministrator(const QHttpServerRequest& request);

    // 编辑管理员
    QHttpServerResponse setAdministrator(const QHttpServerRequest& request);

    // 修改管理员状态
    QHttpServerResponse setStatus(const QHttpServerRequest& request);

    // 删除管理员
    QHttpServerResponse delAdministrator(const QHttpServerRequest& request);

private:
    static HttpServer* instance;   // HttpServer的实力指针
    QHttpServer httpServer;        // Qt封装的QHttpServer对象
    RoleTyep roleType = TempUser;  // 默认临时用户
    // 存放资源id和路径的映射关系
    QMap<QString, QString> idPathTable;
    // 保存视频的弹幕数据
    QList<BarrageInfo> barrages;
};

#endif // HTTPSERVER_H
