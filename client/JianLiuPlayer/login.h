#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    void reset();

    void showWindow();

private slots:
    // 密码登录
    void onPasswordBtnClicked();
    // 邮箱登录
    void onEmailBtnClicked();
    // 获取验证码
    void onAuthcodeBtnClicked();
    // 登录或注册
    void onLoginAndRegisterBtnClicked();
    // 立即注册按钮
    void onRegisterBtnClicked();

    // 获取邮箱验证码成功
    void onAuthcodeDone(const QString authcodeId);

    // 密码登录
    void onLoginNowBtnClicked();

signals:
    void loginSuccess();

private:
    Ui::Login *ui;
    QString authcodeId;      // 保存从服务器获取的邮箱验证码的id
};

#endif // LOGIN_H
