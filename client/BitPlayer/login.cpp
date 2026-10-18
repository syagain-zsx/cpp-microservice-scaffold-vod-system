#include "login.h"
#include "ui_login.h"
#include <QRegularExpressionValidator>
#include <QGraphicsDropShadowEffect>
#include "toast.h"
#include "./model/datacenter.h"
#include "bitplayer.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);

    // 去除窗口边框，并设置窗口为模态对话框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_ShowModal, true);

    // 窗口设置阴影效果
    setAttribute(Qt::WA_TranslucentBackground);   // 将窗口背景设置为透明
    QGraphicsDropShadowEffect* dropShadow = new QGraphicsDropShadowEffect(this);
    dropShadow->setColor(Qt::black);
    dropShadow->setBlurRadius(3);
    dropShadow->setOffset(0, 0);
    ui->loginBg->setGraphicsEffect(dropShadow);

    ui->passwordNum->setEchoMode(QLineEdit::Password);
    // 邮箱编辑框限制
    // 569334855@qq.com
    // [a-zA-Z0-9._%+-]+: 匹配邮箱用户名部分，允许字母、数字、点、下划线、百分号、加号和减号
    // @: 必须包含一个@符号
    // [a-zA-z0-9.-]+: 匹配域名部分，允许字母、数字、点和减号
    // \.[a-zA-Z]{2,}: 匹配顶级域名，必须以点开头，后跟至少两个字母
    QRegularExpression regExp(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QValidator* validator = new QRegularExpressionValidator(regExp, this);
    ui->emailNum->setValidator(validator);

    // 验证码的限制
    QIntValidator* intValidator = new QIntValidator(0, 999999, ui->authcode);
    ui->authcode->setValidator(intValidator);
    ui->authcode->setMaxLength(6);

    // 默认显示邮箱登录
    ui->emailWidget->show();
    ui->passwordWidget->hide();

    connect(ui->minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->quitBtn, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->passwordBtn, &QPushButton::clicked, this, &Login::onPasswordBtnClicked);
    connect(ui->emailBtn, &QPushButton::clicked, this, &Login::onEmailBtnClicked);
    connect(ui->authcodeBtn, &QPushButton::clicked, this, &Login::onAuthcodeBtnClicked);
    connect(ui->loginOrRegister, &QPushButton::clicked, this, &Login::onLoginAndRegisterBtnClicked);
    connect(ui->registerNow, &QPushButton::clicked, this, &Login::onRegisterBtnClicked);
    connect(ui->loginNow, &QPushButton::clicked, this, &Login::onLoginNowBtnClicked);

    // 获取邮箱验证码成功
    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getAuthcodeDone, this, &Login::onAuthcodeDone);

    // 邮箱登录成功
    connect(dataCenter, &model::DataCenter::loginWithEmailDone, this, [=](){
        emit loginSuccess();  // 由MySelfWidget来处理，从新获取登录成功的用户的个人信息 以及 视频列表
        close();
    });

    // 邮箱登录失败
    connect(dataCenter, &model::DataCenter::loginWithEmailFailed, this, [=](const QString& errorInfo){
        Toast::showMessage(errorInfo);
    });

    // 密码登录成功
    connect(dataCenter, &model::DataCenter::loginWithPasswordDone, this, [=](){
        emit loginSuccess();  // 由MySelfWidget来处理，从新获取登录成功的用户的个人信息 以及 视频列表
        close();
    });

    // 密码登录失败
    connect(dataCenter, &model::DataCenter::loginWithPasswordFailed, this, [=](const QString& errorInfo){
        Toast::showMessage(errorInfo);
    });
}

Login::~Login()
{
    delete ui;
}

void Login::reset()
{
    ui->accountNum->setText("");
    ui->passwordNum->setText("");
    ui->authcode->setText("");
    ui->emailNum->setText("");
}

void Login::showWindow()
{
    BitPlayer* bitplayer = BitPlayer::getInstance();
    QPoint point = bitplayer->mapToGlobal(QPoint(0, 0));
    point.setX(point.x() + (bitplayer->width()-width())/2);
    point.setY(point.y() + (bitplayer->height()-height())/2);
    move(point);
}

void Login::onPasswordBtnClicked()
{
    ui->passwordWidget->show();
    ui->emailWidget->hide();

    ui->passwordBtn->setStyleSheet("QPushButton{"
                                   "color : #3ECEFE;"
                                   "font-weight : bold;"
                                   "border : none;"
                                   "border-bottom : 6px solid #3ECEFE;}");

    ui->emailBtn->setStyleSheet("QPushButton{"
                                   "color : #222222;"
                                   "border : none;"
                                   "border-bottom : 2px solid #B5ECFF;}");
}

void Login::onEmailBtnClicked()
{
    ui->emailWidget->show();
    ui->passwordWidget->hide();

    ui->emailBtn->setStyleSheet("QPushButton{"
                                   "color : #3ECEFE;"
                                   "font-weight : bold;"
                                   "border : none;"
                                   "border-bottom : 6px solid #3ECEFE;}");

    ui->passwordBtn->setStyleSheet("QPushButton{"
                                  "color : #222222;"
                                  "border : none;"
                                  "border-bottom : 2px solid #B5ECFF;}");
}

void Login::onAuthcodeBtnClicked()
{
    // 获取邮箱
    QString email = ui->emailNum->text();
    if(email.isEmpty()){
        Toast::showMessage("请输入邮箱!!!");
    }else{
        // 校验邮箱格式是否正确
        if(!ui->emailNum->hasAcceptableInput()){
            Toast::showMessage("邮箱格式有误!!!");
            return;
        }
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->getAuthcodeAsync(email);
    }
}

void Login::onLoginAndRegisterBtnClicked()
{
    // 检测邮箱 或 验证码是否输入
    const QString email = ui->emailNum->text();
    const QString authcode = ui->authcode->text();
    if(email.isEmpty() || authcode.isEmpty()){
        Toast::showMessage("邮箱 或 验证码 不能为空!!!");
        return;
    }

    // 校验邮箱格式是否正确
    if(!ui->emailNum->hasAcceptableInput()){
        Toast::showMessage("邮箱格式错误!!!");
        return;
    }

    // 校验验证码格式是否正确
    if(!ui->authcode->hasAcceptableInput()){
        Toast::showMessage("验证码格式错误!!!");
        return;
    }

    // 发送邮箱登录请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->loginWithEmailAsync(email, authcode, authcodeId);
}

void Login::onRegisterBtnClicked()
{
    // 切换 立即注册 和 立即登录 按钮的样式
    ui->registerNow->setStyleSheet("#registerNow{"
                                   "background-color : #3ECEFE;"
                                   "color : #FFFFFF;"
                                   "border-radius : 36px;"
                                   "}");
    ui->loginNow->setStyleSheet("#loginNow{"
                                "background-color : #FFFFFF;"
                                "color : #222222;"
                                "border-radius : 36px;"
                                "border : 1px solid, #C2C9D9;}");
    // 切换到邮箱登录页面
    onEmailBtnClicked();
}

void Login::onAuthcodeDone(const QString authcodeId)
{
    this->authcodeId = authcodeId;
    Toast::showMessage("验证码已经发送!!!");
}

void Login::onLoginNowBtnClicked()
{
    // 切换 立即注册 和 立即登录 按钮的样式
    ui->loginNow->setStyleSheet("#loginNow{"
                                   "background-color : #3ECEFE;"
                                   "color : #FFFFFF;"
                                   "border-radius : 36px;"
                                   "}");
    ui->registerNow->setStyleSheet("#registerNow{"
                                "background-color : #FFFFFF;"
                                "color : #222222;"
                                "border-radius : 36px;"
                                "border : 1px solid, #C2C9D9;}");

    // 获取邮箱或用户昵称 和 密码
    QString accountNum = ui->accountNum->text();
    QString password = ui->passwordNum->text();
    // 发送密码登录请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->loginWithPasswordAsync(accountNum, password);
}
