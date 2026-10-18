#ifndef BITPLAYER_H
#define BITPLAYER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class BitPlayer;
}
QT_END_NAMESPACE


enum StackedWidgetPage{
    HomePage, MyselfPage, AdminPage, UploadPage
};

class BitPlayer : public QWidget
{
    Q_OBJECT

public:
    ~BitPlayer();
    static BitPlayer* getInstance();

    // 是否显示系统页面切换按钮
    void showSystemPageBtn(bool isShow);
    // 切换到其他用户页面，即我的页面但显示其他用户信息
    void switchToUserInfoPage(const QString& userId);
    // 该方法内部只桌UI界面切换，不做数据的更新处理
    void onSwitchPageUI(int pageId);

private slots:
    void onSwitchPage(int pageId);

private:
    BitPlayer(QWidget *parent = nullptr);

    // 设置界面的UI部分
    void initUI();

    // 绑定信号槽
    void connectSigalAndSlot();

    // 重置页面切换按钮
    void resetswitchBtnInfo(int pageId);

    // 鼠标单击
    void mousePressEvent(QMouseEvent *event);

    // 鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::BitPlayer *ui;

    QPoint dragPos;
    static BitPlayer* instance;
};
#endif // BITPLAYER_H
