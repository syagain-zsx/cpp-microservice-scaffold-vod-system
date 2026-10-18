#include "playerpage.h"
#include "ui_playerpage.h"
#include <QMouseEvent>
#include "toast.h"
#include "util.h"
#include <QShortcut>
#include <QTimer>
#include <QToolTip>
#include "toast.h"
#include "./model/datacenter.h"
#include "bitplayer.h"

PlayerPage::PlayerPage(const model::VideoInfo& videoInfo, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlayerPage)
    ,videoInfo(videoInfo)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowModal);

    volume = new Volume(this);
    playSpeed = new PlaySpeed(this);

    mpvPlayer = new MpvPlayer(ui->screen);
    login = new Login();

    initBarrageArea();

    updateVideoInfoUI();

    // 播放按钮绑定空格快捷键
    QShortcut* shortCut = new QShortcut(ui->playBtn);
    QKeySequence keySequence(" ");
    shortCut->setKey(keySequence);
    connect(shortCut, &QShortcut::activated, this, [=](){
        ui->playBtn->animateClick();
    });

    connect(ui->minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->quitBtn, &QPushButton::clicked, this, &PlayerPage::onQuitBtnClicked);
    connect(ui->volumeBtn, &QPushButton::clicked, this, &PlayerPage::onVolumeBtnClicked);
    connect(ui->speedBtn, &QPushButton::clicked, this, &PlayerPage::onSpeedBtnClicked);
    connect(ui->likeImageBtn, &QPushButton::clicked, this, &PlayerPage::onLikeImageBtnClicked);
    connect(ui->playBtn, &QPushButton::clicked, this, &PlayerPage::onPlayBtnClicked);
    connect(ui->userAvatar, &QPushButton::clicked, this, &PlayerPage::onUserAvatarBtnClicked);
    connect(playSpeed, &PlaySpeed::setPlaySpeed, this, &PlayerPage::onPlaySpeedChanged);
    connect(volume, &Volume::setVolume, this, &PlayerPage::setVolume);
    connect(ui->videoSlider, &PlaySlider::setPlayProgress, this, &PlayerPage::onSetPlayProgress);
    connect(mpvPlayer, &MpvPlayer::playPositionChanged, this, &PlayerPage::onPlayPositionChanged);
    // 所有视频分片播放结束信号槽绑定
    connect(mpvPlayer, &MpvPlayer::endOfPlaylist, this, &PlayerPage::onEndOfPlaylist);
    connect(ui->bulletScreenBtn, &QPushButton::clicked, this, &PlayerPage::onBulletScreenBtnClicked);
    connect(ui->bulletScreenText, &BarrageEdit::sendBulletScreen, this, &PlayerPage::onSendBulletScreenBtnClicked);

    likeCount = videoInfo.likeCount;
    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getIsLikeVdieoDone, this, [=](const QString& videoId, bool isLike){
        if(videoId != this->videoInfo.videoId){
            return;
        }

        if(isLike){
            ui->likeImageBtn->setStyleSheet("border-image : url(:/images/PlayPage/dianzan.png)");
        }else{
            ui->likeImageBtn->setStyleSheet("border-image : url(:/images/PlayPage/quxiaodianzan.png)");
        }

        this->isLike = isLike;
    });
    dataCenter->getIsLikeVdieoAsync(videoInfo.videoId);
}

PlayerPage::~PlayerPage()
{
    delete ui;
    delete mpvPlayer;
    delete login;
}

void PlayerPage::mousePressEvent(QMouseEvent *event)
{
    // 获取鼠标按下时相对于窗口的位置,检测位置是否在窗口的head内部
    QPoint point = event->position().toPoint();
    if(ui->playHead->geometry().contains(point)){
        if(event->button() == Qt::LeftButton){
            // 计算鼠标按下之后的全局坐标和窗口左上角的相对偏移
            // 在整个移动过程中，该偏移不能发生改变
            dragPos = event->globalPosition().toPoint() - geometry().topLeft();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void PlayerPage::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point = event->position().toPoint();
    if(ui->playHead->geometry().contains(point)){
        if(event->buttons() == Qt::LeftButton){
            move(event->globalPosition().toPoint() - dragPos);

            // 移动弹幕窗口到播放窗口的head下放
            QPoint point = geometry().topLeft();   // 播放窗口的左上角
            point.setY(point.ry() + ui->playHead->height());
            barrageArea->move(point);
        }
    }

    QWidget::mousePressEvent(event);
}

void PlayerPage::startPlaying()
{
    buildBulletScreenData();

    //mpvPlayer = new MpvPlayer(ui->screen);

    // 构造m3u8文件的url，然后交给mpv进行播放
    auto dataCenter = model::DataCenter::getInstance();
    QString m3u8FileUrl = dataCenter->getServerUrl();
    m3u8FileUrl += "/HttpService/downloadVideo?fileId=";
    m3u8FileUrl += videoInfo.videoFileId;
    mpvPlayer->startPlay(m3u8FileUrl);
    isUpdatePlayNum = false;

    ui->videoSlider->setPlayStep(0);
    // 视频加载成功之后会立马播放，初始时先将其设置为暂停状态，当用户点击播放按钮之后再让视频播放起来
    mpvPlayer->pause();
    mpvPlayer->setVolume(volume->getVolume());
}

void PlayerPage::setUserAvatar(QPixmap &userAvatar)
{
    ui->userAvatar->setIcon(QIcon(userAvatar));
}

void PlayerPage::setPlayPage(const QString &whichPage)
{
    this->whichPage = whichPage;
}

void PlayerPage::onVolumeBtnClicked()
{
    // 获取播放窗口的左上角的坐标，并将其转换为基于屏幕的坐标
    moveWindows(mapToGlobal(QPoint(0, 0)));
    volume->show();
}

void PlayerPage::onSpeedBtnClicked()
{
    moveWindows(mapToGlobal(QPoint(0, 0)));
    playSpeed->show();
}

void PlayerPage::onLikeImageBtnClicked()
{
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        Toast::showMessage("先登录，登录完成之后才能点赞", login);
        return;
    }

    // 如果视频的状态是待审核状态，则说明是管理员在审核视频，审核视频时不能点赞
    //if(myselfInfo->isBUser() && videoInfo.videoStatus == model::VideoStatus::waitForChecking){
    if(whichPage == "systemPage"){
        Toast::showMessage("管理员审核视频时无需对视频点赞");
        return;
    }

    // 更新点赞按钮的图标
    isLike = !isLike;
    if(isLike){
        likeCount++;
        ui->likeImageBtn->setStyleSheet("border-image : url(:/images/PlayPage/dianzan.png)");
    }else{
        likeCount--;
        ui->likeImageBtn->setStyleSheet("border-image : url(:/images/PlayPage/quxiaodianzan.png)");
    }
    // 更新点赞数量
    ui->likeNum->setText(intToString(likeCount));
}

void PlayerPage::onPlayBtnClicked()
{
    isPlay = !isPlay;
    if(isPlay){
        ui->playBtn->setStyleSheet("border-image : url(:/images/PlayPage/bofang.png);");
        mpvPlayer->play();
    }else{
        ui->playBtn->setStyleSheet("border-image : url(:/images/PlayPage/zanting.png);");
        mpvPlayer->pause();
    }

    // 如果本次播放中，视频的播放数未更新时候再去更新
    if(!isUpdatePlayNum){
        updatePlayNumer();
    }
}

void PlayerPage::onPlaySpeedChanged(double speed)
{
    mpvPlayer->setPlaySpeed(speed);
}

void PlayerPage::setVolume(int volume)
{
    mpvPlayer->setVolume(volume);
}

void PlayerPage::onPlayPositionChanged(int64_t playTime)
{
    QString curPlayTime = secondToTime(playTime);
    QString totalTime = secondToTime(videoInfo.videoDuration);
    ui->videoDuration->setText(curPlayTime + "/" + totalTime);

    ui->videoSlider->setPlayStep((double)playTime / videoInfo.videoDuration);

    // 更新弹幕数据
    showBulletScreen();
}

void PlayerPage::onEndOfPlaylist()
{
    // 当整个视频播放结束之后，将播放按钮图标 以及 播放状态设置为暂停
    isPlay = false;
    ui->playBtn->setStyleSheet("border-image : url(:/images/PlayPage/zanting.png);");
    // 重新加载视频
    startPlaying();
}

void PlayerPage::onSetPlayProgress(double playRatio)
{
    // 根据进度条的比率计算当前播放时间
    int64_t playTime = playRatio*videoInfo.videoDuration;

    mpvPlayer->setCurrentPlayPositon(playTime);
}

void PlayerPage::onBulletScreenBtnClicked()
{
    isStartBS = !isStartBS;
    if(isStartBS){
        // 弹幕开
        ui->bulletScreenBtn->setStyleSheet("border-image : url(:/images/PlayPage/danmu.png)");
        barrageArea->show();
    }else{
        // 弹幕关
        ui->bulletScreenBtn->setStyleSheet("border-image : url(:/images/PlayPage/danmuguan.png)");
        barrageArea->hide();
    }
}

void PlayerPage::onSendBulletScreenBtnClicked(const QString &text)
{
    // 如果视频的状态是待审核状态，则说明是管理员在审核视频，审核视频时不能点赞
    if(whichPage == "systemPage"){
        Toast::showMessage("视频审核中禁止发送弹幕!!!");
        return;
    }

    // 如果用户未登录，先要让用户登录，登录成功之后才能显示弹幕
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        Toast::showMessage("先登录，登录完成之后才能点赞", login);
        return;
    }

    // 如果弹幕窗口关闭，则不能发送弹幕
    if(!isStartBS){
        Toast::showMessage("请打开弹幕开关...");
        return;
    }

    // 构造播放页面显示的弹幕元素
    BulletScreenItem* bsItem = new BulletScreenItem(top);
    bsItem->setBulletScreenText(text);
    // 发送弹幕时，使用当前登录用户的头像
    bsItem->setBulletScreenIcon(makeCircleIcon(loginUserAvatar, 13).pixmap(26, 26));
    int64_t duration = 10000 * top->width() / (double)(top->width() + 30*18);
    bsItem->setBulletScreenAnimation(top->width(), duration);
    bsItem->startAnimation();

    // 将弹幕数据发送到服务器
    model::BarrageInfo barrageInfo;
    barrageInfo.playTime = mpvPlayer->getPlayTime();
    barrageInfo.text = text;
    barrageInfo.userId = myselfInfo->userId;

    dataCenter->loadupBarrageAsync(videoInfo.videoId, barrageInfo);

    // 将发送的弹幕在本地缓存一份
    auto& barrageDatas = dataCenter->getBarrageData();
    barrageDatas[barrageInfo.playTime].push_back(barrageInfo);
    // 清空弹幕编辑框
    ui->bulletScreenText->setText("");
}

void PlayerPage::onQuitBtnClicked()
{
    if(likeCount != videoInfo.likeCount){
        // 视频的点赞信息发生改变，给服务器同步
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->setLikeNumberAsync(videoInfo.videoId);

        // 更新视频列表中的videoId视频的点赞信息：首页 和 我的页面
        auto videoListPtr = dataCenter->getVideoListPtr();
        videoListPtr->updateLikeNumber(videoInfo.videoId, likeCount);
        videoInfo.likeCount = likeCount;

        // 更新我的页面中的视频列表
        auto userVideoListPtr = dataCenter->getVideoListPtr();
        if(!userVideoListPtr->videoInofs.isEmpty()){
            videoListPtr->updateLikeNumber(videoInfo.videoId, likeCount);
        }

        // 更新状态页面中的视频列表
        auto statusVideoListPtr = dataCenter->getVideoListPtr();
        if(!statusVideoListPtr->videoInofs.isEmpty()){
            videoListPtr->updateLikeNumber(videoInfo.videoId, likeCount);
        }

        // 通知videoBox更新点赞数据
        emit updateLikeNum(likeCount);
    }

    this->deleteLater();
}

void PlayerPage::onUserAvatarBtnClicked()
{
#if 0
    // 关闭当前窗口
    onQuitBtnClicked();
    // 让BitPlayer切换页面
    BitPlayer::getInstance()->switchToUserInfoPage(videoInfo.userId);
#endif

    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        Toast::showMessage("请先登录或注册!!!");
        QTimer* timer = new QTimer;
        timer->setInterval(2000);
        connect(timer, &QTimer::timeout, this, [=]{
            onQuitBtnClicked();
            BitPlayer::getInstance()->switchToUserInfoPage(videoInfo.userId);
            delete timer;
        });
        timer->start();
    }else{
        // 关闭当前窗口
        onQuitBtnClicked();
        // 让BitPlayer切换页面
        BitPlayer::getInstance()->switchToUserInfoPage(videoInfo.userId);
    }
}

void PlayerPage::moveWindows(const QPoint &point)
{
    // point已经是基于屏幕的全局坐标
    QPoint newPoint = point + QPoint(this->width() - volume->width() - 13, 533);
    volume->move(newPoint);

    newPoint = point + QPoint(this->width() - playSpeed->width() - 82, 536);
    playSpeed->move(newPoint);
}

QString PlayerPage::secondToTime(int64_t seconds)
{
    QString time;
    // 小时：存在时才显示
    if(seconds/60/60){
        time += QString::asprintf("%02lld:", seconds/60/60);
    }

    // 拼接分和秒
    time += QString::asprintf("%02lld:%02lld", seconds/60, seconds%60);
    return time;
}

void PlayerPage::initBarrageArea()
{
    // 创建弹幕的显示区域对话框，该对话框没有边框，背景透明
    barrageArea = new QDialog(this);
    barrageArea->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    barrageArea->setAttribute(Qt::WA_TranslucentBackground);
    barrageArea->setMinimumSize(this->width(), 38*3);

    // 垂直布局器，添加到对话框中
    QVBoxLayout* layout = new QVBoxLayout(barrageArea);
    barrageArea->setLayout(layout);

    // 在弹幕区域添加用来显示三行弹幕的控件
    top = new QFrame(this);
    top->setFixedSize(this->width(), 38);

    middle = new QFrame(this);
    middle->setFixedSize(this->width(), 38);

    bottom = new QFrame(this);
    bottom->setFixedSize(this->width(), 38);

    layout->addWidget(top);
    layout->addWidget(middle);
    layout->addWidget(bottom);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 移动弹幕窗口到播放窗口的head底下
    QPoint point = mapToGlobal(QPoint(0, 0));
    point.setY(point.y() + ui->playHead->height());
    barrageArea->move(point);
    barrageArea->show();
}

void PlayerPage::buildBulletScreenData()
{
    auto dataCenter = model::DataCenter::getInstance();
    barrages = dataCenter->getBarrageData();

    // 获取当前登录用户的用户头像
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo){
        loginUserAvatar = myselfInfo->userAvatarData;
    }
}

void PlayerPage::showBulletScreen()
{
    // 视频进入播放时再开始加载弹幕
    if(!isPlay){
        return;
    }

    // 如果打开关闭时，则不需要添加弹幕到界面
    if(!isStartBS){
        return;
    }

    // 1. 获取当前playTime点的所有弹幕
    QList<model::BarrageInfo> bulletScrrenList = barrages.value(mpvPlayer->getPlayTime());
    BulletScreenItem* bsItem = nullptr;

    // 获取当前登录用户ID
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    QString logUserId = myselfInfo->userId;

    // 2. 将弹幕显示出来
    int xTop, xMiddle, xBottom;
    xTop = xMiddle = xBottom = top->width();
    for(int i = 0; i < bulletScrrenList.size(); ++i){
        model::BarrageInfo& bsInfo = bulletScrrenList[i];
        if(0 == i%3){
            // 弹幕显示在第一行
            bsItem = new BulletScreenItem(top);
            bsItem->setBulletScreenText(bsInfo.text);

            // 给弹幕设置动画属性
            int duration = 10000*xTop / (double)(top->width() + 30*18);
            bsItem->setBulletScreenAnimation(xTop, duration);
            xTop += bsItem->width() + 4*18; // 同一行间隔4个汉字，因为字体大小为18px，因此4*18，就是4个汉字的宽度

        }else if(1 == i%3){
            // 弹幕显示在第二行
            bsItem = new BulletScreenItem(middle);
            bsItem->setBulletScreenText(bsInfo.text);

            // 给弹幕设置动画属性
            int duration = 10000*xMiddle / (double)(middle->width() + 30*18);
            bsItem->setBulletScreenAnimation(xMiddle, duration);
            xMiddle += bsItem->width() + 4*18; // 同一行间隔4个汉字，因为字体大小为18px，因此4*18，就是4个汉字的宽度
        }else{
            // 弹幕显示在第三行
            bsItem = new BulletScreenItem(bottom);
            bsItem->setBulletScreenText(bsInfo.text);

            // 给弹幕设置动画属性
            int duration = 10000*xBottom / (double)(bottom->width() + 30*18);
            bsItem->setBulletScreenAnimation(xBottom + 2*18, duration);
            xBottom += bsItem->width() + 4*18; // 同一行间隔4个汉字，因为字体大小为18px，因此4*18，就是4个汉字的宽度
        }

        // 检测如果是当前用户发送的弹幕，显示弹幕时需要加上用户头像
        if(bsInfo.userId == logUserId){
            bsItem->setBulletScreenIcon(makeCircleIcon(loginUserAvatar, 13).pixmap(26, 26));
        }
        bsItem->startAnimation();
    }
}

void PlayerPage::updateVideoInfoUI()
{
    // 设置视频标题
    ui->videoTittle->setText(videoInfo.videoTitle);

    // 设置用户昵称
    ui->userNikeName->setText(videoInfo.nickName);

    // 设置视频的上传时间
    ui->loadupTime->setText(formatDate(videoInfo.videoUpTime));

    // 设置点赞数
    // 点赞数：小于一万直接显示数字   大于等于1万，比如：12345 1.23万
    ui->likeNum->setText(intToString(videoInfo.likeCount));

    // 设置播放数
    ui->playNum->setText(intToString(videoInfo.playCount));

    // 设置视频当前播放时间和持续时长
    QString curPlayTime = secondToTime(0);
    QString totalTime = secondToTime(videoInfo.videoDuration);
    ui->videoDuration->setText(curPlayTime + "/" + totalTime);

    // 设置视频的描述信息
    if(videoInfo.videoDesc.size() < 200){
        ui->videoDesc->setText(videoInfo.videoDesc);
    }else{
        ui->videoDesc->setText(videoInfo.videoDesc.left(200)+"...");
        // 注意：直接给QLabel设置QToolTip样式时，该样式不会生效
        // 系统的主题会覆盖QT的tooltip样式，尤其是背景颜色，文字颜色一般是继承父控件的，所以需要强制使用Fusion样式
        // Fusion是QT提供的一种跨平台样式，不受系统影响
        ui->videoDesc->setStyleSheet("#videoDesc{font-size : 14px;}"
                                     "QToolTip{background-color: #FFFFFF;color : #000000;}");
        ui->videoDesc->setToolTip(videoInfo.videoDesc);
    }
}

void PlayerPage::updatePlayNumer()
{
    // 如果视频的状态是待审核状态，则说明是管理员在审核视频，审核视频时不能点赞
    if(videoInfo.videoStatus == model::VideoStatus::waitForChecking){
        return;
    }

    // 更新界面上的播放数据
    videoInfo.playCount++;
    ui->playNum->setText(intToString(videoInfo.playCount));

    // 给服务器发送更新视频播放数的请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->setPlayNumberAsync(videoInfo.videoId);

    // 注意：更新dataCenter中视频列表中的播放次数：首页 和 我的页面
    // 首页视频列表中对应videoId的视频播放数
    auto videoListPtr = dataCenter->getVideoListPtr();
    videoListPtr->incrementPlayNum(videoInfo.videoId);
    // 我的视频列表
    auto userVideoListPtr = dataCenter->getVideoListPtr();
    if(!userVideoListPtr->videoInofs.isEmpty()){
        userVideoListPtr->incrementPlayNum(videoInfo.videoId);
    }

    // 状态视频列表
    auto statusVideoListPtr = dataCenter->getVideoListPtr();
    if(!statusVideoListPtr->videoInofs.isEmpty()){
        statusVideoListPtr->incrementPlayNum(videoInfo.videoId);
    }

    isUpdatePlayNum = true;

    emit incrementPlayNumber();
}



