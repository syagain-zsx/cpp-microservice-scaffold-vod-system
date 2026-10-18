#ifndef ROLETABLE_H
#define ROLETABLE_H

#include <QWidget>
#include "paginator.h"
#include "./model/data.h"

namespace Ui {
class RoleTable;
}

class RoleTable : public QWidget
{
    Q_OBJECT

public:
    explicit RoleTable(QWidget *parent = nullptr);
    ~RoleTable();

private:
    void initStyle();
    void updateRoleTable();
    // 获取管理员列表
    void getAdminList(int page);
    // 重置分页器
    void resetPaginator(int pageCount);

private slots:
    void onResetBtnClicked();
    void onQueryBtnClicked();
    void onInsertBtnClicked();
private:
    Ui::RoleTable *ui;

    // key为：选中、未选中  value：样式
    QMap<QString, QString> styleSheet;
    Paginator* paginator;
    int page = 1;
    model::AdminStatus adminStatus;
};

#endif // ROLETABLE_H
