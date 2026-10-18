#include "newpassworddialog.h"
#include "ui_newpassworddialog.h"
#include "bitplayer.h"

NewPasswordDialog::NewPasswordDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewPasswordDialog)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);

    connect(ui->submitBtn, &QPushButton::clicked, this, &NewPasswordDialog::onSubmitBtnClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::close);

    connect(ui->passwordEdit1, &QLineEdit::editingFinished, this, &NewPasswordDialog::onEdittingFinished);
    connect(ui->passwordEdit2, &QLineEdit::editingFinished, this, &NewPasswordDialog::onEdittingFinished);

    // 位置在主窗口中心显示
    BitPlayer* bitplayer = BitPlayer::getInstance();
    QPoint point = bitplayer->mapToGlobal(QPoint(0, 0));
    point.setX(point.x() + (bitplayer->width()-width())/2);
    point.setY(point.y() + (bitplayer->height()-height())/2);
    move(point);
}

NewPasswordDialog::~NewPasswordDialog()
{
    delete ui;
}

const QString &NewPasswordDialog::getPassword() const
{
    return password;
}

bool NewPasswordDialog::checkPasswordEdit()
{
    // 1. 检测passwordEdit1 或者 passwordEdit2 是否为空
    if(ui->passwordEdit1->text().isEmpty() || ui->passwordEdit2->text().isEmpty()){
        ui->messageLabel->setText("密码不能为空");
        return false;
    }

    // 2. 检测密码是否符合条件
    QString message = passwordValid(ui->passwordEdit1->text());
    if(!message.isEmpty()){
        ui->messageLabel->setText(message);
        return false;
    }

    message = passwordValid(ui->passwordEdit2->text());
    if(!message.isEmpty()){
        ui->messageLabel->setText(message);
        return false;
    }

    // 3. 检测passwordEdit1 或者 passwordEdit2中的密码是否相等
    if(ui->passwordEdit1->text() != ui->passwordEdit2->text()){
        ui->messageLabel->setText("两次密码输入必须一致");
        return false;
    }
    ui->messageLabel->setText("");
    return true;
}

// 返回值含义：检测失败时，返回失败的原因
QString NewPasswordDialog::passwordValid(const QString &password)
{
    // 密码条件：包含大小写字母，数字以及标点符号，至少包含两种类型的字符，长度必须在8~16位之间
    // 1. 验证长度是否在8~16位之间
    if(password.length() < 8){
        return QString("密码长度不能小于 8 位");
    }

    if(password.length() > 16){
        return QString("密码长度不能大于 16 位");
    }

    // 2. 检测密码中的字母是否满足：大小写字母、数字、标点符号
    //    统计出字符种类的信息
    enum CharType{
        Upper,
        Lower,
        Digit,
        Punct
    };

    // QMap用来记录某种字符是否出现过
    QMap<CharType, int> charTypeMap;
    for(int i = 0; i < password.length(); ++i){
        QChar ch = password[i];
        if(ch.isUpper()){
            charTypeMap[Upper] = 1;
        }else if(ch.isLower()){
            charTypeMap[Lower] = 1;
        }else if(ch.isDigit()){
            charTypeMap[Digit] = 1;
        }else if(ch.isPunct()){
            charTypeMap[Punct] = 1;
        }else{
            // 如果当前字符不是以上四种，直接返回false
            return QString("密码中只能包含数字、字母以及特殊符号");
        }
    }

    // 3. 检测密码是否至少包含两种不同的字符
    // 遍历charTypeMap，统计value之和
    int totalCount = 0;
    for(auto it = charTypeMap.begin(); it != charTypeMap.end(); ++it){
        totalCount += it.value();
    }

    if(totalCount < 2){
        return QString("密码中必须至少包含两类型字符");
    }

    return QString("");
}

void NewPasswordDialog::onEdittingFinished()
{
    checkPasswordEdit();
}

void NewPasswordDialog::onSubmitBtnClicked()
{
    if(!checkPasswordEdit()){
        return;
    }

    // 保存密码
    password = ui->passwordEdit1->text();

    close();
}

