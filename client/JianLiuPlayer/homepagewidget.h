#ifndef HOMEPAGEWIDGET_H
#define HOMEPAGEWIDGET_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class HomePageWidget;
}

// 向服务器获取视频列表的方式
enum VideoListStyle
{
    AllStyle,        // 所有视频类型
    KindStyle,       // 分类视频列表
    TagStyle,        // 标签视频列表
    SearchStyle      // 搜索视频列表
};

class HomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageWidget(QWidget *parent = nullptr);
    ~HomePageWidget();
    // 刷新按钮点击
    void onRefreshBtnClicked();

private:
    void initKindsAndTags();
    QPushButton* buildSelectBtn(QWidget* parent, const QString& color, const QString& text);
    void resetTags(const  QList<QString>& tags);
    void initRefreshAndTopBtn();
    void initVideos();
    void connectSignalAndSlot();
    // 清空QGrideLayoyut中的旧视频数据
    void clearLayoutVidoes();

private slots:
    // 分类按钮点击
    void onKindBtnClicked(QPushButton* clickedKindBtn);

    // 标签按钮点击
    void onTagBtnClicked(QPushButton* clickedTagBtn);

    // 文本为分类的按钮点击
    void onKindsBtnClicked();

    // 文本为标签的按钮点击
    void onTagsBtnClicked();

    // 置顶按钮点击
    void onTopBtnClicked();

    // 获取所有视频列表的槽函数
    void updateVideoList();

    // 搜索视频
    void onSearchVideos(const QString& searchText);

    // QScrollBar::valueChanged信号的槽函数
    void OnScrollBarValueChanged(int value);

private:
    Ui::HomePageWidget *ui;
    QString curKind;       // 保存当前获取哪个分类下的视频
    QString curTag;        // 保存当前获取哪个标签下的视频

    // 标记最近一次获取视频列表的方式
    VideoListStyle videoListStyle;
};

#endif // HOMEPAGEWIDGET_H
