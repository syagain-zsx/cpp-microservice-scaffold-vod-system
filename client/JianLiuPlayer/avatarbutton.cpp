#include "avatarbutton.h"

MyPushButton::MyPushButton(QWidget *parent)
    : QPushButton(parent)
{}

void MyPushButton::mousePressEvent(QMouseEvent *event)
{
    if(clickedStatus){
        QPushButton::mousePressEvent(event);
    }
}

void MyPushButton::setClickedStatus(bool clickedStatus)
{
    this->clickedStatus = clickedStatus;
}

/////////////////////////////////////////////////////////////
AvatarButton::AvatarButton(QWidget *parent)
    : MyPushButton{parent}
    , isShow(true)
{
    mask = new QLabel(this);
    mask->setObjectName("maskLabel");
    mask->setGeometry(0, 0, 60, 60);
    mask->setStyleSheet("#maskLabel{"
                        "color : #FFFFFF;"
                        "background-color : rgba(0, 0, 0, 0.5);"
                        "border-radius : 30px;}");
    mask->setText("修改头像");
    mask->setAlignment(Qt::AlignCenter);
    mask->hide();
}

void AvatarButton::changeMode(bool isShow)
{
    this->isShow = isShow;
}

void AvatarButton::enterEvent(QEnterEvent *event)
{
    (void)event;
    if(isShow){
        mask->show();
    }
}

void AvatarButton::leaveEvent(QEvent *event)
{
    (void)event;
    mask->hide();
}
