#include "bitplayer.h"
#include "./ui_bitplayer.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include "util.h"
#include "./model/datacenter.h"


BitPlayer* BitPlayer::instance = nullptr;
BitPlayer* BitPlayer::getInstance()
{
    if(nullptr == instance){
        instance = new BitPlayer();
    }

    return instance;
}

void BitPlayer::showSystemPageBtn(bool isShow)
{
    if(isShow){
        ui->sysPageBtn->show();
    }else{
        ui->sysPageBtn->hide();
    }
}

BitPlayer::BitPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BitPlayer)
{
    ui->setupUi(this);
    initUI();

    connectSigalAndSlot();
}

BitPlayer::~BitPlayer()
{
    delete ui;
}

void BitPlayer::onSwitchPage(int pageId)
{
    onSwitchPageUI(pageId);

    // 如果pageId是我的页面，需要加载个人信息
    if(pageId == MyselfPage){
        ui->myPage->loadMyself();
    }else if(pageId == HomePage){
        LOG()<<"切換到HomePage頁面，重新添加視頻";
        ui->homePage->onRefreshBtnClicked();
    }else if(pageId == UploadPage){
        ui->uploadVideoPage->resetPage();
    }
}

void BitPlayer::switchToUserInfoPage(const QString &userId)
{
    onSwitchPageUI(MyselfPage);
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        // 临时用户加载后显示登录页面
        ui->myPage->loadMyself();
    }else{
        // 加载其他用户的个人信息
        ui->myPage->loadOtherUser(userId);
    }

}

void BitPlayer::onSwitchPageUI(int pageId)
{
    ui->stackedWidget->setCurrentIndex(pageId);
    resetswitchBtnInfo(pageId);
}

void BitPlayer::initUI()
{
    // 去除窗口的边框
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置窗口图标
    setWindowIcon(QIcon(":/images/homePage/logo.png"));

    // 窗口添加阴影效果
    QGraphicsDropShadowEffect* dropShadow = new QGraphicsDropShadowEffect(this);
    dropShadow->setColor(Qt::black);
    dropShadow->setBlurRadius(5);
    dropShadow->setOffset(0, 0);
    ui->bitPlayerBg->setGraphicsEffect(dropShadow);

    // 设置页面切换按钮的文本和图片
    ui->homePageBtn->setTextColor("#000000");
    ui->homePageBtn->setImageAndText(":/images/homePage/shouyexuan.png", "首页", HomePage);
    ui->myPageBtn->setTextColor("#999999");
    ui->myPageBtn->setImageAndText(":/images/homePage/wode.png", "我的", MyselfPage);
    ui->sysPageBtn->setTextColor("#999999");
    ui->sysPageBtn->setImageAndText(":/images/homePage/admin.png", "系统", AdminPage);

    // 默认显示首页
    ui->stackedWidget->setCurrentIndex(HomePage);

    // 登录成功之后，如果当前用户是管理员，则需要显示系统页面切换按钮
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isBUser()){
        ui->sysPageBtn->show();
    }else{
        ui->sysPageBtn->hide();
    }
}

void BitPlayer::connectSigalAndSlot()
{
    connect(ui->minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->quitBtn, &QPushButton::clicked, this, [=]{
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->saveDataFile();
        close();
    });

    connect(ui->homePageBtn, &PageSwitchButton::switchPage, this, &BitPlayer::onSwitchPage);
    connect(ui->myPageBtn, &PageSwitchButton::switchPage, this, &BitPlayer::onSwitchPage);
    connect(ui->sysPageBtn, &PageSwitchButton::switchPage, this, &BitPlayer::onSwitchPage);
    connect(ui->myPage, &MyselfWidget::switchUploadVideoPage, this, [=](int pageId, const QString& videoPath){
        onSwitchPage(pageId);
        ui->uploadVideoPage->setVideoTittle(videoPath);
    });

    connect(ui->uploadVideoPage, &UploadVideoPage::switchMyselfPage, this, [=](int pageId){
        onSwitchPage(pageId);
    });
}

void BitPlayer::resetswitchBtnInfo(int pageId)
{
    // 重新设置bodyLeft中按钮上的文本和图片的高亮
    // 设置文本的颜色
    QList<PageSwitchButton*> switchBtns = findChildren<PageSwitchButton*>();
    for(auto& switchBtn : switchBtns){
        if(switchBtn->getPageId() != pageId){
            switchBtn->setTextColor("#999999");
        }
    }

    // 设置按钮的图片
    if(pageId == HomePage){
        // 在HomePageBtn的图片设置为高亮
        // 将MyselfPageBtn、AdminPageBtn的图片设置为灰色
        ui->homePageBtn->setImage(":/images/homePage/shouyexuan.png");
        ui->myPageBtn->setImage(":/images/homePage/wode.png");
        ui->sysPageBtn->setImage(":/images/homePage/admin");
    }else if(pageId == MyselfPage){
        ui->homePageBtn->setImage(":/images/homePage/shouye.png");
        ui->myPageBtn->setImage(":/images/homePage/wodexuan.png");
        ui->sysPageBtn->setImage(":/images/homePage/admin");
    }else if(pageId == AdminPage){
        ui->homePageBtn->setImage(":/images/homePage/shouye.png");
        ui->myPageBtn->setImage(":/images/homePage/wode.png");
        ui->sysPageBtn->setImage(":/images/homePage/adminxuan");
    }else{
        LOG()<<"暂不支持的页面";
    }
}

void BitPlayer::mousePressEvent(QMouseEvent *event)
{
    // 先要获取鼠标点击时的位置
    QPoint point = event->position().toPoint();

    // 检测该位置是否在head中 且 是鼠标左键单击时
    if(ui->head->geometry().contains(point)){
        if(event->button() == Qt::LeftButton){
            dragPos = event->globalPosition().toPoint() - geometry().topLeft();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void BitPlayer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point = event->position().toPoint();
    if(ui->head->geometry().contains(point)){
        if(event->buttons() == Qt::LeftButton){
            move(event->globalPosition().toPoint() - dragPos);
            return;
        }
    }
}











