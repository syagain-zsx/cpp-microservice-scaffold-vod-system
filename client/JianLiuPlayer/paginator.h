#ifndef PAGINATOR_H
#define PAGINATOR_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include "pagebutton.h"


class Paginator : public QWidget
{
    Q_OBJECT
public:
    explicit Paginator(int pageCount=7, QWidget *parent = nullptr);

private:
    void setBtnStyle(QPushButton* btn);
    void initSignalAndSlots();
    void jumpToPage(int pageNo);
    void jumpToPageCase1(int pageNo);
    void jumpToPageCase2(int pageNo);
    void jumpToPageCase3(int pageNo);

private slots:
    void prevPage();
    void nextPage();
    void pageBtnClicked();

signals:
    void pageChanged(int page);

private:
    int pageCount = 7;    // 分页器中包含的总页数
    int pageSize = 20;    // 一页显示数据条数
    int currentPage = 1;  // 当前显示页号

    // 上一页和下一页
    QPushButton* prevPageBtn;
    QPushButton* nextPageBtn;

    // 页面切换按钮
    QList<PageButton*> pageBtns;
    // 跳转指定页编辑框
    QLineEdit* pageEdit;
};

#endif // PAGINATOR_H
