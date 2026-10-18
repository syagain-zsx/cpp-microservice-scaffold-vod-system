#include "bulletscreenitem.h"
#include "QHBoxLayout"

BulletScreenInfo::BulletScreenInfo(const QString& userId, int64_t playTime, const QString& text)
    : userId(userId)
    , playTime(playTime)
    , text(text)
{}

BulletScreenItem::BulletScreenItem(QWidget *parent)
    : QFrame{parent}
{
    setObjectName("bulletScreenWnd");
    setFixedHeight(38);

    // 弹幕文本 和 用户头像处在水平布局器中
    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);

    // 用户头像
    imageLabel = new QLabel(this);
    imageLabel->setFixedSize(30, 30);
    imageLabel->setScaledContents(true);
    imageLabel->hide();

    // 弹幕文本
    textLabel = new QLabel(this);
    textLabel->setStyleSheet("font-family : 微软雅黑;"
                        "font-size : 18px;"
                        "color : #FFFFFF;"
                        "border : none");
    textLabel->adjustSize();

    // 将头像和弹幕文本的QLabel添加到布局器中
    layout->addWidget(imageLabel);
    layout->addWidget(textLabel);
    layout->setContentsMargins(10, 4, 19, 4);
    layout->setSpacing(8);
}

void BulletScreenItem::setBulletScreenText(const QString &text)
{
    textLabel->setText(text);
    textLabel->adjustSize();
}

void BulletScreenItem::setBulletScreenIcon(const QPixmap &pixmap)
{
    imageLabel->setPixmap(pixmap);
    imageLabel->show();
    this->setStyleSheet("#bulletScreenWnd{"
                        "border : 1px solid #3ECEFE;"
                        "border-radius : 19px;}");
}

void BulletScreenItem::setBulletScreenAnimation(int xPos, int duration)
{
    animation = new QPropertyAnimation(this, "pos", this);
    animation->setDuration(duration);
    animation->setStartValue(QPoint(xPos, 0));
    animation->setEndValue(QPoint(0 - this->width(), 0));

    connect(animation, &QPropertyAnimation::finished, this, [=](){
        delete this;
    });
}

void BulletScreenItem::startAnimation()
{
    show();
    animation->start();
}


