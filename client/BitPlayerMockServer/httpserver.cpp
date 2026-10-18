#include "httpserver.h"
#include <QHttpServerRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include "util.h"

HttpServer* HttpServer::instance = nullptr;

HttpServer *HttpServer::getInstance()
{
    if(nullptr == instance){
        instance = new HttpServer();
    }

    return instance;
}

HttpServer::HttpServer()
{}



bool HttpServer::init()
{
    buildResponseData();

    // 绑定ip和端口号，启动服务器，以接收客户端的请求
    int ret = httpServer.listen(QHostAddress::Any, 8000);

    // 绑定路由规则
    // /hello请求绑定具体的响应处理
    httpServer.route("/hello", [=](const QHttpServerRequest& request){
        return this->hello(request);
    });

    // ping请求绑定具体的响应处理
    httpServer.route("/ping", [=](const QHttpServerRequest& request){
        return this->ping(request);
    });

    // ping请求绑定具体的响应处理
    httpServer.route("/HttpService/tempLogin", [=](const QHttpServerRequest& request){
        return this->tempLogin(request);
    });

    // 获取所有视频列表
    httpServer.route("/HttpService/allVideoList", [=](const QHttpServerRequest& request){
        return this->allVideoList(request);
    });

    // 获取分类视频列表
    httpServer.route("/HttpService/typeVideoList", [=](const QHttpServerRequest& request){
        return this->typeVideoList(request);
    });

    // 获取标签视频列表
    httpServer.route("/HttpService/tagVideoList", [=](const QHttpServerRequest& request){
        return this->tagVideoList(request);
    });

    // 获搜索签视频列表
    httpServer.route("/HttpService/keyVideoList", [=](const QHttpServerRequest& request){
        return this->keyVideoList(request);
    });

    // 下载图片
    httpServer.route("/HttpService/downloadPhoto", [=](const QHttpServerRequest& request){
        return this->downloadPhoto(request);
    });

    // 上传图片
    httpServer.route("/HttpService/downloadPhoto", [=](const QHttpServerRequest& request){
        return this->uploadPhoto(request);
    });

    // 下载视频文件
    httpServer.route("/HttpService/downloadVideo", [=](const QHttpServerRequest& request){
        return this->downloadVideo(request);
    });

    // 下载视频分片文件
    httpServer.route("/videos/", [=](const QString& fileName){
        return this->downloadVideoSegmentation(fileName);
    });

    // 上传视频文件
    httpServer.route("/HttpService/uploadVideo", [=](const QHttpServerRequest& request){
        return this->uploadVideo(request);
    });

    // 获取弹幕数据
    httpServer.route("/HttpService/getBarrage", [=](const QHttpServerRequest& request){
        return this->getBarrage(request);
    });

    // 获取弹幕数据
    httpServer.route("/HttpService/setPlay", [=](const QHttpServerRequest& request){
        return this->setPlay(request);
    });

    // 检测是否是否被点赞过
    httpServer.route("/HttpService/judgeLike", [=](const QHttpServerRequest& request){
        return this->judgeLike(request);
    });

    // 更新点赞数
    httpServer.route("/HttpService/setLike", [=](const QHttpServerRequest& request){
        return this->setLike(request);
    });

    // 新增弹幕
    httpServer.route("/HttpService/newBarrage", [=](const QHttpServerRequest& request){
        return this->newBarrage(request);
    });

    // 获取用户信息
    httpServer.route("/HttpService/getUserInfo", [=](const QHttpServerRequest& request){
        return this->getUserInfo(request);
    });

    // 上传图片
    httpServer.route("/HttpService/uploadPhoto", [=](const QHttpServerRequest& request){
        return this->uploadPhoto(request);
    });

    // 设置用户头像id
    httpServer.route("/HttpService/setAvatar", [=](const QHttpServerRequest& request){
        return this->setAvatar(request);
    });

    // 获取用户视频列表
    httpServer.route("/HttpService/userVideoList", [=](const QHttpServerRequest& request){
        return this->userVideoList(request);
    });

    // 获取用户视频列表
    httpServer.route("/HttpService/removeVideo", [=](const QHttpServerRequest& request){
        return this->removeVideo(request);
    });

    // 关注
    httpServer.route("/HttpService/newAttention", [=](const QHttpServerRequest& request){
        return this->newAttention(request);
    });

    // 取消关注
    httpServer.route("/HttpService/delAttention", [=](const QHttpServerRequest& request){
        return this->delAttention(request);
    });

    // 获取验证码
    httpServer.route("/HttpService/getCode", [=](const QHttpServerRequest& request){
        return this->getCode(request);
    });

    // 短信登录
    httpServer.route("/HttpService/vcodeLogin", [=](const QHttpServerRequest& request){
        return this->vcodeLogin(request);
    });

    // 密码登录
    httpServer.route("/HttpService/passwdLogin", [=](const QHttpServerRequest& request){
        return this->passwdLogin(request);
    });

    // session登录
    httpServer.route("/HttpService/sessionLogin", [=](const QHttpServerRequest& request){
        return this->sessionLogin(request);
    });

    // 退出登录
    httpServer.route("/HttpService/logout", [=](const QHttpServerRequest& request){
        return this->logout(request);
    });

    // 修改密码
    httpServer.route("/HttpService/setPassword", [=](const QHttpServerRequest& request){
        return this->setPassword(request);
    });

    // 修改昵称
    httpServer.route("/HttpService/setNickname", [=](const QHttpServerRequest& request){
        return this->setNickname(request);
    });

    // 修改昵称
    httpServer.route("/HttpService/newVideo", [=](const QHttpServerRequest& request){
        return this->newVideo(request);
    });

    // 获取状态视频列表
    httpServer.route("/HttpService/statusVideoList", [=](const QHttpServerRequest& request){
        return this->statusVideoList(request);
    });

    // 视频审核
    httpServer.route("/HttpService/checkVideo", [=](const QHttpServerRequest& request){
        return this->checkVideo(request);
    });

    // 上架视频
    httpServer.route("/HttpService/saleVideo", [=](const QHttpServerRequest& request){
        return this->saleVideo(request);
    });

    // 下架视频
    httpServer.route("/HttpService/haltVideo", [=](const QHttpServerRequest& request){
        return this->haltVideo(request);
    });

    // 获取管理员信息--通过手机号
    httpServer.route("/HttpService/getAdminByPhone", [=](const QHttpServerRequest& request){
        return this->getAdminByPhone(request);
    });

    // 获取管理员信息--通过状态
    httpServer.route("/HttpService/getAdminListByStatus", [=](const QHttpServerRequest& request){
        return this->getAdminListByStatus(request);
    });

    // 新增管理员
    httpServer.route("/HttpService/newAdministrator", [=](const QHttpServerRequest& request){
        return this->newAdministrator(request);
    });

    // 编辑管理员
    httpServer.route("/HttpService/setAdministrator", [=](const QHttpServerRequest& request){
        return this->setAdministrator(request);
    });

    // 修改管理员的状态
    httpServer.route("/HttpService/setStatus", [=](const QHttpServerRequest& request){
        return this->setStatus(request);
    });

    // 删除管理员
    httpServer.route("/HttpService/delAdministrator", [=](const QHttpServerRequest& request){
        return this->delAdministrator(request);
    });

    return 8000 == ret;
}

void HttpServer::buildResponseData()
{
    // 获取所有视频列表中：用户头像id、视频封面id与具体图片资源路径的映射关系
    // 该模块下的所有id都是从10000开始的
    int resourceId = 10000;
    for(int i = 0; i < 100; ++i){
        idPathTable.insert(QString::number(resourceId++), "/images/touxiang.png");
        idPathTable.insert(QString::number(resourceId++), "/images/videoImage.png");
        idPathTable.insert(QString::number(resourceId++), "/videos/111.m3u8");
    }

    // 获取分类视频列表中：用户头像id、视频封面id与具体图片资源路径的映射关系
    // 该模块下的所有id都是从20000开始的
    resourceId = 20000;
    for(int i = 0; i < 100; ++i){
        idPathTable.insert(QString::number(resourceId++), "/images/touxiang1.png");
        idPathTable.insert(QString::number(resourceId++), "/images/videoImage1.png");
        idPathTable.insert(QString::number(resourceId++), "/videos/111.m3u8");
    }

    // 获取标签视频列表中：用户头像id、视频封面id与具体图片资源路径的映射关系
    // 该模块下的所有id都是从30000开始的
    resourceId = 30000;
    for(int i = 0; i < 100; ++i){
        idPathTable.insert(QString::number(resourceId++), "/images/touxiang2.png");
        idPathTable.insert(QString::number(resourceId++), "/images/videoImage2.png");
        idPathTable.insert(QString::number(resourceId++), "/videos/111.m3u8");
    }

    // 获取搜索视频列表中：用户头像id、视频封面id与具体图片资源路径的映射关系
    // 该模块下的所有id都是从40000开始的
    resourceId = 40000;
    for(int i = 0; i < 100; ++i){
        idPathTable.insert(QString::number(resourceId++), "/images/touxiang3.png");
        idPathTable.insert(QString::number(resourceId++), "/images/videoImage3.png");
        idPathTable.insert(QString::number(resourceId++), "/videos/111.m3u8");
    }

    // 获取用户视频列表中：用户头像id、视频封面id与具体图片资源路径的映射关系
    // 该模块下的所有id都是从50000开始的
    resourceId = 50000;
    for(int i = 0; i < 100; ++i){
        idPathTable.insert(QString::number(resourceId++), "/images/touxiang3.png");
        idPathTable.insert(QString::number(resourceId++), "/images/videoImage3.png");
        idPathTable.insert(QString::number(resourceId++), "/videos/111.m3u8");
    }

    // 上传视频id从60000开始
    idPathTable.insert(QString::number(60000), "/videos/");

    // 获取用户视频列表中：用户头像id、视频封面id与具体图片资源路径的映射关系
    // 该模块下的所有id都是从70000开始的
    resourceId = 70000;
    for(int i = 0; i < 200; ++i){
        idPathTable.insert(QString::number(resourceId++), "/images/touxiang3.png");
        idPathTable.insert(QString::number(resourceId++), "/images/fengjing.png");
        idPathTable.insert(QString::number(resourceId++), "/videos/111.m3u8");
    }
}

QHttpServerResponse HttpServer::hello(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[hello] 收到 hello 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    /*
         * {
         *     requestId : 1234,
         *     errorCode : 0,
         *     errorMsg : "",
         *     data{
         *         hello : world
         *     }
         * }
         */
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonObject jsonBody;
    jsonBody["hello"] = "world";
    jsonResp["data"] = jsonBody;


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::ping(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[ping] 收到 ping 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    /*
         * {
         *     requestId : 1234,
         *     errorCode : 0,
         *     errorMsg : "",
         *     data{
         *         ping : pong
         *     }
         * }
         */
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonObject jsonBody;
    jsonBody["ping"] = "pong";
    jsonResp["data"] = jsonBody;


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::tempLogin(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[tempLogin] 收到 tempLogin 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonObject jsonBody;
    // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    QString sessionId = QUuid::createUuid().toString();
    sessionId = sessionId.mid(25, 12);
    jsonBody["sessionId"] = sessionId;
    jsonResp["result"] = jsonBody;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::allVideoList(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[allVideoList] 收到 allVideoList 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    int pageCount = jsonReq["pageCount"].toInt();
    QJsonObject resultJsonObj;
    resultJsonObj["totalCount"] = 100;

    int videoId = 10000;
    int userId = 10000;
    int resourceId = 10000;
    QJsonArray videoLists;
    for(int i = 0; i < pageCount; ++i){
        QJsonObject videoJsonObj;
        videoJsonObj["videoId"] = QString::number(videoId++);
        videoJsonObj["userId"] = QString::number(userId++);
        videoJsonObj["userAvatarId"] = QString::number(resourceId++);
        videoJsonObj["nickname"] = "用户昵称";
        videoJsonObj["photoFileId"] = QString::number(resourceId++);
        videoJsonObj["videoFileId"] = QString::number(resourceId++);
        videoJsonObj["likeCount"] = 1234;
        videoJsonObj["playCount"] = 3456;
        videoJsonObj["videoSize"] = 10240;
        videoJsonObj["videoDesc"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩~";
        videoJsonObj["videoTitle"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoJsonObj["videoDuration"] = 10;
        videoJsonObj["videoUpTime"] = "9-16 12:28:58";

        videoLists.append(videoJsonObj);
    }
    resultJsonObj["videoList"] = videoLists;
    jsonResp["result"] = resultJsonObj;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::typeVideoList(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[typeVideoList] 收到 typeVideoList 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    int pageCount = jsonReq["pageCount"].toInt();
    pageCount = 10;
    QJsonObject resultJsonObj;
    resultJsonObj["totalCount"] = 100;

    int videoId = 20000;
    int userId = 20000;
    int resourceId = 20000;
    QJsonArray videoLists;
    for(int i = 0; i < pageCount; ++i){
        QJsonObject videoJsonObj;
        videoJsonObj["videoId"] = QString::number(videoId++);
        videoJsonObj["userId"] = QString::number(userId++);
        videoJsonObj["userAvatarId"] = QString::number(resourceId++);
        videoJsonObj["nickname"] = "用户昵称";
        videoJsonObj["photoFileId"] = QString::number(resourceId++);
        videoJsonObj["videoFileId"] = QString::number(resourceId++);
        videoJsonObj["likeCount"] = 1234;
        videoJsonObj["playCount"] = 23456;
        videoJsonObj["videoSize"] = 10240;
        videoJsonObj["videoDesc"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩~";
        videoJsonObj["videoTitle"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoJsonObj["videoDuration"] = 10;
        videoJsonObj["videoUpTime"] = "9-16 12:28:58";

        videoLists.append(videoJsonObj);
    }
    resultJsonObj["videoList"] = videoLists;
    jsonResp["result"] = resultJsonObj;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::tagVideoList(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[tagVideoList] 收到 tagVideoList 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    int pageCount = jsonReq["pageCount"].toInt();
    pageCount = 13;
    QJsonObject resultJsonObj;
    resultJsonObj["totalCount"] = 100;

    int videoId = 30000;
    int userId = 30000;
    int resourceId = 30000;
    QJsonArray videoLists;
    for(int i = 0; i < pageCount; ++i){
        QJsonObject videoJsonObj;
        videoJsonObj["videoId"] = QString::number(videoId++);
        videoJsonObj["userId"] = QString::number(userId++);
        videoJsonObj["userAvatarId"] = QString::number(resourceId++);
        videoJsonObj["nickname"] = "用户昵称";
        videoJsonObj["photoFileId"] = QString::number(resourceId++);
        videoJsonObj["videoFileId"] = QString::number(resourceId++);
        videoJsonObj["likeCount"] = 1234;
        videoJsonObj["playCount"] = 23456;
        videoJsonObj["videoSize"] = 10240;
        videoJsonObj["videoDesc"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩~";
        videoJsonObj["videoTitle"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoJsonObj["videoDuration"] = 10;
        videoJsonObj["videoUpTime"] = "9-16 12:28:58";

        videoLists.append(videoJsonObj);
    }
    resultJsonObj["videoList"] = videoLists;
    jsonResp["result"] = resultJsonObj;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::keyVideoList(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[keyVideoList] 收到 keyVideoList 请求， requestId = "<<jsonReq["requestId"].toString();
    LOG()<<jsonReq["searchKey"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    int pageCount = jsonReq["pageCount"].toInt();
    pageCount = 1;
    QJsonObject resultJsonObj;
    resultJsonObj["totalCount"] = 100;

    int videoId = 40000;
    int userId = 40000;
    int resourceId = 40000;
    QJsonArray videoLists;
    for(int i = 0; i < pageCount; ++i){
        QJsonObject videoJsonObj;
        videoJsonObj["videoId"] = QString::number(videoId++);
        videoJsonObj["userId"] = QString::number(userId++);
        videoJsonObj["userAvatarId"] = QString::number(resourceId++);
        videoJsonObj["nickname"] = "用户昵称";
        videoJsonObj["photoFileId"] = QString::number(resourceId++);
        videoJsonObj["videoFileId"] = QString::number(resourceId++);
        videoJsonObj["likeCount"] = 1234;
        videoJsonObj["playCount"] = 23456;
        videoJsonObj["videoSize"] = 10240;
        videoJsonObj["videoDesc"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩~";
        videoJsonObj["videoTitle"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoJsonObj["videoDuration"] = 10;
        videoJsonObj["videoUpTime"] = "9-16 12:28:58";

        videoLists.append(videoJsonObj);
    }
    resultJsonObj["videoList"] = videoLists;
    jsonResp["result"] = resultJsonObj;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::downloadPhoto(const QHttpServerRequest &request)
{
    // 从URL中解析出查询字符串
    QUrlQuery query(request.url());
    QString requestId = query.queryItemValue("requestId");
    QString fileId = query.queryItemValue("fileId");
    LOG()<<"[downloadPhoto] 收到 downloadPhoto 请求， requestId="<<requestId;

    // 构造图片路径
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    QString imagePath = dir.absolutePath();
    imagePath += idPathTable[fileId];
    //LOG()<<imagePath;

    // 读取图片数据
    QByteArray imageData = loadFileToByteArray(imagePath);

    // 构造HTTP响应并返回给客户端
    QHttpServerResponse httpResp(imageData, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/octet-stream");
    return httpResp;
}

QHttpServerResponse HttpServer::uploadPhoto(const QHttpServerRequest &request)
{
    // 从URL中解析出查询字符串
    QUrlQuery query(request.url());
    QString requestId = query.queryItemValue("requestId");
    QString sessionId = query.queryItemValue("sessionId");
    LOG()<<"[uploadPhoto] 收到 uploadPhoto 请求， requestId="<<requestId<<", sessionId="<<sessionId;

    // 解析出图片数据
    QByteArray imageData = request.body();

    // 将图片保存在服务器中
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    QString imagePath = dir.absolutePath();
    imagePath += "/images/temp.png";
    LOG()<<imagePath;
    writeByteArrayToFile(imagePath, imageData);

    // 设置图片id和图片在服务器中路径的对应关系
    idPathTable["1000"] = "/images/temp.png";

    // 构造响应
    QJsonObject resultJson;
    resultJson["fileId"] = "1000";

    QJsonObject jsonResp;
    jsonResp["requestId"] = requestId;
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    // 构造HTTP响应并返回给客户端
    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json");
    return httpResp;
}

QHttpServerResponse HttpServer::downloadVideo(const QHttpServerRequest &request)
{
    // 从URL中解析出查询字符串
    QUrlQuery query(request.url());
    QString fileId = query.queryItemValue("fileId");
    LOG()<<"[downloadVideo] 收到 downloadVideo 请求";

    // 构造图片路径
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    QString imagePath = dir.absolutePath();
    imagePath += idPathTable[fileId];
    LOG()<<imagePath;

    // 读取图片数据
    QByteArray imageData = loadFileToByteArray(imagePath);

    // 构造HTTP响应并返回给客户端
    QHttpServerResponse httpResp(imageData, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/octet-stream");
    return httpResp;
}

QHttpServerResponse HttpServer::downloadVideoSegmentation(const QString& fileName)
{
    // 构造视频文件的路径
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    QString videoPath = dir.absolutePath();
    videoPath += "/videos/" + fileName;
    LOG()<<videoPath;

    // 读取视频分片数据
    QByteArray videoData = loadFileToByteArray(videoPath);

    // 构造HTTP响应并返回给客户端
    QHttpServerResponse httpResp(videoData, QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/octet-stream");
    return httpResp;
}

QHttpServerResponse HttpServer::uploadVideo(const QHttpServerRequest &request)
{
    // 从URL中解析出查询字符串
    QUrlQuery query(request.url());
    QString requestId = query.queryItemValue("requestId");
    LOG()<<"[uploadVideo] 收到 uploadVideo 请求, requestId = "<<requestId;

    // 获取视频文件数据
    QByteArray videoData = request.body();

    // 构造图片路径
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    QString videoPath = dir.absolutePath();
    videoPath += idPathTable["60000"];
    videoPath += "222.mp4";
    LOG()<<videoPath;

    // 将视频数据写入到文件
    writeByteArrayToFile(videoPath, videoData);

    // 构造http响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = requestId;
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";
    QJsonObject resultJson;
    resultJson["fileId"] = "60000";
    jsonResp["result"] = resultJson;

    // 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    // 构造HTTP响应并返回给客户端
    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::getBarrage(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[getBarrage] 收到 getBarrage 请求， requestId = "<<jsonReq["requestId"].toString();

    // 构造一批弹幕数据
    QString videoId = jsonReq["videoId"].toString();

    // 构造不同时间点的弹幕数据
    QList<BarrageInfo> barrageList;
    for(int i = 0; i < 3; ++i){
        int playTime = i + 2;
        BarrageInfo barrageInfo("100001", playTime, "我是弹幕"+QString::number(i));
        barrageList.append(barrageInfo);
    }
    // 构造相同时间点的弹幕数据
    for(int i = 0; i < 5; ++i){
            int playTime = 3;
        BarrageInfo barrageInfo("100001", playTime, "我是第"+QString::number(playTime)+"秒的弹幕"+QString::number(i));
        barrageList.append(barrageInfo);
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonArray barrageArray;
    for(int i = 0; i < barrageList.size(); ++i)
    {
        BarrageInfo& barrageInfo = barrageList[i];
        QJsonObject barrageJson;
        barrageJson["barrageId"] = "50000"+QString::number(i);
        barrageJson["userId"] = barrageInfo.userId;
        barrageJson["barrageTime"] = barrageInfo.playTime;
        barrageJson["barrageContent"] = barrageInfo.text;
        barrageArray.append(barrageJson);
    }

    QJsonObject resultJsonObj;
    resultJsonObj["barrageList"] = barrageArray;
    jsonResp["result"] = resultJsonObj;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setPlay(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[setPlay] 收到 setPlay 请求， requestId = "<<jsonReq["requestId"].toString();

    QString videoId = jsonReq["videoId"].toString();
    LOG()<<"视频"<<videoId<<"播放次数更新成功";


    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::judgeLike(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[judgeLike] 收到 judgeLike 请求， requestId = "<<jsonReq["requestId"].toString();

    QString videoId = jsonReq["videoId"].toString();
    LOG()<<"视频"<<videoId<<"播放次数更新成功";


    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonObject resultJson;
    resultJson["isLike"] = true;
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setLike(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[setLike] 收到 setLike 请求， requestId = "<<jsonReq["requestId"].toString();

    QString videoId = jsonReq["videoId"].toString();
    LOG()<<"视频"<<videoId<<"播放次数更新成功";

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::newBarrage(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[newBarrage] 收到 newBarrage 请求， requestId = "<<jsonReq["requestId"].toString();

    // 解析出客户端新增的弹幕数据
    QString videoId = jsonReq["videoId"].toString();
    BarrageInfo barrageInfo;
    QJsonObject barrageJson = jsonReq["barrageInfo"].toObject();
    barrageInfo.text = barrageJson["barrageContent"].toString();
    barrageInfo.playTime = barrageJson["barrageTime"].toInteger();
    LOG()<<"视频"<<videoId<<"新增弹幕："<<barrageInfo.text<<" "<<barrageInfo.playTime;

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::getUserInfo(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    const QString userId = jsonReq["userId"].toString();
    LOG()<<"[getUserInfo] 收到 getUserInfo 请求， requestId = "<<jsonReq["requestId"].toString()
          << ", userId = "<<userId;

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 构造用户信息
    QJsonObject userInfoJson;
    if(userId.isEmpty()){
        // 当前用户个人信息
        userInfoJson["userId"] = "100001";             // 用户id
        userInfoJson["phoneNum"] = "15012345678";      // 用户手机号
        userInfoJson["nickname"] = "张三";             // 用户昵称
        // 角色类型：0-未知用户  1-超级管理员  2-普通管理员  3-普通用户  4-临时用户
        QJsonArray roleTyleArray;
        roleTyleArray.append(3);
        userInfoJson["roleType"] = roleTyleArray;     // 用户角色
        // 身份类型: 0-位置  1-C端用户   2-B端用户
        QJsonArray identityTypeArray;
        identityTypeArray.append(1);
        userInfoJson["identityType"] = identityTypeArray;     // 用户身份
        userInfoJson["likeCount"] = 12345;             // 点赞数
        userInfoJson["playCount"] = 12346;             // 播放数
        userInfoJson["followedCount"] = 123;           // 关注数
        userInfoJson["followerCount"] = 123;           // 粉丝数
        // 用户状态：0-无状态  1-启用  2-禁用---用户状态主要是给管理员页面使用
        userInfoJson["userStatus"] = 0;               // 管理员状态
        userInfoJson["isFollowing"] = 0;              // 用户是否被关注
        userInfoJson["userMemo"] = "";                // 备注信息
        userInfoJson["userCTime"] = "";               // 用户创建时间
        userInfoJson["avatarFileId"] = "10000";       // 用户头像id
    }else{
        // 其他用户个人信息
        userInfoJson["userId"] = "100002";             // 用户id
        userInfoJson["phoneNum"] = "15012345679";      // 用户手机号
        userInfoJson["nickname"] = "李四";             // 用户昵称
        // 角色类型：0-未知用户  1-超级管理员  2-普通管理员  3-普通用户  4-临时用户
        QJsonArray roleTyleArray;
        roleTyleArray.append(3);
        userInfoJson["roleType"] = roleTyleArray;     // 用户角色
        // 身份类型: 0-位置  1-C端用户   2-B端用户
        QJsonArray identityTypeArray;
        identityTypeArray.append(1);
        userInfoJson["identityType"] = identityTypeArray;     // 用户身份
        userInfoJson["likeCount"] = 23456;             // 点赞数
        userInfoJson["playCount"] = 23457;             // 播放数
        userInfoJson["followedCount"] = 234;           // 关注数
        userInfoJson["followerCount"] = 234;           // 粉丝数
        // 用户状态：0-无状态  1-启用  2-禁用---用户状态主要是给管理员页面使用
        userInfoJson["userStatus"] = 0;               // 管理员状态
        userInfoJson["isFollowing"] = 1;              // 用户是否被关注
        userInfoJson["userMemo"] = "";                // 备注信息
        userInfoJson["userCTime"] = "";               // 用户创建时间
        userInfoJson["avatarFileId"] = "20000";       // 用户头像id
    }

    QJsonObject resultJson;
    resultJson["userInfo"] = userInfoJson;
    jsonResp["result"] = resultJson;
    LOG()<<jsonResp;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setAvatar(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[setAvatar] 收到 setAvatar 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::userVideoList(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[userVideoList] 收到 userVideoList 请求， requestId = "<<jsonReq["requestId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    int pageCount = jsonReq["pageCount"].toInt();
    QJsonObject resultJsonObj;
    resultJsonObj["totalCount"] = 100;

    int videoId = 50000;
    int userId = 20000;
    int resourceId = 50000;
    QJsonArray videoLists;
    for(int i = 0; i < pageCount; ++i){
        QJsonObject videoJsonObj;
        videoJsonObj["videoId"] = QString::number(videoId++);
        videoJsonObj["userId"] = QString::number(userId++);
        videoJsonObj["userAvatarId"] = QString::number(resourceId++);
        videoJsonObj["nickname"] = "用户昵称";
        videoJsonObj["photoFileId"] = QString::number(resourceId++);
        videoJsonObj["videoFileId"] = QString::number(resourceId++);
        videoJsonObj["likeCount"] = 1234;
        videoJsonObj["playCount"] = 3456;
        videoJsonObj["videoSize"] = 10240;
        videoJsonObj["videoDesc"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩~";
        videoJsonObj["videoTitle"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoJsonObj["videoDuration"] = 10;
        videoJsonObj["videoUpTime"] = "9-16 12:28:58";
        videoJsonObj["videoStatus"] = rand()%4 + 1;
        videoJsonObj["checkerId"] = "1234";
        videoJsonObj["checkerName"] = "张三";
        videoJsonObj["checkerAvatar"] = "50000";

        videoLists.append(videoJsonObj);
    }
    resultJsonObj["videoList"] = videoLists;
    jsonResp["result"] = resultJsonObj;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::removeVideo(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[removeVideo] 收到 removeVideo 请求， requestId = "<<jsonReq["requestId"].toString()
         <<", videoId = "<<jsonReq["videoId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::newAttention(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[newAttention] 收到 newAttention 请求， requestId = "<<jsonReq["requestId"].toString()
          <<", userId = "<<jsonReq["userId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::delAttention(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[delAttention] 收到 delAttention 请求， requestId = "<<jsonReq["requestId"].toString()
          <<", userId = "<<jsonReq["userId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::getCode(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[getCode] 收到 getCode 请求， requestId = "<<jsonReq["requestId"].toString();

    // 要发送验证码的手机号
    QString phoneNum = jsonReq["phoneNumber"].toString();
    LOG()<<"给手机号："<<phoneNum<<"发送验证码 123456";

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonObject authcodeIdJson;
    authcodeIdJson["codeId"] = "111111";
    jsonResp["result"] = authcodeIdJson;


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::vcodeLogin(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[vcodeLogin] 收到 vcodeLogin 请求， requestId = "<<jsonReq["requestId"].toString();

    // 要发送验证码的手机号
    QString phoneNum = jsonReq["phoneNumber"].toString();
    QString authcode = jsonReq["verifyCode"].toString();
    QString authcodeId = jsonReq["codeId"].toString();
    LOG()<<"登录手机号："<<phoneNum<<" 验证码: "<<authcode<<" 验证码id："<<authcodeId;

    int errorCode = 0;
    QString errorMsg;
    if(phoneNum != "15012345678"){
        errorCode = 600;
        errorMsg = "手机号输入有误";
    }

    if(authcode != "123456"){
        errorCode = 601;
        errorMsg = "验证码输入有误";
    }

    if(authcodeId != "111111"){
        errorCode = 602;
        errorMsg = "验证码id输入有误";
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = errorCode;
    jsonResp["errorMsg"] = errorMsg;


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::passwdLogin(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[passwdLogin] 收到 passwdLogin 请求， requestId = "<<jsonReq["requestId"].toString();

    // 要发送验证码的手机号
    QString phoneNum = jsonReq["phoneNumber"].toString();
    QString password = jsonReq["password"].toString();
    LOG()<<"登录手机号："<<phoneNum<<" 密码: "<<password;

    int errorCode = 0;
    QString errorMsg;
    if(phoneNum != "15012345678"){
        errorCode = 603;
        errorMsg = "手机号输入有误";
    }

    if(password != "hello12345"){
        errorCode = 604;
        errorMsg = "密码输入有误";
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = errorCode;
    jsonResp["errorMsg"] = errorMsg;


    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::sessionLogin(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[sessionLogin] 收到 sessionLogin 请求， requestId = "<<jsonReq["requestId"].toString();
    LOG()<<"sessionId = "<<jsonReq["sessionId"].toString();


    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";
    QJsonObject resultJson;
    resultJson["isGuest"] = false;
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::logout(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[logout] 收到 logout 请求， requestId = "<<jsonReq["requestId"].toString();
    LOG()<<"sessionId = "<<jsonReq["sessionId"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";
    QJsonObject resultJson;
    resultJson["sessionId"] = jsonReq["sessionId"].toString();
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setPassword(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[setPassword] 收到 setPassword 请求， requestId = "<<jsonReq["requestId"].toString();
    LOG()<<"password = "<<jsonReq["password"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setNickname(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[setNickname] 收到 setNickname 请求， requestId = "<<jsonReq["requestId"].toString();
    LOG()<<"nickname = "<<jsonReq["nickname"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::newVideo(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[newVideo] 收到 newVideo 请求， requestId = "<<jsonReq["requestId"].toString();

    // 解析上传视频信息
    QJsonObject videoInfo = jsonReq["videoInfo"].toObject();
    QString videoFileId = videoInfo["videoFileId"].toString();
    QString photoFileId = videoInfo["photoFileId"].toString();
    QString videoTitle = videoInfo["videoTitle"].toString();
    int videoType = videoInfo["videoType"].toInt();
    QString videoDesc = videoInfo["videoDesc"].toString();
    int64_t duration = videoInfo["duration"].toInteger();
    LOG()<<"videoFileId = " << videoFileId;
    LOG()<<"photoFileId = " << photoFileId;
    LOG()<<"videoTitle = " << videoTitle;
    LOG()<<"videoType = " << videoType;
    LOG()<<"videoDesc = " << videoDesc;
    LOG()<<"duration = " << duration;

    QJsonArray tagsArray = videoInfo["videoTag"].toArray();
    for(int i = 0; i < tagsArray.size(); ++i){
        int tagId = tagsArray[i].toInt();
        LOG()<<"tagId"<<i<<" = "<<tagId;
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::statusVideoList(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[statusVideoList] 收到 statusVideoList 请求， requestId = "<<jsonReq["requestId"].toString();

    int videoId = 70000;
    int userId = 70000;
    int resourceId = 70000;
    QJsonArray videoLists;
    int pageCount = jsonReq["pageCount"].toInt();
    for(int i = 0; i < pageCount; ++i){
        QJsonObject videoJsonObj;
        videoJsonObj["videoId"] = QString::number(videoId++);
        videoJsonObj["userId"] = QString::number(userId++);
        videoJsonObj["userAvatarId"] = QString::number(resourceId++);
        videoJsonObj["nickname"] = "王五";
        videoJsonObj["photoFileId"] = QString::number(resourceId++);
        videoJsonObj["videoFileId"] = QString::number(resourceId++);
        videoJsonObj["likeCount"] = 1234;
        videoJsonObj["playCount"] = 3456;
        videoJsonObj["videoSize"] = 10240;
        videoJsonObj["videoDesc"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩一条视频告诉你去了北京该怎么玩"
                                    "一条视频告诉你去了北京该怎么玩~";
        videoJsonObj["videoTitle"] = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoJsonObj["videoDuration"] = 10;
        videoJsonObj["videoUpTime"] = "9-16 12:28:58";
        int videoStatus = jsonReq["videoStatus"].toInt();
        if(0 == videoStatus){
            videoJsonObj["videoStatus"] = rand()%4+1;
        }else{
            videoJsonObj["videoStatus"] = videoStatus;
        }

        videoJsonObj["checkerId"] = "12345";
        videoJsonObj["checkerName"] = "李四";
        videoJsonObj["checkerAvatar"] = "50000";

        videoLists.append(videoJsonObj);
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    QJsonObject resultJson;
    resultJson["videoList"] = videoLists;
    resultJson["totalCount"] = 200;
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::checkVideo(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[checkVideo] 收到 checkVideo 请求， requestId = "<<jsonReq["requestId"].toString();

    QString videoId = jsonReq["videoId"].toString();
    bool result = jsonReq["checkResult"].toBool();
    if(result){
        LOG()<<"视频"<<videoId<<"审核通过";
    }else{
        LOG()<<"视频"<<videoId<<"审核驳回";
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::saleVideo(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[saleVideo] 收到 saleVideo 请求， requestId = "<<jsonReq["requestId"].toString();

    QString videoId = jsonReq["videoId"].toString();
    LOG()<<"视频"<<videoId<<"上架";

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::haltVideo(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[haltVideo] 收到 haltVideo 请求， requestId = "<<jsonReq["requestId"].toString();

    QString videoId = jsonReq["videoId"].toString();
    LOG()<<"视频"<<videoId<<"下架";

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::getAdminByPhone(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[getAdminByPhone] 收到 getAdminByPhone 请求， requestId = "<<jsonReq["requestId"].toString();

    QString phoneNumber = jsonReq["phoneNumber"].toString();
    LOG()<<"管理员手机号："<<phoneNumber;

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 构造管理员信息的userInfo json对象
    QJsonObject userInfo;
    userInfo["userId"] = QString::number(1234);
    userInfo["nickname"] = "王五";
    userInfo["roleType"] = 2;
    userInfo["phoneNumber"] = phoneNumber;
    userInfo["userStatu"] = rand()%2 + 1;    // 状态：1表示启用  2表示禁用
    userInfo["userMemo"] = "视频审核管理员";

    QJsonObject resultJson;
    resultJson["userInfo"] = userInfo;
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::getAdminListByStatus(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[getAdminListByStatus] 收到 getAdminListByStatus 请求， requestId = "<<jsonReq["requestId"].toString();

    int adminStatus = jsonReq["userStatus"].toInt();
    if(0 == adminStatus){
        LOG()<<"获取所有管理员信息列表";
    }else if(1 == adminStatus){
        LOG()<<"获取启用管理员信息列表";
    }else{
        LOG()<<"获取禁用管理员信息列表";
    }

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 构造管理员信息的userInfo json对象
    int pageCount = jsonReq["pageCount"].toInt();
    QStringList nicknames = {"张三", "李四", "王五", "赵六", "田七"};
    QStringList remarks = {"视频审核", "运营人员1", "运营人员2"};
    QJsonArray userList;
    const QString phoneNumber = "15012345679";
    int adminId = 1234;
    for(int i = 0; i < pageCount; ++i){
        QJsonObject adminInfoJson;
        adminInfoJson["userId"] = QString::number(adminId++);
        adminInfoJson["nickname"] = nicknames[rand()%5];
        adminInfoJson["roleType"] = 2;
        adminInfoJson["phoneNumber"] = phoneNumber;
        if(0 == adminStatus){
            adminInfoJson["userStatu"] = rand()%2 + 1;    // 状态：1表示启用  2表示禁用
        }else{
            adminInfoJson["userStatu"] = adminStatus;
        }

        adminInfoJson["userMemo"] = remarks[rand()%3];

        userList.append(adminInfoJson);
    }

    QJsonObject resultJson;
    resultJson["userList"] = userList;
    resultJson["totalCount"] = 200;
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::newAdministrator(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[newAdministrator] 收到 newAdministrator 请求， requestId = "<<jsonReq["requestId"].toString();

    QJsonObject adminJson = jsonReq["userInfo"].toObject();
    LOG()<<"新增管理员信息：";
    LOG()<<"nickname = "<<adminJson["nickname"].toString();
    int roleType = adminJson["roleType"].toInt();
    if(1 == roleType){
        LOG()<<"管理员角色：超级管理员";
    }else if(2 == roleType){
        LOG()<<"管理员角色：平台管理员";
    }
    int adminStatus = adminJson["userStatu"].toInt();
    if(1 == adminStatus){
        LOG()<<"管理员状态：启用";
    }else if(2 == adminStatus){
        LOG()<<"管理员状态：禁用";
    }
    LOG()<<"管理员备注："<<adminJson["userMemo"].toString();
    LOG()<<"管理员手机号："<<adminJson["phoneNumber"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";


    QJsonObject resultJson;
    resultJson["userId"] = 23456;
    jsonResp["result"] = resultJson;

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setAdministrator(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[newAdministrator] 收到 newAdministrator 请求， requestId = "<<jsonReq["requestId"].toString();

    QJsonObject adminJson = jsonReq["userInfo"].toObject();
    LOG()<<"新增管理员信息：";
    LOG()<<"编辑管理员id = "<<adminJson["userId"].toString();
    LOG()<<"nickname = "<<adminJson["nickname"].toString();
    int adminStatus = adminJson["userStatus"].toInt();
    if(1 == adminStatus){
        LOG()<<"管理员状态：启用";
    }else if(2 == adminStatus){
        LOG()<<"管理员状态：禁用";
    }
    LOG()<<"管理员备注："<<adminJson["userMemo"].toString();

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::setStatus(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[setStatus] 收到 setStatus 请求， requestId = "<<jsonReq["requestId"].toString();

    QString userId = jsonReq["userId"].toString();
    int adminStatus = jsonReq["userStatus"].toInt();
    QString status = "启用";
    if(2 == adminStatus){
        status = "禁用";
    }
    LOG()<<"修改管理员id = "<<userId<<"的状态为"<<status;

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}

QHttpServerResponse HttpServer::delAdministrator(const QHttpServerRequest &request)
{
    // 1. 获取到请求中数据
    QJsonDocument docReq = QJsonDocument::fromJson(request.body());
    const QJsonObject& jsonReq = docReq.object();
    LOG()<<"[delAdministrator] 收到 delAdministrator 请求， requestId = "<<jsonReq["requestId"].toString();

    QString userId = jsonReq["userId"].toString();
    LOG()<<"删除管理员id = "<<userId<<"成功!!!";

    // 2. 构造响应体
    QJsonObject jsonResp;
    jsonResp["requestId"] = jsonReq["requestId"].toString();
    jsonResp["errorCode"] = 0;
    jsonResp["errorMsg"] = "";

    // 3. 返回响应
    QJsonDocument docResp;
    docResp.setObject(jsonResp);

    QHttpServerResponse httpResp(docResp.toJson(), QHttpServerResponse::StatusCode::Ok);
    httpResp.setHeader("Content-Type", "application/json; charset=utf-8");
    return httpResp;
}
