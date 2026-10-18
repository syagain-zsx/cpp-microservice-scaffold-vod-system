#ifndef PLAYERPAGE_H
#define PLAYERPAGE_H

#include <QWidget>
#include "volume.h"
#include "playspeed.h"
#include "./mpv/mpvplayer.h"
#include <QDialog>
#include <QFrame>
#include "bulletscreenitem.h"
#include "./model/data.h"
#include "login.h"

namespace Ui {
class PlayerPage;
}

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(const model::VideoInfo& videoInfo, QWidget *parent = nullptr);
    ~PlayerPage();

    // 鼠标按下
    void mousePressEvent(QMouseEvent* event);

    // 鼠标移动
    void mouseMoveEvent(QMouseEvent* event);

    void startPlaying();

    // 设置用户头像
    void setUserAvatar(QPixmap& userAvatar);

    // 设置视频是哪个页面播放
    void setPlayPage(const QString& whichPage);

private slots:
    void onVolumeBtnClicked();
    void onSpeedBtnClicked();
    void onLikeImageBtnClicked();
    void onPlayBtnClicked();
    void onPlaySpeedChanged(double speed);
    void setVolume(int volume);
    void onPlayPositionChanged(int64_t playTime);
    // 整个视频播放结束槽函数
    void onEndOfPlaylist();
    void onSetPlayProgress(double playRatio);
    void onBulletScreenBtnClicked();
    // 发射弹幕槽函数
    void onSendBulletScreenBtnClicked(const QString& text);
    void onQuitBtnClicked();
    // 用户头像点击
    void onUserAvatarBtnClicked();

private:
    void moveWindows(const QPoint& point);
    QString secondToTime(int64_t seconds);

    // 布局弹幕主窗口
    void initBarrageArea();
    // 构造弹幕数据
    void buildBulletScreenData();

    // 显示弹幕信息
    void showBulletScreen();

    // 更新视频信息
    void updateVideoInfoUI();

    // 更新播放数
    void updatePlayNumer();

signals:
    // 通过videoBox更新界面中的播放数
    void incrementPlayNumber();

    // 通知videoBox更新界面中的点赞数
    void updateLikeNum(int64_t likeCount);

private:
    Ui::PlayerPage *ui;
    QPoint dragPos;
    Volume* volume;
    PlaySpeed* playSpeed;
    Login* login;

    // 添加MpvPlayer的对象
    MpvPlayer* mpvPlayer = nullptr;
    bool isPlay = false;    // 默认情况下视频是暂停状态

    // 弹幕相关的成员
    QDialog* barrageArea;
    QFrame* top;
    QFrame* middle;
    QFrame* bottom;
    // key：视频的当前播放时间
    // value: 当前播放时间下的所有弹幕数据
    QMap<int64_t, QList<BulletScreenInfo>> bulletScreens;
    bool isStartBS = true;  // 弹幕默认情况下打开
    model::VideoInfo videoInfo;   // 保存视频信息
    // 保存视频的弹幕信息
    // key: 当前视频的播放时间
    // value : 当前播放时间下的所有弹幕数据
    QHash<int64_t, QList<model::BarrageInfo>> barrages;
    bool isUpdatePlayNum = false;   // 标记视频播放数是否已经更新，默认情况下未更新
    bool isLike = false;            // 标记点赞信息是否被更改过
    int64_t likeCount;              // 标记点赞数

    QString whichPage;
    QByteArray loginUserAvatar;
};

#endif // PLAYERPAGE_H
