#ifndef VIDEOBOX_H
#define VIDEOBOX_H

#include <QWidget>
#include "playerpage.h"
#include "./model/data.h"

namespace Ui {
class VideoBox;
}

class VideoBox : public QWidget
{
    Q_OBJECT

public:
    explicit VideoBox(const model::VideoInfo& videoInfo, QWidget *parent = nullptr);
    ~VideoBox();

    bool eventFilter(QObject *watched, QEvent *event)override;

    // 是否显示 “...” 按钮，该按钮主要用来支持删除我的页面中的视频
    void showMoreBtn(bool isShow);

    // 设置视频用户昵称-用户修改昵称后，VideoBox中显示的用户昵称需要同步修改
    void setNicknameOfVideoUser(const QString& nickname);

private:
    void updateVideoInfoUI();
    void setVideoDuration(int64_t duration);

    // 设置视频封面
    void setVideoImage(const QString& photoFileId);

    //  重写paintEvent事件，避免图片平铺重叠
    void paintEvent(QPaintEvent* event)override;

    // 设置用户头像
    void setUserAvatar(const QString& userAvatarId);

private slots:
    void onPlayClicked();
    // 视频封面图片下载成功
    void getVideoImageDone(const QString& imageId, QByteArray imageData);
    // 设置用户头像成功
    void getUserAvatarDone(const QString& imageId, QByteArray imageData);
    // 获取弹幕数据成功槽函数
    void getVideoBarrageSuccess(const QString& videoId);
    // ... 按钮点击
    void onMoreBtnClicked();

signals:
    void deleteVideo(const QString& videoId);

private:
    Ui::VideoBox *ui;
    PlayerPage* playerPage = nullptr;
    model::VideoInfo videoInfo;
    QPixmap videoCoverImage;   // 视频封面画图
    QPixmap userAvatar;        // 用户头像
};

#endif // VIDEOBOX_H
