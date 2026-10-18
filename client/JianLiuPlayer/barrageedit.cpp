#include "barrageedit.h"
#include <QHBoxLayout>
#include "./model/datacenter.h"
#include "toast.h"

BarrageEdit::BarrageEdit(QWidget *parent)
    : QLineEdit{parent}
{
    // 设置编辑框的尺寸
    setFixedSize(1030, 32);
    setPlaceholderText("这个视频在说什么...");
    setMaxLength(30);
    setTextMargins(12, 6, 0, 7);  // 左 上 右 下

    // 初始化发送按钮
    sendBSBtn = new QPushButton(this);
    sendBSBtn->setFixedSize(58, 28);
    sendBSBtn->setText("发送");
    sendBSBtn->setCursor(QCursor(Qt::ArrowCursor));
    sendBSBtn->setStyleSheet("background-color : #3ECEFE;"
                             "color : #FFFFFF;"
                             "border-radius : 4px;"
                             "font-family : 微软雅黑;"
                             "font-size : 14px;");
    sendBSBtn->move(width()-sendBSBtn->width()-2, 2);
    connect(sendBSBtn, &QPushButton::clicked, this, &BarrageEdit::onSendBSBtnClicked);

    login = new Login();
    connect(login, &Login::loginSuccess, this, [=]{
        setPlaceholderText("发个友善的弹幕~~~");
        sendBSBtn->setText("发送");
    });
}

BarrageEdit::~BarrageEdit()
{
    delete login;
}

void BarrageEdit::mousePressEvent(QMouseEvent *event)
{
    // 当用户点击弹幕编辑框时，需要先检测用户是否登录
    // 如果是临时用户，需要提示让用户登录
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isTempUser()){
        if(placeholderText() != "请先登录或注册"){
            setPlaceholderText("请先登录或注册");
            sendBSBtn->setText("登录");
        }else{
            Toast::showMessage("请先登录或注册", login);
        }
    }
}

void BarrageEdit::onSendBSBtnClicked()
{
    if(sendBSBtn->text() == "登录"){
        Toast::showMessage("请先登录或注册", login);
    }else{
        QString barrageText = text();
        if(barrageText.isEmpty()){
            Toast::showMessage("请先输入要发送的弹幕~~~");
            return;
        }

        emit sendBulletScreen(text());
    }
}
