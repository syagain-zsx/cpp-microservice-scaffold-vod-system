#ifndef MYSELFWIDGET_H
#define MYSELFWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "login.h"

namespace Ui {
class MyselfWidget;
}

////////////////////////////////// AttentionButton类 //////////////////////////////////
class AttentionButton : public QPushButton
{
    Q_OBJECT
public:
    explicit AttentionButton(QWidget* parent = nullptr);
    bool isAttention()const;
    void changeStatus(bool isAttentioned);
private:
    bool isAttentionStatus = false;    // 是否被关注
};


////////////////////////////////// MyselfWidget类 //////////////////////////////////
class MyselfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyselfWidget(QWidget *parent = nullptr);
    ~MyselfWidget();
    // 获取自己的信息
    void getMyselfInfo();
    // 加载当前用户信息
    void loadMyself();
    // 获取其他用户信息
    void getOtherUserInfo(const QString& userId);
    // 加载其他用户信息
    void loadOtherUser(const QString& userId);

private:
    void initUI();
    void connectSignalsAndSlots();
    void hideWidget(bool isHide = true);
    // 获取用户视频列表
    void getUserVideoList(const QString& userId, int pageIndex);
    // 清空界面视频数据
    void clearVideoList();

private slots:
    void onAvatarBtnClicked();
    void onSettingBtnClicked();
    void onUploadVideoBtnClicked();
    void onNicknameBtnClicked();
    void onQuitBtnClicked();
    // 获取当前用户信息成功
    void getMyselfInfoDone();
    // 获取其他用户信息成功
    void getOtherUserInfoDone();
    // 获取用户头像成功
    void getAvatarDone(const QString& fileId, const QByteArray avatarData);
    // 注意：上传图片分为两个阶段完成
    // 1. 上传图片文件-->服务器返回fileId
    void uploadAvatarDone1(const QString& fileId, QWidget* whichPage);
    // 2. 修改个人信息中的fileIdId
    void uploadAvatarDone2();
    // 获取用户视频列表成功
    void getUserListDone(const QString& userId, const QString& whichPage);
    // 当QScrollArea的垂直滚动条滑动到底部的时候，需要获取下一页视频
    void onScrollAreaValueChanged(int value);
    // 删除视频
    void deleteVideo(const QString& videoId);
    // 删除视频完成
    void deleteVideoDone(const QString& videoId);
    // 关注按钮点击
    void onAttentionBtnClicked();
    // 关注用户成功
    void newAttentionDone();
    // 取消关注用户成功
    void delAttentionDone();
    // 退出登录成功
    void logoutDone();
    // 修改昵称成功
    void setNicknameDone(const QString& nickname);

signals:
    void switchUploadVideoPage(int pageIndex, const QString& videoPath);

private:
    Ui::MyselfWidget *ui;
    QString userId;       // 记录当前页面显示的是哪个用户的数据，如果为空表示当前登录用户，否则表示其他用户信息
    Login* login;
    bool isModifyPassword = false;
};

#endif // MYSELFWIDGET_H
