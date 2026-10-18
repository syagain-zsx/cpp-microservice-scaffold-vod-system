#include "datacenter.h"
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QFileInfo>
#include <QSettings>
#include "../util.h"
namespace  model {

DataCenter* DataCenter::instance = nullptr;

DataCenter* DataCenter::getInstance()
{
    if(nullptr == instance){
        instance = new DataCenter();
    }

    // will的代码
    // other 的代码
    return instance;
}



DataCenter::DataCenter(QObject *parent)
    : QObject{parent}
    , httpClient(this)
{
    // 将JSON中的数据读取到文件
    loadDataFile();
    // 将serverurl交给httpClient
    httpClient.setHTTPURL(serverUrl);
}

DataCenter::~DataCenter()
{
    delete kindAndTag;
}

void DataCenter::initDataFile()
{
    // 构造出要保存的文件的路径  保存到系统 appdata目录
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = basePath + "/BitPlayer.json";
    LOG()<<filePath;

    // 检测路径是否存在
    QDir dir;
    if(!dir.exists(basePath)){
        dir.mkpath(basePath);
    }

    // 在构造好的路径中创建文件
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        LOG()<<"打开文件失败! "<<file.errorString();
        return;
    }

    // 打开文件成功，往文件中写入一些输出的内容
    QString data = "{\n\n}";
    file.write(data.toUtf8());
    file.close();
}

void DataCenter::saveDataFile()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = basePath + "/BitPlayer.json";

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        LOG()<<"打开文件失败! "<<file.errorString();
        return;
    }

    // 将要写入的数据以json的方式组织起来
    // sessionId
    QJsonObject jsonObj;
    jsonObj["loginSessionId"] = loginSessionId;
    // 用户类型信息：用户类型 和 用户身份
    QJsonArray roleTypeArray;
    for(auto& role : myselfInfo->roleType){
        roleTypeArray.append(role);
    }
    jsonObj["roleType"] = roleTypeArray;

    QJsonArray identityTypeArray;
    for(auto& id : myselfInfo->identityType){
        identityTypeArray.append(id);
    }
    jsonObj["identityType"] = identityTypeArray;

    // 需要将json写入到文件中
    QJsonDocument jsonDoc(jsonObj);
    QString data = jsonDoc.toJson();
    file.write(data.toUtf8());

    file.close();
}

void DataCenter::loadDataFile()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = basePath + "/BitPlayer.json";

    // 判定文件是否存在，不存在则创建，此处创建一个空白的json文件
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists()){
        initDataFile();
    }

    // 以读的方式打开文件
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        LOG()<<"打开文件失败! "<<file.errorString();
        return;
    }

    // 读取文件内容，解析为JSON对象
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    if(jsonDoc.isNull()){
        LOG()<<"解析 JSON 文件失败！JSON格式有错误!";
        return;
    }

    // 从JSON中将数据解析出来
    QJsonObject jsonObj = jsonDoc.object();
    // sessionId
    loginSessionId = jsonObj["loginSessionId"].toString();
    // roleType
    if(nullptr == myselfInfo){
        myselfInfo = new UserInfo();
    }
    QJsonArray roleTypeArray = jsonObj["roleType"].toArray();
    for(int i = 0; i < roleTypeArray.size(); ++i){
        myselfInfo->roleType.append(roleTypeArray[i].toInt());
    }
    // identityType
    QJsonArray identityTypeArray = jsonObj["identityType"].toArray();
    for(int i = 0; i < identityTypeArray.size(); ++i){
        myselfInfo->identityType.append(identityTypeArray[i].toInt());
    }

    // 加载服务器的配置信息：ip地址 和 端口号----config.ini文件保存在exe所在目录
    QDir dir(QDir::currentPath());
    QString configPath = dir.absolutePath();
    configPath += "/config.ini";

    // 读取ini文件中信息
    QSettings config(configPath, QSettings::IniFormat);
    config.beginGroup("server");    // 进入sever节
    serverUrl += "http://";
    serverUrl += config.value("serverIp").toString();  // 获取服务器id并拼接到url
    serverUrl += ":";
    serverUrl += config.value("serverPort").toString();  // 读取服务器端口号并拼接到url
    LOG()<<serverUrl;
}

const QString &DataCenter::getServerUrl() const
{
    return serverUrl;
}

const KindAndTag *DataCenter::getKindAndTagClassPtr()
{
    if(nullptr == kindAndTag){
        kindAndTag = new KindAndTag();
    }

    return kindAndTag;
}

void DataCenter::setSessionId(const QString &sessionId)
{
    loginSessionId = sessionId;
}

const QString &DataCenter::getLoginSessionId() const
{
    return loginSessionId;
}

VideoList *DataCenter::getVideoListPtr()
{
    if(nullptr == videoListPtr){
        videoListPtr = new VideoList();
    }
    return videoListPtr;
}

void DataCenter::setVideoList(const QJsonObject &videoListJsonObj)
{
    getVideoListPtr();

    // 解析出单个视频信息，并保存到视频列表中
    QJsonArray videoListArray = videoListJsonObj["videoList"].toArray();
    for(int i = 0; i < videoListArray.size(); ++i){
        QJsonObject videoInfoObj = videoListArray[i].toObject();
        VideoInfo videoInfo;
        videoInfo.laodVideoInfo(videoInfoObj);
        videoListPtr->addVideo(videoInfo);
    }

    videoListPtr->setVideoTotalCount(videoListJsonObj["totalCount"].toInteger());
    if(0 == videoListArray.size()){
        videoListPtr->setPageIndex(videoListPtr->getPageIndex()-1);
    }
}

void DataCenter::setBarrageData(const QJsonArray &barrageArray)
{
    // 清空之前视频中的弹幕数据
    barrages.clear();

    // 重新添加当前视频的弹幕数据
    // 解析出单个弹幕数据
    QList<BarrageInfo> timeBarrages;
    for(int i = 0; i < barrageArray.size(); ++i){
        QJsonObject barrageObj = barrageArray[i].toObject();
        BarrageInfo barrageInfo;
        barrageInfo.loadBarrageInfo(barrageObj);

        barrages[barrageInfo.playTime].append(barrageInfo);

        // 注意：timeBarrages内部放置的是相同时间点的所有弹幕数据
        if(!timeBarrages.isEmpty() && timeBarrages[0].playTime != barrageInfo.playTime){
            // 该条弹幕时当前时间下的第一条弹幕信息
            // 此时应该将上个时间点的弹幕保存起来，然后处理新的弹幕数据
            barrages.insert(timeBarrages[0].playTime, timeBarrages);
            timeBarrages.clear();
        }

        timeBarrages.append(barrageInfo);
    }

    // 注意，最后一个时间点的弹幕数据还没有保存
    if(!timeBarrages.isEmpty()){
        barrages.insert(timeBarrages[0].playTime, timeBarrages);
    }
}

QHash<int64_t, QList<BarrageInfo> > &DataCenter::getBarrageData()
{
    return barrages;
}

void DataCenter::setMyselfInfo(const QJsonObject &myselfJson)
{
    if(nullptr == myselfInfo){
        myselfInfo = new UserInfo();
    }

    myselfInfo->loadUserInfo(myselfJson);
}

UserInfo *DataCenter::getMyselfInfo()
{
    return myselfInfo;
}

void DataCenter::setOtherUserInfo(const QJsonObject &myselfJson)
{
    if(nullptr == otherUserInfo){
        otherUserInfo = new UserInfo();
    }

    otherUserInfo->loadUserInfo(myselfJson);
}

UserInfo *DataCenter::getOtherUserInfo()
{
    return otherUserInfo;
}

void DataCenter::buildTempUserInfo()
{
    if(myselfInfo == nullptr){
        myselfInfo = new UserInfo();
    }

    // 临时用户没有个人信息，服务器权限受限，无法从服务器获取到临时用户个人信息
    myselfInfo->buildTempUserInfo();
}

void DataCenter::clearUserInfo()
{
    if(myselfInfo){
        delete myselfInfo;
        myselfInfo = nullptr;
    }
}

void DataCenter::setAvatar(const QString &fileId)
{
    myselfInfo->avatarFileId = fileId;
}

void DataCenter::setUserVideoList(const QJsonObject &videoListJson)
{
    getUserVideoList();

    // 解析视频信息
    QJsonArray videoListArray = videoListJson["videoList"].toArray();
    for(int i = 0; i < videoListArray.size(); ++i){
        // 解析出单个视频的信息
        QJsonObject videoInfoJson = videoListArray[i].toObject();
        VideoInfo videoInfo;
        videoInfo.laodVideoInfo(videoInfoJson);

        // 视频信息添加到用户视频列表中
        userVideoList->videoInofs.push_back(videoInfo);
    }

    // 设置视频总个数
    int videoTotalCount = videoListJson["totalCount"].toInt();
    LOG()<<"视频总个数："<<videoTotalCount;
    userVideoList->setVideoTotalCount(videoTotalCount);

    // 如果本次未获取到新视频，说名后续已经没有视频了，页面索引不需要+1
    if(0 == videoListArray.size()){
        userVideoList->setPageIndex(userVideoList->getPageIndex()-1);
    }
}

VideoList *DataCenter::getUserVideoList()
{
    if(nullptr == userVideoList){
        userVideoList = new VideoList();
    }

    return userVideoList;
}

void DataCenter::setStatusVideoList(const QJsonObject &videoListJson)
{
    getUserVideoList();

    // 解析视频信息
    QJsonArray videoListArray = videoListJson["videoList"].toArray();
    for(int i = 0; i < videoListArray.size(); ++i){
        // 解析出单个视频的信息
        QJsonObject videoInfoJson = videoListArray[i].toObject();
        VideoInfo videoInfo;
        videoInfo.laodVideoInfo(videoInfoJson);

        // 视频信息添加到用户视频列表中
        statusVideoList->videoInofs.push_back(videoInfo);
    }

    // 设置视频总个数
    int videoTotalCount = videoListJson["totalCount"].toInt();
    LOG()<<"视频总个数："<<videoTotalCount;
    statusVideoList->setVideoTotalCount(videoTotalCount);
}

VideoList *DataCenter::getStatusVideoList()
{
    if(nullptr == statusVideoList){
        statusVideoList = new VideoList();
    }

    return statusVideoList;
}

void DataCenter::setAdminList(const QJsonObject &resultJson, bool isAdminStatus)
{
    getAdminList();
    if(isAdminStatus){
        // 通过管理员状态获取的管理员信息---一次性可以获取多个
        int64_t totalAdminCount = resultJson["totalCount"].toInteger();
        adminList->totalCount = totalAdminCount;

        // 解析出各个管理员的信息
        QJsonArray adminInfoArray = resultJson["userList"].toArray();
        for(int i = 0; i < adminInfoArray.size(); ++i){
            QJsonObject adminJson = adminInfoArray[i].toObject();
            AdminInfo adminInfo;
            adminInfo.loadAdminInfo(adminJson);
            adminList->addAdminInfo(adminInfo);
        }
    }else{
        // 通过管理员邮箱获取的管理员信息--一次性只能获取1个
        AdminInfo adminInfo;
        adminInfo.loadAdminInfo(resultJson["userInfo"].toObject());
        adminList->addAdminInfo(adminInfo);
        adminList->totalCount = 1;
    }
}

AdminList *DataCenter::getAdminList()
{
    if(nullptr == adminList){
        adminList = new AdminList();
    }
    return adminList;
}

void DataCenter::helloAsync()
{
    httpClient.hello();
}

void DataCenter::pingAsync()
{
    httpClient.ping();
}


void DataCenter::loginTempUserAsync()
{
    httpClient.loginTempUser();
}

void DataCenter::getAllVideoListAsync()
{
    httpClient.getAllVideoList();
}

void DataCenter::getAllVideoListInKindAsync(int kindId)
{
    httpClient.getAllVideoListInKind(kindId);
}

void DataCenter::getAllVideoListInTagAsync(int tagId)
{
    httpClient.getAllVideoListInTag(tagId);
}

void DataCenter::getAllVideoListSearchTextAsync(const QString &searchText)
{
    httpClient.getAllVideoListSearchText(searchText);
}

void DataCenter::downloadPhotoAsync(const QString &photoFileId)
{
    httpClient.downloadPhoto(photoFileId);
}

void DataCenter::uploadPhotoAsync(const QByteArray &photoData, QWidget* whichPage)
{
    httpClient.uploadPhoto(photoData, whichPage);
}

void DataCenter::downloadVideoAsync(const QString &videoFileId)
{
    httpClient.downloadVideo(videoFileId);
}

void DataCenter::uploadVideoAsync(const QString &videoPath)
{
    httpClient.uploadVideoAsync(videoPath);
}

void DataCenter::getVideoBarrageAsync(const QString &videoId)
{
    httpClient.getVideoBarrage(videoId);
}

void DataCenter::setPlayNumberAsync(const QString &videoId)
{
    httpClient.setPlayNumber(videoId);
}

void DataCenter::getIsLikeVdieoAsync(const QString &videoId)
{
    httpClient.getIsLikeVdieo(videoId);
}

void DataCenter::setLikeNumberAsync(const QString &videoId)
{
    httpClient.setLikeNumber(videoId);
}

void DataCenter::loadupBarrageAsync(const QString &videoId, const BarrageInfo &barrageInfo)
{
    httpClient.loadupBarrage(videoId, barrageInfo);
}

void DataCenter::getMyselfInfoAsync()
{
    httpClient.getUserInfo("");
}

void DataCenter::getOtherUserInfoAsync(const QString &userId)
{
    httpClient.getUserInfo(userId);
}

void DataCenter::setAvatarAsync(const QString &fileId)
{
    httpClient.setAvatar(fileId);
}

void DataCenter::getUserVideoListAsync(const QString &userId, int pageIndex, VideoStatus videoStatus, const QString& whichPage)
{
    httpClient.getUserVideoList(userId, pageIndex, videoStatus, whichPage);
}

void DataCenter::deleteVideoAsync(const QString &videoId)
{
    httpClient.deleteVideo(videoId);
}

void DataCenter::newAttentionAsync(const QString &userId)
{
    httpClient.newAttention(userId);
}

void DataCenter::delAttentionAsync(const QString &userId)
{
    httpClient.delAttention(userId);
}

void DataCenter::getAuthcodeAsync(const QString &email)
{
    httpClient.getAuthcode(email);
}

void DataCenter::loginWithEmailAsync(const QString &email, const QString &authcode, const QString &authcodeId)
{
    httpClient.loginWithEmail(email, authcode, authcodeId);
}

void DataCenter::loginWithPasswordAsync(const QString &username, const QString &password)
{
    httpClient.loginWithPassword(username, password);
}

void DataCenter::loginSessionAsync()
{
    httpClient.loginSession();
}

void DataCenter::logoutAsync()
{
    httpClient.logout();
}

void DataCenter::setPasswordAsync(const QString &password)
{
    httpClient.setPassword(password);
}

void DataCenter::setNicknameAsync(const QString &nickname)
{
    httpClient.setNickname(nickname);
}

void DataCenter::uploadVideoDescAsync(const VideoDesc &videoDesc)
{
    httpClient.uploadVideoDesc(videoDesc);
}

void DataCenter::getStatusVideoListAsync(model::VideoStatus videoStatus, int pageIndex)
{
    httpClient.getStatusVideoList(videoStatus, pageIndex);
}

void DataCenter::checkVideoAsync(const QString &videoId, bool result)
{
    httpClient.checkVideo(videoId, result);
}

void DataCenter::putawayVideoAsync(const QString &videoId)
{
    httpClient.putawayVideo(videoId);
}

void DataCenter::discardVideoAsync(const QString &videoId)
{
    httpClient.discardVideo(videoId);
}

void DataCenter::getAdminInfoByEmailAsync(const QString &email)
{
    httpClient.getAdminInfoByEmail(email);
}

void DataCenter::getAdminInfoByStatusAsync(int pageIndex, AdminStatus adminStatus)
{
    httpClient.getAdminInfoByStatus(pageIndex, adminStatus);
}

void DataCenter::newAdminAsync(const AdminInfo &adminInfo)
{
    httpClient.newAdmin(adminInfo);
}

void DataCenter::editAdminAsync(const AdminInfo &adminInfo)
{
    httpClient.editAdmin(adminInfo);
}

void DataCenter::setAdminStatusAsync(const AdminInfo &adminInfo)
{
    httpClient.setAdminStatus(adminInfo);
}

void DataCenter::delAdminAsync(const QString &userId)
{
    httpClient.delAdmin(userId);
}

} // end model

