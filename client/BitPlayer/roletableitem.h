#ifndef ROLETABLEITEM_H
#define ROLETABLEITEM_H

#include <QWidget>
#include "./model/data.h"

namespace Ui {
class RoleTableItem;
}

class RoleTableItem : public QWidget
{
    Q_OBJECT

public:
    explicit RoleTableItem(QWidget *parent, const model::AdminInfo& adminInfo, int seqNumber);
    ~RoleTableItem();

private:
    void updateUI(int seqNumber);
    void initStyleSheet();
    bool logUserIsAdminSelf(const QString& text);

private slots:
    void onEditBtnClicked();
    void onStatusBtnClicked();
    void onDeleteBtnClicked();

private:
    Ui::RoleTableItem *ui;
    model::AdminInfo adminInfo;
    QHash<QString, QString> styleSheet;
};

#endif // ROLETABLEITEM_H
