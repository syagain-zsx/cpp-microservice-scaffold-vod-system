#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchLineEdit(QWidget *parent = nullptr);

private slots:
    void onSearchBtnClicked();

signals:
    // 发送搜索信号给首页
    void searchVideos(const QString& searchText);
};

#endif // SEARCHLINEEDIT_H
