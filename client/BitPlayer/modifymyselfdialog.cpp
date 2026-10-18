#include "modifymyselfdialog.h"
#include "ui_modifymyselfdialog.h"
#include "util.h"
#include "newpassworddialog.h"
#include "./model/datacenter.h"
#include "toast.h"
#include "bitplayer.h"

ModifyMyselfDialog::ModifyMyselfDialog(bool isModifyPassoword, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModifyMyselfDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    if(isModifyPassoword){
        ui->passwordWidget->show();
        ui->passwordBtn->hide();
    }else{
        ui->passwordWidget->hide();
        ui->passwordBtn->show();
    }


    // 加载用户邮箱 和 昵称 到界面
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    ui->emailNumberLabel->setText(hideEmail(myselfInfo->email));
    ui->nickNameEdit->setText(myselfInfo->nickname);
    ui->nickNameEdit->setMaxLength(16);

    connect(ui->submitBtn, &QPushButton::clicked, this, &ModifyMyselfDialog::onSubmitBtnClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &ModifyMyselfDialog::onCancelBtnClicked);
    connect(ui->passwordBtn, &QPushButton::clicked, this, &ModifyMyselfDialog::showPasswordDlg);
    connect(ui->changePasswordBtn, &QPushButton::clicked, this, &ModifyMyselfDialog::showPasswordDlg);

    // 设置昵称失败
    connect(dataCenter, &model::DataCenter::setNicknameFailed, this, [this](const QString& reason){
        Toast::showMessage(reason);
    });

    // 位置在主窗口中心显示
    BitPlayer* bitplayer = BitPlayer::getInstance();
    QPoint point = bitplayer->mapToGlobal(QPoint(0, 0));
    point.setX(point.x() + (bitplayer->width()-width())/2);
    point.setY(point.y() + (bitplayer->height()-height())/2);
    move(point);
}

ModifyMyselfDialog::~ModifyMyselfDialog()
{
    delete ui;
}

void ModifyMyselfDialog::onSubmitBtnClicked()
{
    auto dataCenter = model::DataCenter::getInstance();
    // 修改密码
    if(!newPassword.isEmpty()){
        dataCenter->setPasswordAsync(newPassword);
    }

    // 修改昵称
    // trimmed(): 去除字符串起始和末尾的空白字符
    const QString nickname = ui->nickNameEdit->text().trimmed();
    if(nickname.size() > 16){
        Toast::showMessage("昵称最多16个汉字!!!");
        return;
    }

    auto myselfInfo = dataCenter->getMyselfInfo();
    if(nickname != myselfInfo->nickname){
        dataCenter->setNicknameAsync(nickname);
    }

    // 关闭对话框
    close();
}


void ModifyMyselfDialog::onCancelBtnClicked()
{
    close();
}

void ModifyMyselfDialog::showPasswordDlg()
{
    NewPasswordDialog* dlg = new NewPasswordDialog();
    dlg->exec();

    /*
     * 用户点击修改密码，弹出修改密码对话框，输入密码后点击提交，此时拿到的password肯定不为空
     * 用户再次点击修改密码，弹出修改密码对话框，输入密码后点击取消，此处拿到的password就是空的
     * 如果直接将从修改密码对话框获取到的密码设置到 新密码中，那随着第二次的取消动作，就会使得一次
     * 修改的值也被取消了
    */
    const QString& password = dlg->getPassword();
    if(password.isEmpty()){
        LOG()<<"取消密码设置";
        return;
    }

    newPassword = password;
    LOG()<<"新密码已经设置: "<<password;

    // 隐藏修改密码按钮，显示已经修改
    ui->passwordBtn->hide();
    ui->passwordWidget->show();
    delete dlg;
}
