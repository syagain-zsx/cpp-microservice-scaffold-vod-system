#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include <QWidget>

namespace Ui {
class AdminWidget;
}

class AdminWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWidget(QWidget *parent = nullptr);
    ~AdminWidget();

private slots:
    void onCheckBtnClicked();
    void onRoleBtnClicked();

private:
    Ui::AdminWidget *ui;

    // 按钮未选中
    const QString inactiveTabStyle = "QPushButton{"
                                     "background-color : #FFFFFF;"
                                     "font-family : 微软雅黑;"
                                     "font-size : 14px;"
                                     "color : #666666;"
                                     "border : none;"
                                     "border-bottom : 2px solid #F5F6F8;}";

    // 按钮选中
    const QString activeTabStyle = "QPushButton{"
                                   "background-color : #FFFFFF;"
                                   "font-family : 微软雅黑;"
                                   "font-size : 14px;"
                                   "color : #666666;"
                                   "border : none;"
                                   "border-bottom : 2px solid #3ECEFF;}";
};

#endif // ADMINWIDGET_H
