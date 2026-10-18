#ifndef CHECKTABLE_H
#define CHECKTABLE_H

#include <QWidget>
#include "paginator.h"
#include "./model/data.h"

namespace Ui {
class CheckTable;
}

class CheckTable : public QWidget
{
    Q_OBJECT

public:
    explicit CheckTable(QWidget *parent = nullptr);
    ~CheckTable();

private:
    void getVideoList(int page);
    void resetPaginator(int pageCount);

private slots:
    void onResetBtnClicked();
    void onQueryBtnClicked();
    // 获取用户视频列表成功
    void updateCheckTable(const QString& userId, const QString whichPage);

private:
    Ui::CheckTable *ui;
    Paginator* paginator;
    int page = 1;
    model::VideoStatus videoStatus;    // 点击查询按钮时获取视频状态
};

#endif // CHECKTABLE_H
