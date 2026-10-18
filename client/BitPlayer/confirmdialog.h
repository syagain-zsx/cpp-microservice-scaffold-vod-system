#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>

class ConfirmDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfirmDialog(QWidget *parent = nullptr);
    bool getConfirm()const;
    void setText(const QString& text);
private:
    bool isConfirm = false;
    QLabel* textLabel;
};

#endif // CONFIRMDIALOG_H
