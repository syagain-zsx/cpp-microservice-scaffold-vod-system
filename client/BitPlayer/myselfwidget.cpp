#include "myselfwidget.h"
#include "ui_myselfwidget.h"
#include "videobox.h"
#include "util.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QScrollBar>
#include "modifymyselfdialog.h"
#include "bitplayer.h"
#include "./model/data.h"
#include "./model/datacenter.h"
#include "toast.h"
#include "confirmdialog.h"

////////////////////////////////// AttentionButton类 //////////////////////////////////
AttentionButton::AttentionButton(QWidget *parent)
    : QPushButton(parent)
{
    changeStatus(false);
}

bool AttentionButton::isAttention() const
{
    return isAttentionStatus;
}

void AttentionButton::changeStatus(bool isAttentioned)
{
    // 根据被关注的状态，调整按钮的样式
    this->isAttentionStatus = isAttentioned;
    if(isAttentioned){
        this->setText("已关注");
        this->setStyleSheet("QPushButton{"
                            "background-color : transparent;"
                            "color : #3ECEFE;"
                            "border-radius : 4px;"
                            "border : 1px solid #3ECEFE;"
                            "padding-left : 13px;"
                            "padding-right : 14px;"
                            "}");
        // 设置按钮上的图标
        this->setIconSize(QSize(24, 24));
        this->setIcon(QIcon(":/images/myself/guanzhu.png"));
    }else{
        this->setText("关注");
        this->setStyleSheet("QPushButton{"
                            "background-color : #3ECEFE;"
                            "color : #FFFFFF;"
                            "border-radius : 4px;"
                            "border : none;"
                            "}");
        // 取消图标
        this->setIcon(QIcon());
    }
}

////////////////////////////////// MyselfWidget类 //////////////////////////////////
MyselfWidget::MyselfWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyselfWidget)
{
    ui->setupUi(this);

    initUI();
    connectSignalsAndSlots();
}

MyselfWidget::~MyselfWidget()
{
    delete login;
    delete ui;
}

void MyselfWidget::loadMyself()
{
    // 获取自己的个人信息
    getMyselfInfo();

    // 加载个人视频
    userId = "";
    getUserVideoList("", 1);

    // 切换个人模式，即允许用户修改头像
    ui->avatarBtn->changeMode(true);
    ui->avatarBtn->setClickedStatus(true);
}

void MyselfWidget::getMyselfInfo()
{
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfoPtr = dataCenter->getMyselfInfo();
    if(nullptr == myselfInfoPtr){
        // 需要先获取当前用户个人信息
        dataCenter->getMyselfInfoAsync();
    }else{
        // 用户信息之前已经获取了，需要更新界面数据
        getMyselfInfoDone();
    }
}

void MyselfWidget::getMyselfInfoDone()
{
    // 1. 获取用户数据
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    // 更具当前用户的角色类型 确定界面需要显示的控件元元素
    if(myselfInfo->isTempUser()){
        // 当前用户是临时用户
        hideWidget(true);
        ui->avatarBtn->setIcon(QIcon(":/images/myself/defaultAvatar.png"));
        // 注意临时用户不允许修改头像
        ui->avatarBtn->setClickedStatus(false);
        ui->nicknameBtn->setText("点击登录");
        // 修改用户昵称之后，如果用户昵称比较短，想要让按钮宽度跟随文本的宽度自动进行调整
        ui->nicknameBtn->adjustSize();
        ui->nicknameBtn->setClickedStatus(true);
        return;
    }else if(myselfInfo->isBUser()){
        // 该用户为B端用户：管理员 或者 超级管理员，需要显示系统页面切换按钮
        // bitPlayer
        BitPlayer* bitPlayer = BitPlayer::getInstance();
        bitPlayer->showSystemPageBtn(true);
    }

    // 普通用户 或者 管理员
    hideWidget(false);

    // 2. 更新界面数据
    // 设置用户昵称，根据昵称宽度调整按钮的宽度
    ui->nicknameBtn->setText(myselfInfo->nickname);
    ui->nicknameBtn->adjustSize();
    ui->nicknameBtn->setClickedStatus(false);
    // 根据昵称按钮文本长度移动设置按钮，让设置按钮紧跟在用户昵称按钮之后
    QRect rect = ui->nicknameBtn->geometry();
    ui->settingBtn->move(rect.x() + rect.width() + 8, ui->settingBtn->geometry().y());

    // 设置关注数、粉丝数、点赞数、播放数
    ui->attentionCountLabel->setText(intToString2(myselfInfo->followedCount));
    ui->fansCountLabel->setText(intToString2(myselfInfo->followerCount));
    ui->likeCountLabel->setText(intToString2(myselfInfo->likeCount));
    ui->playCountLabel->setText(intToString2(myselfInfo->playCount));

    // 3. 设置用户头像
    if(myselfInfo->avatarFileId.isEmpty()){
        ui->avatarBtn->setIcon(QIcon(":/images/myself/defaultAvatar.png"));
        // 保存一份默认用户头像
        myselfInfo->userAvatarData = loadFileToByteArray(":/images/myself/defaultAvatar.png");
    }else{
        dataCenter->downloadPhotoAsync(myselfInfo->avatarFileId);
    }

    // 4. 其他设置：隐藏关注按钮，允许修改头像按钮点击，修改我的视频
    ui->attentionBtn->hide();
    ui->avatarBtn->setClickedStatus(true);
    ui->myVideoLabel->setText("我的视频");
}

void MyselfWidget::getOtherUserInfo(const QString &userId)
{
    // 给服务器发送获取其他用户信息的请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->getOtherUserInfoAsync(userId);
}

void MyselfWidget::getOtherUserInfoDone()
{
    // 1. 界面控件显示隐藏处理
    hideWidget(false);
    // 隐藏 设置用户信息按钮、退出登录按钮、上传视频按钮
    ui->settingBtn->hide();
    ui->quitBtn->hide();
    ui->uploadVideoBtn->hide();
    //ui->avatarBtn->setEnabled(true);
    ui->avatarBtn->setClickedStatus(false);
    // 使用 QSignalBlocker 禁用信号
    ui->nicknameBtn->setClickedStatus(false);

    // 2. 获取其他用户的个人信息
    auto dataCenter = model::DataCenter::getInstance();
    auto otherUserInfo = dataCenter->getOtherUserInfo();

    // 3. 更新界面数据
    // 设置昵称、关注数、粉丝数、点赞数、播放数
    ui->nicknameBtn->setText(otherUserInfo->nickname);
    ui->attentionCountLabel->setText(intToString2(otherUserInfo->followedCount));
    ui->fansCountLabel->setText(intToString2(otherUserInfo->followerCount));
    ui->likeCountLabel->setText(intToString2(otherUserInfo->likeCount));
    ui->playCountLabel->setText(intToString2(otherUserInfo->playCount));
    // 设置关注按钮状态--注意：如果在播放页面点击的用户头像刚好时自己时，跳转过来后则不显示关注按钮
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(otherUserInfo->userId == myselfInfo->userId){
        ui->attentionBtn->hide();
    }else{
        ui->attentionBtn->show();
        ui->attentionBtn->changeStatus(otherUserInfo->isFollowing);
    }

    ui->myVideoLabel->setText("TA的视频");

    // 4. 更新用户头像
    if(otherUserInfo->avatarFileId.isEmpty()){
        ui->avatarBtn->setIcon(QIcon(":/images/myself/defaultAvatar.png"));
    }else{
        dataCenter->downloadPhotoAsync(otherUserInfo->avatarFileId);
    }
}

void MyselfWidget::loadOtherUser(const QString &otherUserId)
{
    // 加载其他用户的个人信息
    getOtherUserInfo(otherUserId);

    // 加载其他用户的视频列表
    userId = otherUserId;
    getUserVideoList(otherUserId, 1);

    // 切换到其他用户模式
    ui->avatarBtn->changeMode(false);
    ui->avatarBtn->setClickedStatus(false);
}

void MyselfWidget::getAvatarDone(const QString &fileId, const QByteArray avatarData)
{
    // 获取当前用户头像
    auto myself = model::DataCenter::getInstance()->getMyselfInfo();
    if(myself != nullptr && fileId == myself->avatarFileId){
        ui->avatarBtn->setIcon(makeCircleIcon(avatarData, ui->avatarBtn->width()/2));
        // 保存用户头像数据，加载弹幕时需要用到
        myself->userAvatarData = avatarData;
    }

    // 设置其他用户头像
    auto otherUserInfo = model::DataCenter::getInstance()->getOtherUserInfo();
    if(otherUserInfo != nullptr && otherUserInfo->avatarFileId == fileId){
        ui->avatarBtn->setIcon(makeCircleIcon(avatarData, ui->avatarBtn->width()/2));
    }

    // 注意：以上两个设置是互斥，要么点击 我的 进入个人信息页面，此时设置的就是 当前登录用户的头像
    // 要么在播放页面中点击上传视频用户的头像 进入我的页面 此时展示的是其他用户的信息，设置的也是其他用户的头像
}

void MyselfWidget::uploadAvatarDone1(const QString &fileId, QWidget* whichPage)
{
    if(whichPage != ui->avatarBtn){
        return;
    }

    // 上传图片已经成功，此时需要用fileId修改服务器上用户信息中的头像id
    // 发送修改用户头像id请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->setAvatarAsync(fileId);
}

void MyselfWidget::uploadAvatarDone2()
{
    // 重新通过用户头像 fileId 获取头像，头像获取成功之后重新设置
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    dataCenter->downloadPhotoAsync(myselfInfo->avatarFileId);
}

void MyselfWidget::initUI()
{
    ui->attentionBtn->hide();
    login = new Login();
    ui->layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

#ifndef TEST_UI
    // 往视频显示区域添加VideoBox-测试
    int resourceId = 10000;
    for(int i = 0; i < 20; ++i){
        model::VideoInfo videoInfo;
        resourceId++;
        videoInfo.userAvatarId = "";
        videoInfo.photoFileId = QString::number(resourceId++);
        videoInfo.videoFileId = QString::number(resourceId++);
        videoInfo.nickName = "用户昵称";
        videoInfo.likeCount = 1234;
        videoInfo.playCount = 23456;
        videoInfo.videoTitle = "【北京旅游攻略】一条视频告诉你去了北京该怎么玩";
        videoInfo.videoDuration = 10;
        videoInfo.videoUpTime = "9-16 12:28:58";
        VideoBox* videoBox =  new VideoBox(videoInfo);
        ui->layout->addWidget(videoBox, i/4, i%4);
    }
#endif
}

void MyselfWidget::connectSignalsAndSlots()
{
    // 修改头像按钮点击信号槽绑定
    connect(ui->avatarBtn, &QPushButton::clicked, this, &MyselfWidget::onAvatarBtnClicked);
    // 修改个人信息按钮点击信号槽绑定
    connect(ui->settingBtn, &QPushButton::clicked, this, &MyselfWidget::onSettingBtnClicked);
    // 上传视频按钮点击信号槽绑定
    connect(ui->uploadVideoBtn, &QPushButton::clicked, this, &MyselfWidget::onUploadVideoBtnClicked);
    // QScrollArea垂直滚动条滑动
    connect(ui->scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &MyselfWidget::onScrollAreaValueChanged);
    // 关注按钮信号藏绑定
    connect(ui->attentionBtn, &QPushButton::clicked, this, &MyselfWidget::onAttentionBtnClicked);
    // 昵称按钮点击信号槽绑定
    connect(ui->nicknameBtn, &QPushButton::clicked, this, &MyselfWidget::onNicknameBtnClicked);
    // 退出登录按钮信号槽绑定
    connect(ui->quitBtn, &QPushButton::clicked, this, &MyselfWidget::onQuitBtnClicked);
    // 获取个人信息成功信号槽绑定
    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getMyselfInfoDone, this, &MyselfWidget::getMyselfInfoDone);
    // 获取其他用户信息成功
    connect(dataCenter, &model::DataCenter::getOtherUserInfoDone, this, &MyselfWidget::getOtherUserInfoDone);
    // 获取用户头像成功
    connect(dataCenter, &model::DataCenter::downloadPhotoDone, this, &MyselfWidget::getAvatarDone);
    // 上传用户图像
    connect(dataCenter, &model::DataCenter::uploadPhotoDone, this, &MyselfWidget::uploadAvatarDone1);
    // 修改用户头像id成功
    connect(dataCenter, &model::DataCenter::setAvatarDone, this, &MyselfWidget::uploadAvatarDone2);
    // 获取用户视频列表成功
    connect(dataCenter, &model::DataCenter::getUserVideoListDone, this, &MyselfWidget::getUserListDone);
    // 删除视频成功
    connect(dataCenter, &model::DataCenter::deleteVideoDone, this, &MyselfWidget::deleteVideoDone);
    // 关注用户
    connect(dataCenter, &model::DataCenter::newAttentionDone, this, &MyselfWidget::newAttentionDone);
    // 取消关注用户
    connect(dataCenter, &model::DataCenter::delAttentionDone, this, &MyselfWidget::delAttentionDone);
    // 邮箱登录成功
    connect(login, &Login::loginSuccess, this, [=]{
        //dataCenter->getMyselfInfoAsync();
        // 清空datacenter中保存的临时用户信息，从新加载当前用户的个人信息
        dataCenter->clearUserInfo();
        loadMyself();
    });
    // 退出登录成功
    connect(dataCenter, &model::DataCenter::logoutDone, this, &MyselfWidget::logoutDone);
    // 修改密码成功
    connect(dataCenter, &model::DataCenter::setPasswordDone, this, [=]{
        isModifyPassword = true;
    });
    // 修改昵称成功
    connect(dataCenter, &model::DataCenter::setNicknameDone, this, &MyselfWidget::setNicknameDone);
}

void MyselfWidget::hideWidget(bool isHide)
{
    // 临时用户需要隐藏界面上的控件，非临时用户则显示
    if(isHide){
        ui->attentionBtn->hide();
        ui->attentionCountLabel->hide();
        ui->attentionLabel->hide();
        ui->fansCountLabel->hide();
        ui->fansLabel->hide();
        ui->likeCountLabel->hide();
        ui->likeLabel->hide();
        ui->playCountLabel->hide();
        ui->playLabel->hide();
        ui->settingBtn->hide();
        ui->quitBtn->hide();
        ui->uploadVideoBtn->hide();
        ui->scrollArea->hide();
        ui->titleBar->hide();
    }else{
        ui->attentionBtn->show();
        ui->attentionCountLabel->show();
        ui->attentionLabel->show();
        ui->fansCountLabel->show();
        ui->fansLabel->show();
        ui->likeCountLabel->show();
        ui->likeLabel->show();
        ui->playCountLabel->show();
        ui->playLabel->show();
        ui->settingBtn->show();
        ui->quitBtn->show();
        ui->uploadVideoBtn->show();
        ui->scrollArea->show();
        ui->titleBar->show();
    }
}

void MyselfWidget::getUserVideoList(const QString &userId, int pageIndex)
{
    // 如果获取的是第一页的视频，将界面中的视频信息 以及 DataCenter中保存的视频信息全部清空
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo && myselfInfo->isTempUser()){
        LOG()<<"临时用户，无需获取用户视频列表";
        return;
    }

    auto userVideoList = dataCenter->getUserVideoList();
    if(1 == pageIndex){
        // 清空DataCenter中的用户视频数据
        userVideoList->clearVideoList();
        // 清空界面中的视频数据
        clearVideoList();
    }

    dataCenter->getUserVideoListAsync(userId, pageIndex, model::putaway, "myPage");
    // pageIndex+1，在获取下一页视频的时候就可以直接使用VideoList中的pageIndex
    userVideoList->setPageIndex(pageIndex+1);
}

void MyselfWidget::getUserListDone(const QString &userId, const QString& whichPage)
{
    if("myPage" != whichPage){
        return;
    }
    auto dataCenter = model::DataCenter::getInstance();
    auto userVideoList = dataCenter->getUserVideoList();

    const int rowCount = 4;
    for(int i = ui->layout->count(); i < userVideoList->getVideoCount(); ++i){
        int row = i / rowCount;
        int col = i % rowCount;

        VideoBox* videoBox = new VideoBox(userVideoList->videoInofs[i]);
        ui->layout->addWidget(videoBox, row, col);
        // 如果是当前登录用户获取自己的视频列表，删除更多按钮应该显示出来
        if(userId.isEmpty()){
            videoBox->showMoreBtn(true);
            // 绑定删除信号
            connect(videoBox, &VideoBox::deleteVideo, this, &MyselfWidget::deleteVideo);
        }
    }

    repaint();
}

void MyselfWidget::onScrollAreaValueChanged(int value)
{
    if(0 == value){
        return;
    }

    // 如果垂直滚动条滚动到底部的时候，需要获取下一页视频
    if(value == ui->scrollArea->verticalScrollBar()->maximum()){
        auto dataCenter = model::DataCenter::getInstance();
        auto userVideList = dataCenter->getUserVideoList();
        dataCenter->getUserVideoListAsync(userId, userVideList->getPageIndex(), model::putaway, "myPage");
        userVideList->setPageIndex(userVideList->getPageIndex()+1);
    }
}

void MyselfWidget::deleteVideo(const QString &videoId)
{
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->deleteVideoAsync(videoId);
}

void MyselfWidget::deleteVideoDone(const QString &videoId)
{
    LOG()<<"删除视频成功，videoId = "<<videoId;
    getUserVideoList("", 1);
}

void MyselfWidget::onAttentionBtnClicked()
{
    // 注意：如果当前用户是临时用户，临时用户不能去关注其他用户
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        Toast::showMessage("请先登陆或注册", login);
        return;
    }

    bool isAttentioned = ui->attentionBtn->isAttention();
    isAttentioned = !isAttentioned;
    ui->attentionBtn->changeStatus(isAttentioned);

    // 通知服务器修该用户是否被关注信息
    // todo
    auto otherUserInfo = dataCenter->getOtherUserInfo();
    if(otherUserInfo ==nullptr){
        Toast::showMessage("被关注的用户不存在，无法关注...");
        return;
    }

    if(isAttentioned){
        dataCenter->newAttentionAsync(otherUserInfo->userId);
    }else{
        dataCenter->delAttentionAsync(otherUserInfo->userId);
    }
}

void MyselfWidget::newAttentionDone()
{
    auto dataCenter = model::DataCenter::getInstance();
    auto otherUserInfo = dataCenter->getOtherUserInfo();

    // 更新界面UI
    int64_t followerCount = otherUserInfo->followerCount + 1;
    ui->fansCountLabel->setText(intToString2(followerCount));
    // 更新DataCenter
    otherUserInfo->followerCount = followerCount;

    // 增加当前用户的关注数
    auto myselfInfo = dataCenter->getMyselfInfo();
    myselfInfo->followedCount++;
}

void MyselfWidget::delAttentionDone()
{
    auto dataCenter = model::DataCenter::getInstance();
    auto otherUserInfo = dataCenter->getOtherUserInfo();

    // 更新界面UI
    int64_t followerCount = otherUserInfo->followerCount - 1;
    ui->fansCountLabel->setText(intToString2(followerCount));
    // 更新DataCenter
    otherUserInfo->followerCount = followerCount;

    // 增加当前用户的关注数
    auto myselfInfo = dataCenter->getMyselfInfo();
    myselfInfo->followedCount--;
}

void MyselfWidget::clearVideoList()
{
    QLayoutItem* videoBoxWidget = nullptr;
    while(nullptr != (videoBoxWidget = ui->layout->takeAt(0))){
        delete videoBoxWidget->widget();
        delete videoBoxWidget;
    }
}

void MyselfWidget::onAvatarBtnClicked()
{
    // 0. 注意：临时用户不能通过点击按钮修改用户头像
    auto dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getMyselfInfo()->isTempUser()){
        Toast::showMessage("请先登陆，登录成功之后才能修改头像");
        return;
    }

    // 1. 弹出打开对话框，从本地磁盘选择用户头像--QFileDialog::getOpenFileName
    QString fileName = QFileDialog::getOpenFileName(nullptr, "修改头像", "", tr("Images (*.png *.jpg)"));
    if(fileName.isEmpty()){
        LOG()<<"取消选择头像文件";
        return;
    }

    // 图片大小不能超过5M
    QFileInfo fileInfo(fileName);
    if(fileInfo.size() >= 1024*1024*5){
        Toast::showMessage("头像大小不能超过5M");
        return;
    }

    // 2. 获取图片数据
    QByteArray fileData = loadFileToByteArray(fileName);
    if(fileData.isEmpty()){
        LOG()<<"读取文件失败!";
        return;
    }

    // 3. 将图片数据设置成按钮图标
    //ui->avatarBtn->setIcon(makeCircleIcon(fileData, ui->avatarBtn->width()/2));

    // 4. 将图片数据上传到服务器
    dataCenter->uploadPhotoAsync(fileData, ui->avatarBtn);
}

void MyselfWidget::onSettingBtnClicked()
{
    ModifyMyselfDialog* dlg = new ModifyMyselfDialog(isModifyPassword);
    dlg->exec();
    delete dlg;
}

void MyselfWidget::onUploadVideoBtnClicked()
{
    // 弹出打开文件对话框，让用户选择要上传的视频文件
    QString fileName = QFileDialog::getOpenFileName(nullptr, "上传视频", "", tr("Videos (*.mp4 *.rmvb *.avi *.mov)"));
    if(fileName.isEmpty()){
        LOG()<<"取消上传文件";
        return;
    }

    LOG()<<fileName;

    // 检测视频大小，上传视频大小不能超过4G
    QFileInfo fileInfo(fileName);
    int64_t fileSize = fileInfo.size();
    if(fileSize > (int64_t)1024*1024*1024*4){
        LOG()<<"视频大小必须小于4G";
        return;
    }

    // 发送上传视频请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->uploadVideoAsync(fileName);

    // 切换到上传视频页面
    emit switchUploadVideoPage(UploadPage, fileName);
}

void MyselfWidget::onNicknameBtnClicked()
{
    // 如果当前用户是临时用户，显示登录窗口
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        // 登录之前先清除之前界面上流的旧数据
        login->reset();
        login->show();
    }
}

void MyselfWidget::onQuitBtnClicked()
{
    // 显示确认或取消对话框
    ConfirmDialog confirmDlg;
    confirmDlg.setText("确认退出登录吗？");
    confirmDlg.exec();

    // 如果用户选择确定则退出
    if(confirmDlg.getConfirm()){
        // 给服务器发送退出登录请求
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->logoutAsync();
        // 如果用户不退出播放系统，重新登录其他账号时，则显示修改密码按钮
        isModifyPassword = false;
    }
}

void MyselfWidget::logoutDone()
{
    // 清空界面上的数据 以及 DataCenter中的数据
    clearVideoList();
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->getVideoListPtr()->clearVideoList();

    // 将当前用户设置为临时用户
    dataCenter->buildTempUserInfo();
    loadMyself();

    // 注意：主界面上的系统页面切换按钮隐藏
    BitPlayer::getInstance()->showSystemPageBtn(false);
}

void MyselfWidget::setNicknameDone(const QString &nickname)
{
    // 修改我的页面中的昵称
    ui->nicknameBtn->setText(nickname);
    ui->nicknameBtn->adjustSize();
    QRect rect = ui->nicknameBtn->geometry();
    ui->settingBtn->move(rect.x() + rect.width() + 8, rect.y());

    // 修改DataCenter中保存的个人信息中的昵称
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    myselfInfo->nickname = nickname;

    // 我的页面中视频列表中昵称需要修改
    QList<VideoBox*> videoList = ui->container->findChildren<VideoBox*>();
    for(auto& videoBox : videoList){
        videoBox->setNicknameOfVideoUser(nickname);
    }
    // DataCenter中视频列表中昵称也需要修改
    auto userVideoList = dataCenter->getVideoListPtr()->getVideoList();
    for(auto& videoList : userVideoList){
        if(videoList.userId == myselfInfo->userId){
            videoList.nickName = nickname;
        }
    }
}
