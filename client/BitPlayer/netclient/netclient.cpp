#include "netclient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QStandardPaths>
#include <QJsonArray>
#include <QUrlQuery>
#include "./../util.h"
#include "../model/datacenter.h"

namespace network {

NetClient::NetClient(model::DataCenter* dataCenter)
    :dataCenter(dataCenter)
{}

void NetClient::setHTTPURL(const QString &HTTP_URL)
{
    this->HTTP_URL = HTTP_URL;
}

QString NetClient::makeRequestId()
{
    // 请求ID以R开头，保留UUID中从第25为开始的后12位
    return  "R" + QUuid::createUuid().toString().sliced(25, 12);
}

QNetworkReply *NetClient::sendHttpRequest(const QString &resourcePath, QJsonObject &reqBody)
{
    reqBody["requestId"] = makeRequestId();

    QNetworkRequest httpReq;
    httpReq.setUrl(HTTP_URL + resourcePath);
    httpReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf8");

    // 2. 发送请求
    QJsonDocument docment(reqBody);
    QNetworkReply* httpReply = httpClient.post(httpReq, docment.toJson());
    return httpReply;
}

QJsonObject NetClient::handleHttpResponse(QNetworkReply *httpReply, bool &ok, QString &reason)
{
    // a. 判定http层面是否出错
    if(httpReply->error() != QNetworkReply::NoError){
        ok = false;
        reason = httpReply->errorString();
        httpReply->deleteLater();
        return QJsonObject();
    }

    // b. 获取到响应体-需要对响应体进行反序列化
    QByteArray replyBody = httpReply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyBody);
    if(jsonDoc.isNull()){
        ok = false;
        reason = "解析 JSON 文件失败! JSON 文件格式有误!";
        httpReply->deleteLater();
        return QJsonObject();
    }

    // c. 判定业务上的逻辑是否正确
    QJsonObject replyObj = jsonDoc.object();
    if(replyObj["errorCode"].toInt() != 0){
        ok = false;
        reason = replyObj["errorMsg"].toString();
        httpReply->deleteLater();
        return replyObj;
    }

    // d. 释放httpReply对象并返回反序列化之后的json数据
    ok = true;
    httpReply->deleteLater();
    return replyObj;
}

/*
 * 请求URL post /hello
 * {
 *     requestId : 1234
 * }
 */
void NetClient::hello()
{
    // 1. 构造请求
    // 请求体 和 请求头部信息
    QJsonObject reqBody;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/hello", reqBody);

    // 3. 异步处理请求
    // post方法执行完成之后会立马返回，但此时并不一定能立马收到服务端的响应
    // 当客户端收到服务端的响应之后，QNetworkReply 会出发finished的信号
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"hello 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject userData = replyObj["data"].toObject();
        LOG()<<userData["hello"].toString();

        // 数据保存
        // dataCenter->saveData(...);
        // d. 通过界面做一些数据的展示处理(根据发送的请求来确定是否需要)
        emit dataCenter->helloDone();
    });
}

/*
 * 请求URL post /ping
 * {
 *     requestId : 1234
 * }
 */

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
void NetClient::ping()
{
    // 1. 构造ping请求
    // 请求体 和 请求头部信息
    QJsonObject reqBody;

    // 2. 发送ping请求
    QNetworkReply* httpReply = sendHttpRequest("/ping", reqBody);

    // 3. 异步处理ping请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"ping 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject userData = replyObj["data"].toObject();
        LOG()<<userData["ping"].toString();

        // d. 通过界面做一些数据的展示处理(根据发送的请求来确定是否需要)
        emit dataCenter->pingDone();
    });
}

void NetClient::loginTempUser()
{
    // 1. 构造请求体
    QJsonObject reqBody;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/tempLogin", reqBody);

    // 3. 异步处理tempLogin请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"tempLogin 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject resultObj = replyObj["result"].toObject();
        dataCenter->setSessionId(resultObj["sessionId"].toString());
        LOG()<<"tempLogin 登录成功, resquestId = "<<replyObj["requestId"].toString();
        // d. 通过界面做一些数据的展示处理(根据发送的请求来确定是否需要)
        emit dataCenter->loginTempUserDone();
    });
}

void NetClient::getAllVideoList()
{
    // 1. 构造请求体
    auto videoListPtr = dataCenter->getVideoListPtr();
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["pageIndex"] = videoListPtr->getPageIndex();
    reqBody["pageCount"] = model::VideoList::PAGE_COUNT;
    videoListPtr->setPageIndex(videoListPtr->getPageIndex()+1);

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/allVideoList", reqBody);

    // 3. 异步处理allVideoList请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"allVideoList 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject resultObj = replyObj["result"].toObject();
        // 将获取到的视频信息保存到视频列表
        dataCenter->setVideoList(resultObj);

        // d. 统计界面显示视频信息
        emit dataCenter->getAllVideoListDone();
        LOG()<<"allVideoList 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getAllVideoListInKind(int kindId)
{
    // 1. 构造请求体
    auto videoListPtr = dataCenter->getVideoListPtr();
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoTypeId"] = kindId;
    reqBody["pageIndex"] = videoListPtr->getPageIndex();
    reqBody["pageCount"] = model::VideoList::PAGE_COUNT;

    videoListPtr->setPageIndex(videoListPtr->getPageIndex()+1);

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/typeVideoList", reqBody);

    // 3. 异步处理typeVideoList请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"typeVideoList 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject resultObj = replyObj["result"].toObject();
        // 将获取到的视频信息保存到视频列表
        dataCenter->setVideoList(resultObj);

        // d. 统计界面显示视频信息
        emit dataCenter->getAllVideoListInKindDone();
        LOG()<<"typeVideoList 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getAllVideoListInTag(int tagId)
{
    // 1. 构造请求体
    auto videoListPtr = dataCenter->getVideoListPtr();
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoTag"] = tagId;
    reqBody["pageIndex"] = videoListPtr->getPageIndex();
    reqBody["pageCount"] = model::VideoList::PAGE_COUNT;

    videoListPtr->setPageIndex(videoListPtr->getPageIndex()+1);

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/tagVideoList", reqBody);

    // 3. 异步处理tagVideoList请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"tagVideoList 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject resultObj = replyObj["result"].toObject();
        // 将获取到的视频信息保存到视频列表
        dataCenter->setVideoList(resultObj);

        // d. 统计界面显示视频信息
        emit dataCenter->getAllVideoListInTagDone();
        LOG()<<"tagVideoList 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getAllVideoListSearchText(const QString &searchText)
{
    // 1. 构造请求体
    auto videoListPtr = dataCenter->getVideoListPtr();
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["searchKey"] = searchText;
    reqBody["pageIndex"] = videoListPtr->getPageIndex();
    reqBody["pageCount"] = model::VideoList::PAGE_COUNT;

    videoListPtr->setPageIndex(videoListPtr->getPageIndex()+1);

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/keyVideoList", reqBody);

    // 3. 异步处理keyVideoList请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"keyVideoList 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject resultObj = replyObj["result"].toObject();
        // 将获取到的视频信息保存到视频列表
        dataCenter->setVideoList(resultObj);

        // d. 统计界面显示视频信息
        emit dataCenter->getAllVideoListSearchTextDone();
        LOG()<<"keyVideoList 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::downloadPhoto(const QString &photoFileId)
{
    // 1. 构造请求
    QString queryString;
    queryString += "requestId=";
    queryString += makeRequestId();
    queryString += "&";
    queryString += "sessionId=";
    queryString += dataCenter->getLoginSessionId();
    queryString += "&";
    queryString += "fileId=";
    queryString += photoFileId;

    // 2. 发送请求
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + "/HttpService/downloadPhoto?" + queryString));
    QNetworkReply* httpReply = httpClient.get(httpReq);

    // 3. 处理响应结果
    connect(httpReply, &QNetworkReply::finished, this, [=]{
        // 判定HTTP层面是否出错
        if(httpReply->error() != QNetworkReply::NoError){
            LOG()<<httpReply->errorString();
            httpReply->deleteLater();
            return;
        }

        // 获取图片数据
        QByteArray imageData = httpReply->readAll();
        emit dataCenter->downloadPhotoDone(photoFileId, imageData);
        httpReply->deleteLater();
        LOG()<<"downloadPhoto请求结束，下载图片成功，图片id="<<photoFileId;
    });
}

void NetClient::uploadPhoto(const QByteArray &photoData, QWidget* whichPage)
{
    // 1. 构造请求
    QString queryString;
    queryString += "requestId=";
    queryString += makeRequestId();
    queryString += "&";
    queryString += "sessionId=";
    queryString += dataCenter->getLoginSessionId();

    // 2. 发送请求
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + "/HttpService/uploadPhoto?" + queryString));
    httpReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    QNetworkReply* httpReply = httpClient.post(httpReq, photoData);

    // 3. 处理响应结果
    connect(httpReply, &QNetworkReply::finished, this, [=]{
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"uploadPhoto 请求出错，reason = "<<reason;
            return;
        }

        const QString& requestId = replyObj["requestId"].toString();
        QJsonObject resultJosn = replyObj["result"].toObject();
        const QString& fileId = resultJosn["fileId"].toString();

        // c. 发射信号，通过界面更新数据
        emit dataCenter->uploadPhotoDone(fileId, whichPage);
        LOG()<<"uploadPhoto请求结束，上传图片成功，requestId = "<<requestId<<", fileId = "<<fileId;
    });
}

void NetClient::downloadVideo(const QString &videoFileId)
{
    // 1. 构造请求
    QString queryString;
    queryString += "fileId=";
    queryString += videoFileId;

    // 2. 发送请求
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + "/HttpService/downloadVideo?" + queryString));
    QNetworkReply* httpReply = httpClient.get(httpReq);

    // 3. 处理响应结果
    connect(httpReply, &QNetworkReply::finished, this, [=]{
        // 判定HTTP层面是否出错
        if(httpReply->error() != QNetworkReply::NoError){
            LOG()<<httpReply->errorString();
            httpReply->deleteLater();
            return;
        }

        // 获取m3u8文件数据
        // 将m3u8文件保存在本地，然后将路径交给mpv来进行播放
        QByteArray m3u8Data = httpReply->readAll();
        QString m3u8FilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m3u8FilePath += "";
        m3u8FilePath += videoFileId;
        m3u8FilePath += ".m3u8";
        LOG()<<m3u8FilePath;

        writeByteArrayToFile(m3u8FilePath, m3u8Data);

        emit dataCenter->downloadVideoDone(m3u8FilePath, videoFileId);
        httpReply->deleteLater();
        LOG()<<"downloadVideo请求结束，下载视频成功，视频文件id="<<videoFileId;
    });
}

void NetClient::uploadVideoAsync(const QString &videoPath)
{
    // 1. 构造请求
    // QString queryString;
    // queryString += "requestId=";
    // queryString += makeRequestId();
    // queryString += "&";
    // queryString += "sessionId=";
    // queryString += dataCenter->getLoginSessionId();
    // 构建请求对象，并设置url基础部分
    QUrl url(HTTP_URL + "/HttpService/uploadVideo");
    //添加请求参数到查询对象中
    QUrlQuery query;
    query.addQueryItem("requestId", makeRequestId());
    query.addQueryItem("sessionId", dataCenter->getLoginSessionId());
    url.setQuery(query);

    // 读取文件数据
    QByteArray videoData = loadFileToByteArray(videoPath);

    // 2. 发送请求
    QNetworkRequest httpReq(url);
    httpReq.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
    QNetworkReply* httpReply = httpClient.post(httpReq, videoData);

    // 3. 处理响应结果
    connect(httpReply, &QNetworkReply::finished, this, [=]{
        // 解析响应
        bool ok = false;
        QString reason;
        QJsonObject bodyObject = handleHttpResponse(httpReply, ok, reason);

        // 判定响应是否出错
        if(!ok){
            LOG()<<"uploadVideo请求出错："<<reason;
            return;
        }

        // 解析出视频文件id
        const QString requestId = bodyObject["requestId"].toString();
        QJsonObject resultJson = bodyObject["result"].toObject();
        QString fileId = resultJson["fileId"].toString();

        emit dataCenter->uploadVideoDone(fileId);
        LOG()<<"uploadVideo请求结束，上传视频成功，视频文件id="<<fileId<<" request = "<<requestId;
    });
}

void NetClient::getVideoBarrage(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/getBarrage", reqBody);

    // 3. 异步处理getBarrage请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"getBarrage 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析响应体中服务端交给客户端的具体数据
        QJsonObject resultObj = replyObj["result"].toObject();
        // 将获取到的视频信息保存到视频列表
        dataCenter->setBarrageData(resultObj["barrageList"].toArray());

        // d. 统计界面显示视频信息
        emit dataCenter->getVideoBarrageDone(videoId);
        LOG()<<"getBarrage 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::setPlayNumber(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setPlay", reqBody);

    // 3. 异步处理setPlay请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setPlay 请求出错，reason = "<<reason;
            return;
        }

        LOG()<<"setPlay 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getIsLikeVdieo(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/judgeLike", reqBody);

    // 3. 异步处理judgeLike请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"judgeLike 请求出错，reason = "<<reason;
            return;
        }
        // c. 发送信号通过界面更新
        QJsonObject resultJson = replyObj["result"].toObject();
        emit dataCenter->getIsLikeVdieoDone(videoId, resultJson["isLike"].toBool());
        LOG()<<"judgeLike 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::setLikeNumber(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setLike", reqBody);

    // 3. 异步处理 setLike 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setLike 请求出错，reason = "<<reason;
            return;
        }

        LOG()<<"setLike 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::loadupBarrage(const QString &videoId, model::BarrageInfo barrageInfo)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 构造弹幕信息
    QJsonObject barrageJson;
    barrageJson["barrageContent"] = barrageInfo.text;
    barrageJson["barrageTime"] = barrageInfo.playTime;
    reqBody["barrageInfo"] = barrageJson;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/newBarrage", reqBody);

    // 3. 异步处理 newBarrage 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"newBarrage 请求出错，reason = "<<reason;
            return;
        }

        LOG()<<"newBarrage 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getUserInfo(const QString &userId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();

    if(!userId.isEmpty()){
        // 获取他人的用户信息
        reqBody["userId"] = userId;
    }else{
        // 获取自己的个人信息
    }

    // 其他同学的代码
    
    if(userId.isEmpty()){
        LOG()<<"获取当前用户的个人信息";
    }else{
        LOG()<<"获取其他用户的个人信息，userId = "<<userId;
    }

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/getUserInfo", reqBody);

    // 3. 异步处理 getUserInfo 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"getUserInfo 请求出错，reason = "<<reason;
            return;
        }

        // c.将个人信息保存到DataCenter中
        QJsonObject resultJson = replyObj["result"].toObject();
        LOG()<<resultJson;
        QJsonObject userInfoJson = resultJson["userInfo"].toObject();
        if(userId.isEmpty()){
            // 获取当前当用户个人信息
            dataCenter->setMyselfInfo(userInfoJson);
            emit dataCenter->getMyselfInfoDone();
            LOG()<<"getUserInfo 当前用户信息成功, resquestId = "<<replyObj["requestId"].toString();
        }else{
            // 获取其他用户个人信息
            dataCenter->setOtherUserInfo(userInfoJson);
            emit dataCenter->getOtherUserInfoDone();
            LOG()<<"getUserInfo 其他用户信息成功, resquestId = "<<replyObj["requestId"].toString();
        }

    });
}

void NetClient::setAvatar(const QString &fileId)
{
    // 注意：修改用户头像id之前，确保用户头像的图片已经上传了服务器
    // 此处的fileId就是服务器返回的 该图片的id
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["fileId"] = fileId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setAvatar", reqBody);

    // 3. 异步处理 setAvatar 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setAvatar 请求出错，reason = "<<reason;
            return;
        }

        // c.将个人信息保存到DataCenter中
        dataCenter->setAvatar(fileId);

        // d. 发射信号，通知界面更新头像
        emit dataCenter->setAvatarDone();
        LOG()<<"setAvatar 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getUserVideoList(const QString &userId, int pageIndex, model::VideoStatus videoStatus, const QString& whichPage)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    if(userId.isEmpty()){
        LOG()<<"获取当前用户自己的视频列表";
    }else{
        reqBody["userId"] = userId;
        LOG()<<"获取"<<userId<<"用户自己的视频列表";
    }

    reqBody["pageIndex"] = pageIndex;
    reqBody["pageCount"] = model::VideoList::PAGE_COUNT;
    reqBody["videoStatus"] = videoStatus;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/userVideoList", reqBody);

    // 3. 异步处理 userVideoList 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"userVideoList 请求出错，reason = "<<reason;
            return;
        }

        // c.将个人信息保存到DataCenter中
        QJsonObject resultJson = replyObj["result"].toObject();
        if("myPage" == whichPage){
            dataCenter->setUserVideoList(resultJson);
            // d. 发射信号，通知界面更新头像
            emit dataCenter->getUserVideoListDone(userId, whichPage);
            if(userId.isEmpty()){
                LOG()<<"userVideoList 当前用户视频列表成功, resquestId = "<<replyObj["requestId"].toString();
            }else{
                LOG()<<"userVideoList 其他用户视频列表成功, resquestId = "<<replyObj["requestId"].toString();
            }
        }else if("checkPage" == whichPage){
            dataCenter->setStatusVideoList(resultJson);
            // d. 发射信号，通知界面更新头像
            emit dataCenter->getUserVideoListDone(userId, whichPage);
            LOG()<<"userVideoList 指定用户视频列表成功, resquestId = "<<replyObj["requestId"].toString();
        }
    });
}

void NetClient::deleteVideo(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/removeVideo", reqBody);

    // 3. 异步处理 removeVideo 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"removeVideo 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面更新删除视频
        emit dataCenter->deleteVideoDone(videoId);
        LOG()<<"removeVideo 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::newAttention(const QString &userId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["userId"] = userId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/newAttention", reqBody);

    // 3. 异步处理 newAttention 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"newAttention 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面更新关注信息
        emit dataCenter->newAttentionDone();
        LOG()<<"newAttention 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::delAttention(const QString &userId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["userId"] = userId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/delAttention", reqBody);

    // 3. 异步处理 delAttention 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"delAttention 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面取消关注信息
        emit dataCenter->delAttentionDone();
        LOG()<<"delAttention 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getAuthcode(const QString &email)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["email"] = email;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/getCode", reqBody);

    // 3. 异步处理 getCode 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"getCode 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面输入验证码
        QJsonObject resultJson = replyObj["result"].toObject();
        QString authcodeId = resultJson["codeId"].toString();
        emit dataCenter->getAuthcodeDone(authcodeId);
        LOG()<<"getCode 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::loginWithEmail(const QString &email, const QString &authcode, const QString &authcodeId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["email"] = email;
    reqBody["verifyCode"] = authcode;
    reqBody["codeId"] = authcodeId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/vcodeLogin", reqBody);

    // 3. 异步处理 vcodeLogin 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"vcodeLogin 请求出错，reason = "<<reason;
            emit dataCenter->loginWithEmailFailed(reason);
            return;
        }

        // c.发射信号，通知界面输入验证码
        emit dataCenter->loginWithEmailDone();
        LOG()<<"vcodeLogin 登录成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::loginWithPassword(const QString &username, const QString &password)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["username"] = username;
    reqBody["password"] = password;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/passwdLogin", reqBody);

    // 3. 异步处理 passwdLogin 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"passwdLogin 请求出错，reason = "<<reason;
            emit dataCenter->loginWithPasswordFailed(reason);
            return;
        }

        // c.发射信号，通知界面登录成功
        emit dataCenter->loginWithPasswordDone();
        LOG()<<"passwdLogin 登录成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::loginSession()
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/sessionLogin", reqBody);

    // 3. 异步处理 sessionLogin 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"sessionLogin 请求出错，reason = "<<reason;
            emit dataCenter->loginSessionFailed(reason);
            return;
        }

        // c.发射信号，通知界面登录成功
        QJsonObject resultJson = replyObj["result"].toObject();
        bool isTempUser = resultJson["isGuest"].toBool();
        emit dataCenter->loginSessionDone(isTempUser);
        LOG()<<"sessionLogin 登录成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::logout()
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/logout", reqBody);

    // 3. 异步处理 logout 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"logout 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面登录成功
        QJsonObject resultJson = replyObj["result"].toObject();
        QString sessionId = resultJson["sessionId"].toString();
        dataCenter->setSessionId(sessionId);
        emit dataCenter->logoutDone();
        LOG()<<"logout 退出登录成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::setPassword(const QString &password)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["password"] = password;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setPassword", reqBody);

    // 3. 异步处理 setPassword 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setPassword 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面登录成功
        emit dataCenter->setPasswordDone();
        LOG()<<"setPassword 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::setNickname(const QString &nickname)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["nickname"] = nickname;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setNickname", reqBody);

    // 3. 异步处理 setNickname 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setNickname 请求出错，reason = "<<reason;
            emit dataCenter->setNicknameFailed(reason);
            return;
        }

        // c.发射信号，通知界面登录成功
        emit dataCenter->setNicknameDone(nickname);
        LOG()<<"setNickname 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::uploadVideoDesc(const model::VideoDesc &videoDesc)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();

    // 构造视频信息Json对象
    QJsonObject videoInfoJson;
    videoInfoJson["videoFileId"] = videoDesc.videoFileId;
    videoInfoJson["photoFileId"] = videoDesc.photoFileId;
    videoInfoJson["videoTitle"] = videoDesc.videoTitle;
    videoInfoJson["duration"] = videoDesc.duration;
    videoInfoJson["videoDesc"] = videoDesc.videoDesc;

    auto kindAndTag = dataCenter->getKindAndTagClassPtr();
    if(!videoDesc.kind.isEmpty()){
        // 分类
        videoInfoJson["videoType"] = kindAndTag->getKindId(videoDesc.kind);

        // 标签--联调时可能会出问题
        QJsonArray tagsArray;
        for(auto& tag : videoDesc.tags){
            tagsArray.append(kindAndTag->getTagId(videoDesc.kind, tag));
        }

        videoInfoJson["videoTag"] = tagsArray;
    }

    reqBody["videoInfo"] = videoInfoJson;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/newVideo", reqBody);

    // 3. 异步处理 newVideo 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"newVideo 请求出错，reason = "<<reason;
            return;
        }

        // c.发射信号，通知界面登录成功
        emit dataCenter->uploadVideoDescDone();
        LOG()<<"newVideo 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getStatusVideoList(model::VideoStatus videoStatus, int pageIndex)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoStatus"] = videoStatus;
    reqBody["pageIndex"] = pageIndex;
    reqBody["pageCount"] = model::VideoList::PAGE_COUNT;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/statusVideoList", reqBody);

    // 3. 异步处理 statusVideoList 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"statusVideoList 请求出错，reason = "<<reason;
            return;
        }

        // c.将个人信息保存到DataCenter中
        QJsonObject resultJson = replyObj["result"].toObject();
        dataCenter->setStatusVideoList(resultJson);
        emit dataCenter->getStatusVideoListDone();
        LOG()<<"statusVideoList 指定用户视频列表成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::checkVideo(const QString &videoId, bool result)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;
    reqBody["checkResult"] = result;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/checkVideo", reqBody);

    // 3. 异步处理 checkVideo 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"checkVideo 请求出错，reason = "<<reason;
            return;
        }

        // c. 发射信号通知界面完成数据的更新
        emit dataCenter->checkVideoDone();
        LOG()<<"checkVideo 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::putawayVideo(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/saleVideo", reqBody);

    // 3. 异步处理 saleVideo 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"saleVideo 请求出错，reason = "<<reason;
            return;
        }

        // c. 发射信号通知界面完成数据的更新
        emit dataCenter->putawayVideoDone();
        LOG()<<"saleVideo 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::discardVideo(const QString &videoId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["videoId"] = videoId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/haltVideo", reqBody);

    // 3. 异步处理 discardVideo 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"discardVideo 请求出错，reason = "<<reason;
            return;
        }

        // c. 发射信号通知界面完成数据的更新
        emit dataCenter->discardVideoDone();
        LOG()<<"discardVideo 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getAdminInfoByEmail(const QString &email)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["email"] = email;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/getAdminByEmail", reqBody);

    // 3. 异步处理 getAdminByEmail 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"getAdminByEmail 请求出错，reason = "<<reason;
            emit dataCenter->getAdminInfoByEmailFailed();
            return;
        }

        // c. 解析出管理员信息并保存到DataCenter中
        QJsonObject resultJson = replyObj["result"].toObject();
        dataCenter->setAdminList(resultJson, false);

        // d. 发射信号通知界面完成数据的更新
        emit dataCenter->getAdminInfoByEmailDone();
        LOG()<<"getAdminByEmail 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::getAdminInfoByStatus(int pageIndex, model::AdminStatus adminStatus)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["pageIndex"] = pageIndex;
    reqBody["pageCount"] = model::AdminList::PAGE_COUNT;
    reqBody["userStatus"] = adminStatus;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/getAdminListByStatus", reqBody);

    // 3. 异步处理 getAdminListByStatus 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"getAdminListByStatus 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析出管理员信息并保存到DataCenter中
        QJsonObject resultJson = replyObj["result"].toObject();
        dataCenter->setAdminList(resultJson, true);

        // d. 发射信号通知界面完成数据的更新
        emit dataCenter->getAdminInfoByStatusDone();
        LOG()<<"getAdminListByStatus 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::newAdmin(const model::AdminInfo &adminInfo)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();

    QJsonObject userInfo;
    userInfo["nickname"] = adminInfo.nickname;
    userInfo["roleType"] = adminInfo.roleType;
    userInfo["userStatus"] = adminInfo.userStatu;
    userInfo["userMemo"] = adminInfo.userMemo;
    userInfo["email"] = adminInfo.email;
    reqBody["userInfo"] = userInfo;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/newAdministrator", reqBody);

    // 3. 异步处理 newAdministrator 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"newAdministrator 请求出错，reason = "<<reason;
            return;
        }

        // c. 解析出管理员信息并保存到DataCenter中
        QJsonObject resultJson = replyObj["result"].toObject();
        LOG()<<"新增管理员的id = "<<resultJson["userId"].toString();

        // d. 发射信号通知界面完成数据的更新
        emit dataCenter->newAdminDone();
        LOG()<<"newAdministrator 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::editAdmin(const model::AdminInfo &adminInfo)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();

    QJsonObject userInfo;
    userInfo["userId"] = adminInfo.userId;
    userInfo["nickname"] = adminInfo.nickname;
    userInfo["userStatus"] = adminInfo.userStatu;
    userInfo["userMemo"] = adminInfo.userMemo;
    reqBody["userInfo"] = userInfo;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setAdministrator", reqBody);

    // 3. 异步处理 setAdministrator 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setAdministrator 请求出错，reason = "<<reason;
            return;
        }

        // c. 发射信号通知界面完成数据的更新
        emit dataCenter->editAdminDone(adminInfo.userId);
        LOG()<<"setAdministrator 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::setAdminStatus(const model::AdminInfo &adminInfo)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["userId"] = adminInfo.userId;
    reqBody["userStatus"] = adminInfo.userStatu;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/setStatus", reqBody);

    // 3. 异步处理 setStatus 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"setStatus 请求出错，reason = "<<reason;
            return;
        }

        // c. 发射信号通知界面完成数据的更新
        emit dataCenter->setAdminStatusDone();
        LOG()<<"setStatus 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

void NetClient::delAdmin(const QString &userId)
{
    // 1. 构造请求体
    QJsonObject reqBody;
    reqBody["sessionId"] = dataCenter->getLoginSessionId();
    reqBody["userId"] = userId;

    // 2. 发送请求
    QNetworkReply* httpReply = sendHttpRequest("/HttpService/delAdministrator", reqBody);

    // 3. 异步处理 delAdministrator 请求的响应
    connect(httpReply, &QNetworkReply::finished, this, [=](){
        // a. 解析Http响应
        bool ok = false;
        QString reason;
        QJsonObject replyObj = handleHttpResponse(httpReply, ok, reason);

        // b. 判定响应是否出错
        if(!ok){
            LOG()<<"delAdministrator 请求出错，reason = "<<reason;
            return;
        }

        // c. 发射信号通知界面完成数据的更新
        emit dataCenter->delAdminDone();
        LOG()<<"delAdministrator 成功, resquestId = "<<replyObj["requestId"].toString();
    });
}

}// end network





