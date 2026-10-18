#include "editadmindialog.h"
#include "ui_editadmindialog.h"
#include "bitplayer.h"
#include "./model/datacenter.h"
#include "toast.h"

EditAdminDialog::EditAdminDialog(QWidget *parent, const QString& text, model::AdminInfo& adminInfo)
    : QDialog(parent)
    , ui(new Ui::EditAdminDialog)
    , adminInfo(adminInfo)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->tittleLabel->setText(text);

    // 移动窗口位置,和主界面左上角对齐--BitPlayer
    BitPlayer* bitPlayer = BitPlayer::getInstance();
    this->move(bitPlayer->mapToGlobal(QPoint(0, 0)));

    // 对emailEdit中的邮箱进行校验
    QRegularExpression regExp(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QValidator* validator = new QRegularExpressionValidator(regExp, this);
    ui->emailEdit->setValidator(validator);

    // 设置管理员角色
    ui->roleComboBox->addItem("平台管理员");
    ui->roleComboBox->setCurrentIndex(0);
    ui->nameEdit->setMaxLength(16);

    // 设置管理员信息
    if(!adminInfo.userId.isEmpty()){
        // 编辑管理员信息
        ui->emailEdit->setText(adminInfo.email);
        // 目前角色只有一个
        if(model::RoleType::Admin == adminInfo.roleType){
            ui->roleComboBox->setCurrentIndex(0);
        }
        ui->nameEdit->setText(adminInfo.nickname);
        if(model::eable == adminInfo.userStatu){
            ui->startBtn->setChecked(true);
        }else{
            ui->stopBtn->setChecked(true);
        }
        ui->commentTextEdit->setPlainText(adminInfo.userMemo);
    }

    // 编辑框内容改变
    connect(ui->commentTextEdit, &QPlainTextEdit::textChanged, this, [=]{
        QString text = ui->commentTextEdit->toPlainText();
        int wordCount = text.length();
        if(wordCount > 10){
            wordCount = 10;
            ui->commentTextEdit->setPlainText(text.mid(0, 10));
        }
        ui->wordCount->setText(QString::number(wordCount) + "/10");

        QTextCursor textCursor =  ui->commentTextEdit->textCursor();
        textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->commentTextEdit->setTextCursor(textCursor);
    });

    connect(ui->submitBtn, &QPushButton::clicked, this, &EditAdminDialog::onSubmitBtnClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &EditAdminDialog::onCancelBtnClicked);
}

EditAdminDialog::~EditAdminDialog()
{
    delete ui;
}

bool EditAdminDialog::getCommitResult() const
{
    return isCommit;
}

void EditAdminDialog::setEmailEditReadOnly(bool isReadOnly)
{
    ui->emailEdit->setReadOnly(isReadOnly);
}

void EditAdminDialog::onSubmitBtnClicked()
{
    // 校验邮箱格式是否正确
    if(!ui->emailEdit->hasAcceptableInput()){
        Toast::showMessage("管理员邮箱格式错误!!!");
        return;
    }

    // 当用户点击提交时，将新增或编辑的管理员信息收集 adminInfo
    adminInfo.email = ui->emailEdit->text();

    // 角色
    adminInfo.roleType = model::RoleType::Admin;
    // 昵称
    adminInfo.nickname = ui->nameEdit->text();
    // 状态
    model::AdminStatus adminStatus = model::AdminStatus::eable;
    if(ui->stopBtn->isChecked()){
        adminStatus = model::AdminStatus::disable;
    }

    // 注意：禁止管理员自己修改自己
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->userId == adminInfo.userId && adminStatus != adminInfo.userStatu){
        Toast::showMessage("禁止管理员启用或禁用自己");
        return;
    }

    adminInfo.userStatu = adminStatus;
    adminInfo.userMemo = ui->commentTextEdit->toPlainText();
    isCommit = true;
    close();
}

void EditAdminDialog::onCancelBtnClicked()
{
    isCommit = false;
    close();
}
