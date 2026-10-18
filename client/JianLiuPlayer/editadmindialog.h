#ifndef EDITADMINDIALOG_H
#define EDITADMINDIALOG_H

#include <QWidget>
#include <QDialog>
#include "./model/data.h"

namespace Ui {
class EditAdminDialog;
}

class EditAdminDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditAdminDialog(QWidget *parent, const QString& text, model::AdminInfo& adminInfo);
    ~EditAdminDialog();
    bool getCommitResult()const;
    void setEmailEditReadOnly(bool isReadOnly = true);

private slots:
    void onSubmitBtnClicked();
    void onCancelBtnClicked();

private:
    Ui::EditAdminDialog *ui;
    model::AdminInfo& adminInfo;
    bool isCommit = true;
};

#endif // EDITADMINDIALOG_H
