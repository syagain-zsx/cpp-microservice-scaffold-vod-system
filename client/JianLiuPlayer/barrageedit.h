#ifndef BARRAGEEDIT_H
#define BARRAGEEDIT_H

#include <QLineEdit>
#include <QPushButton>
#include "login.h"

class BarrageEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit BarrageEdit(QWidget *parent = nullptr);
    ~BarrageEdit();

    // 鼠标在编辑框中点击时
    virtual void mousePressEvent(QMouseEvent* event)override;

private slots:
    void onSendBSBtnClicked();

signals:
    void sendBulletScreen(const QString& text);

private:
    QPushButton* sendBSBtn;
    Login* login;
};

#endif // BARRAGEEDIT_H
