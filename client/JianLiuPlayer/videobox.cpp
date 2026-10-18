#include "videobox.h"
#include "ui_videobox.h"
#include "util.h"
#include <QDir>
#include <QMenu>
#include "./model/datacenter.h"

VideoBox::VideoBox(const model::VideoInfo& videoInfo, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoBox)
    , videoInfo(videoInfo)
{
    ui->setupUi(this);

    ui->delVideoBtn->hide();

    // 给视频封面、视频标题控件安装事件拦截器
    ui->imageBox->installEventFilter(this);
    ui->videoTittle->installEventFilter(this);

    updateVideoInfoUI();
    auto dataCenter = model::DataCenter::getInstance();
    // 下载视频封面图成功
    connect(dataCenter, &model::DataCenter::downloadPhotoDone, this, &VideoBox::getVideoImageDone);
    // 下载用户头像成功
    connect(dataCenter, &model::DataCenter::downloadPhotoDone, this, &VideoBox::getUserAvatarDone);

    // ... 按钮点击
    connect(ui->delVideoBtn, &QPushButton::clicked, this, &VideoBox::onMoreBtnClicked);
}

VideoBox::~VideoBox()
{
    delete ui;
}

bool VideoBox::eventFilter(QObject *watched, QEvent *event)
{
    if(ui->imageBox == watched || ui->videoTittle == watched){
        if(event->type() == QEvent::MouseButtonPress){
            onPlayClicked();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void VideoBox::showMoreBtn(bool isShow)
{
    if(isShow){
        ui->delVideoBtn->show();
    }else{
        ui->delVideoBtn->hide();
    }
}

void VideoBox::setNicknameOfVideoUser(const QString &nickname)
{
    videoInfo.nickName = nickname;
    ui->userNickName->setText(nickname);
}

void VideoBox::updateVideoInfoUI()
{
    // 设置视频标题
    ui->videoTittle->setText(videoInfo.videoTitle);

    // 如果数字小于10000 直接显示数字 如果大于10000，以小数12345 将来1.2万
    // 设置点赞数
    ui->likeNum->setText(intToString(videoInfo.likeCount));

    // 设置播放量
    ui->playNum->setText(intToString(videoInfo.playCount));

    // 设置用户昵称
    ui->userNickName->setText(videoInfo.nickName);

    // 设置视频上传时间
    ui->loadupTime->setText(" · " + formatDate(videoInfo.videoUpTime));

    // 设置视频持续时长
    // 12:45
    // 01:24:23
    setVideoDuration(videoInfo.videoDuration);

    // 设置视频封面
    setVideoImage(videoInfo.photoFileId);

    // 设置用户头像
    setUserAvatar(videoInfo.userAvatarId);
}

void VideoBox::setVideoDuration(int64_t duration)
{
    QString time;
    // 小时如果存在则显示
    if(duration/60/60){
        time += QString::asprintf("%02lld:", duration/60/60);
    }

    // 拼接上分和秒
    time += QString::asprintf("%02lld:%02lld", duration/60, duration%60);
    ui->videoDuration->setText(time);
}

void VideoBox::setVideoImage(const QString &photoFileId)
{
    // 向服务器发起下载图片的请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->downloadPhotoAsync(photoFileId);
}

void VideoBox::paintEvent(QPaintEvent *event)
{
    // 将图片数据更新到ui->imageBox中,注意：ui->imageBox是QWidget
    // 但QWidget不像QLabel一样，直接提供设置图片的方法
    // 因此只能通过画笔之类将图片画到QWidget中
    // QLabel label;
    // label.setPixmap(pixmap);

    // true: 控件在每次绘制时会自动使用调色板中的画刷来自动填充背景
    ui->imageBox->setAutoFillBackground(true);

    QPalette palette = ui->imageBox->palette();

    // 注意：由于图片尺寸可能和ui->imageBox的尺寸不相符
    // 导致图片绘制到控件上之后可能会占据控件的一部分，或者平铺等问题
    // 因此在使用pixmap构造画刷时，需要对图片进行缩放，将其缩放到控件的尺寸
    // Qt::KeepAspectRatioByExpanding: 图片在缩放时会保存原始的宽高比，但尽可能扩展以填充满控件
    // Qt::SmoothTransformation: 采用平滑的差值算法对图片进行缩放，可能提高图片的视觉质量
    videoCoverImage = videoCoverImage.scaled(ui->imageBox->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QBrush brush(videoCoverImage);
    palette.setBrush(QPalette::Window, brush);
    ui->imageBox->setPalette(palette);
}

void VideoBox::setUserAvatar(const QString &userAvatarId)
{
    // 如果用户没有设置过头像，则使用默认头像
    if(userAvatarId.isEmpty()){
        ui->userIcon->setStyleSheet("border-image : url(:/images/myself/defaultAvatar.png);");
        userAvatar = QPixmap(":/images/myself/defaultAvatar.png");
    }else{
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->downloadPhotoAsync(userAvatarId);
    }
}

void VideoBox::onPlayClicked()
{
    // 先从服务器获取弹幕数据
    auto dataCenter = model::DataCenter::getInstance();
    // 断开弹幕获取成功所绑定的所有槽函数
    disconnect(dataCenter, &model::DataCenter::getVideoBarrageDone, nullptr, nullptr);
    // 获取弹幕成功的信号槽绑定
    connect(dataCenter, &model::DataCenter::getVideoBarrageDone, this, &VideoBox::getVideoBarrageSuccess);
    dataCenter->getVideoBarrageAsync(videoInfo.videoId);

    // 再播放视频，注意：视频播放实在弹幕获取成功之后
    // 因此播放实现在获取弹幕成功的槽函数中处理
}

void VideoBox::getVideoImageDone(const QString &imageId, QByteArray imageData)
{
    // 确保下载的图片是当前VideoBox要设置的视频的封面图
    if(imageId != videoInfo.photoFileId){
        return;
    }

    // 将图片数据构造成一个QPixmap的对象
    videoCoverImage.loadFromData(imageData);
    repaint();
}

void VideoBox::getUserAvatarDone(const QString &imageId, QByteArray imageData)
{
    // 检测是否为当前用户的头像
    if(imageId != videoInfo.userAvatarId || imageData.isEmpty()){
        return;
    }

    // 制作原型头像并设置到界面
    userAvatar = makeCircleIcon(imageData, ui->userIcon->width()/2).pixmap(ui->userIcon->size());
    ui->userIcon->setPixmap(userAvatar);
}

void VideoBox::getVideoBarrageSuccess(const QString &videoId)
{
    if(videoId != videoInfo.videoId){
        return;
    }

    playerPage = new PlayerPage(videoInfo);

    playerPage->setUserAvatar(userAvatar);
    // 显示播放窗口
    playerPage->show();

    // 从服务器下载m3u8文件并播放
    // auto dataCenter = model::DataCenter::getInstance();
    // dataCenter->downloadVideoAsync(videoInfo.videoFileId);
    // connect(dataCenter, &model::DataCenter::downloadVideoDone, this, [=](const QString& m3u8Path, const QString& videoFileId){
    //     if(videoInfo.videoFileId != videoFileId){
    //         return;
    //     }

    //     playerPage->startPlaying(m3u8Path);
    // });
    playerPage->startPlaying();

    // 更新播放数
    connect(playerPage, &PlayerPage::incrementPlayNumber, this, [=]{
        this->videoInfo.playCount++;
        ui->playNum->setText(intToString(this->videoInfo.playCount));
    });

    // 更新点赞数
    connect(playerPage, &PlayerPage::updateLikeNum, this, [=](int64_t likeCount){
        this->videoInfo.likeCount = likeCount;
        ui->likeNum->setText(intToString(this->videoInfo.likeCount));
    });
}

void VideoBox::onMoreBtnClicked()
{
    // 设置菜单以及菜单项的样式
    QString style;
    // 菜单的样式
    style += "QMenu{"
             "background-color : #FFFFFF;"
             "border : none;"
             "border-radius : 6px;"
             "padding : 0"
             "}";
    // 设置菜单项样式
    style += "QMenu::item{"
             "background-color : #FFFFFF;"
             "border : none;"
             "border-radius : 6px;"
             "min-width : 50px;"
             "min-height : 32px;"
             "font-size : 12px;"
             "color : #222222;"
             "padding-left : 24px;"
             "}";
    // 菜单项选中样式
    style += "QMenu::item:selected{"
             "background-color : rgb(62, 206, 254);"
             "color : #FFFFFF;"
             "}";
    // 创建一个菜单
    QMenu menu(this);
    menu.setStyleSheet(style);

    // 如果想要让QMenu的圆角效果生效，需要去处窗口框架和默认阴影，并且要将菜单设置透明
    menu.setWindowFlags(menu.windowFlags() | Qt::FramelessWindowHint
                                           | Qt::NoDropShadowWindowHint);
    menu.setAttribute(Qt::WA_TranslucentBackground, true);

    // 在菜单中添加一个菜单项
    menu.addAction("删除");

    // 在鼠标点击位置弹出上下文菜单
    QPoint point = QCursor::pos();
    QAction* action = menu.exec(point);
    // action：用户对菜单中指定菜单项进行操作之后，exec会返回被操作菜单项对应的QAction
    // 否则返回nullptr
    if(nullptr == action){
        return;
    }

    if(action->text() == "删除"){
        // 点击删除菜单项
        // 删除操作无法直接在VideoBox中操作，由MySelfWidget来出来真正的删除动作
        // 因为删除之后，需要涉及更新页面视频列表操作，因此交给MySelfWidget来处理比较好
        emit deleteVideo(videoInfo.videoId);
    }
}



