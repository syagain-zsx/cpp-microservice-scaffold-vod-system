#ifndef NEWPASSWORDDIALOG_H
#define NEWPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class NewPasswordDialog;
}

class NewPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPasswordDialog(QWidget *parent = nullptr);
    ~NewPasswordDialog();
    const QString& getPassword()const;

private:
    bool checkPasswordEdit();
    QString passwordValid(const QString& password);

private slots:
    // QLineEdit编辑结束时触发editingFinished信号的槽函数
    void onEdittingFinished();

    // 提交按钮槽函数
    void onSubmitBtnClicked();

private:
    Ui::NewPasswordDialog *ui;
    QString password;
};

#endif // NEWPASSWORDDIALOG_H
