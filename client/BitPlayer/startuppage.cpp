#include "startuppage.h"
#include <QLabel>
#include <QTimer>
#include "./model/datacenter.h"
#include "toast.h"

StartupPage::StartupPage(QDialog *parent)
    : QDialog{parent}
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setFixedSize(1450, 860);
    setStyleSheet("background-color : #FFFFFF");

    // 在窗口中显示图片
    QLabel* imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap(":/images/startupPage/biteshipin.png"));
    imageLabel->move(524, 374);

    // 临时用户登录成功信号槽绑定
    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::loginTempUserDone, this, [=]{
        loginSuccess = true;
        dataCenter->buildTempUserInfo();
        mySelfInfoSuccess = true;
    });

    // session登录成功
    connect(dataCenter, &model::DataCenter::loginSessionDone, this, [=](bool isTempUser){
        loginSuccess = true;
        if(isTempUser){
            dataCenter->buildTempUserInfo();
            mySelfInfoSuccess = true;
        }else{
            dataCenter->getMyselfInfoAsync();
        }
    });

    // session登录失败
    connect(dataCenter, &model::DataCenter::loginSessionFailed, this, [=](const QString& reason){
        loginSuccess = false;
        Toast::showMessage("session登录失败, "+reason);
    });

    connect(dataCenter, &model::DataCenter::getMyselfInfoDone, this, [=]{
        mySelfInfoSuccess = true;
    });
}

void StartupPage::startTimer()
{
    QTimer* timer = new QTimer();
    // 定义式会在时间达到之后重复出发timeout信号
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, [=]{
        if(loginSuccess && mySelfInfoSuccess){
            timer->stop();
            delete timer;
            close();
        }
    });

    timer->start(2000);

    // 如果sessionId为空，说明用户是第一次使用BitPlayer，此时以临时用户方式登录
    // 如果sessionId非空，但是当前用户是临时用户，此时以临时用户方式登录
    // 如果sessionId非空，当前用户也是非临时用户，此时使用session登录
    auto dataCenter = model::DataCenter::getInstance();
    QString sessionId = dataCenter->getLoginSessionId();
    if(sessionId.isEmpty() || dataCenter->getMyselfInfo()->isTempUser()){
        // 临时登录
        dataCenter->loginTempUserAsync();
    }else{
        dataCenter->loginSessionAsync();
    }
}
