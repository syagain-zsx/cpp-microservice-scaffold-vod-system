#ifndef AVATARBUTTON_H
#define AVATARBUTTON_H

#include <QPushButton>
#include <QLabel>

class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyPushButton(QWidget *parent = nullptr);
    void setClickedStatus(bool enable);

protected:
    void mousePressEvent(QMouseEvent* event) override;

protected:
    bool clickedStatus;     // 标记按钮是否可以点击
};

class AvatarButton : public MyPushButton
{
    Q_OBJECT
public:
    explicit AvatarButton(QWidget *parent = nullptr);
    void changeMode(bool isShow);
private:
    // 鼠标进入
    void enterEvent(QEnterEvent* event)override;

    // 鼠标离开
    void leaveEvent(QEvent* evnet)override;

private:
    QLabel* mask;    // 遮罩层
    bool isShow;     // 是否显示遮罩层
};

#endif // AVATARBUTTON_H
