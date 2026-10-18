#ifndef BULLETSCREENITEM_H
#define BULLETSCREENITEM_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPropertyAnimation>

class BulletScreenInfo
{
public:
    QString userId;    // 发送弹幕的用户id
    QString videoId;   // 弹幕对应的视频的id
    int64_t playTime;  // 发送弹幕时视频的当前播放时间
    QString text;      // 弹幕内容

    BulletScreenInfo(const QString& userId = "", int64_t playTime = 0, const QString& text = "");
};


class BulletScreenItem : public QFrame
{
    Q_OBJECT
public:
    explicit BulletScreenItem(QWidget *parent = nullptr);
    // 设置弹幕中文本
    void setBulletScreenText(const QString& text);

    // 设置弹幕中头像
    void setBulletScreenIcon(const QPixmap& pixmap);

    // 给弹幕项设置动画效果
    void setBulletScreenAnimation(int xPos, int duration);

    // 开启动画
    void startAnimation();

private:
    QLabel* textLabel;              // 弹幕中的文本
    QLabel* imageLabel;             // 弹幕中的用户头像
    QPropertyAnimation* animation;  // 动画类指针
};

#endif // BULLETSCREENITEM_H
