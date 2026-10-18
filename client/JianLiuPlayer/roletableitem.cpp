#include "roletableitem.h"
#include "ui_roletableitem.h"
#include "editadmindialog.h"
#include "./model/datacenter.h"
#include "toast.h"


RoleTableItem::RoleTableItem(QWidget *parent, const model::AdminInfo& adminInfo, int seqNumber)
    : QWidget(parent)
    , ui(new Ui::RoleTableItem)
    , adminInfo(adminInfo)
{
    ui->setupUi(this);

    // 初始化样式表
    initStyleSheet();

    updateUI(seqNumber);

    connect(ui->editBtn, &QPushButton::clicked, this, &RoleTableItem::onEditBtnClicked);
    connect(ui->statusButton, &QPushButton::clicked, this, &RoleTableItem::onStatusBtnClicked);
    connect(ui->delBtn, &QPushButton::clicked, this, &RoleTableItem::onDeleteBtnClicked);

    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::editAdminDone, this, [=](const QString& adminId){
        ui->emailLabel->setText(this->adminInfo.email);
        ui->nameLabel->setText(this->adminInfo.nickname);
        // 状态
        if(model::eable == this->adminInfo.userStatu){
            // 启用
            ui->statusButton->setText("启用");
        }else if(model::disable == this->adminInfo.userStatu){
            // 禁用
            ui->statusButton->setText("禁用");
        }

        // 修改状态按钮的样式
        ui->statusButton->setStyleSheet(styleSheet[ui->statusButton->text()]);

        ui->commentLabel->setText(this->adminInfo.userMemo);

        // 该信号触发，说明管理员信息已经将其同步到服务器
        // 本地Datacenter中保存的管理员信息页需要同步过来
        auto adminLists = dataCenter->getAdminList()->adminInfos;
        for(auto& admin : adminLists){
            if(admin.userId == adminId){
                admin.nickname = this->adminInfo.nickname;
                admin.userStatu = this->adminInfo.userStatu;
                admin.userMemo = this->adminInfo.userMemo;
                break;
            }
        }
    });
}

RoleTableItem::~RoleTableItem()
{
    delete ui;
}

void RoleTableItem::updateUI(int seqNumber)
{
    // 序号
    ui->idLabel->setText(QString::number(seqNumber));
    // 用户角色
    if(model::Admin == adminInfo.roleType){
        // 普通管理员
        ui->roleLabel->setText("平台管理员");
    }else if(model::SuperAdmin == adminInfo.roleType){
        // 超级管理员 ： 没有编辑和删除的操作，只显示一个按钮，状态按钮的点击操作禁用
        ui->delBtn->hide();
        ui->editBtn->setText("--");
        ui->editBtn->move(1145, 25);
        ui->editBtn->setEnabled(false);
        ui->statusButton->setEnabled(false);
        ui->editBtn->setStyleSheet(styleSheet["--"]);
    }

    // 邮箱
    ui->emailLabel->setText(adminInfo.email);
    // 用户昵称
    ui->nameLabel->setText(adminInfo.nickname);
    // 状态
    if(model::eable == adminInfo.userStatu){
        // 启用
        ui->statusButton->setText("启用");
    }else if(model::disable == adminInfo.userStatu){
        // 禁用
        ui->statusButton->setText("禁用");
    }

    // 修改状态按钮的样式
    ui->statusButton->setStyleSheet(styleSheet[ui->statusButton->text()]);
    // 备注
    ui->commentLabel->setText(adminInfo.userMemo);
    ui->statusButton->setEnabled(false);
}

void RoleTableItem::initStyleSheet()
{
    styleSheet.insert("启用", "#statusButton{"
                              "border : none;"
                              "background-color : #EBF3FF;"
                              "border-radius : 10px;"
                              "border : 1px solid #EBEDF0;"
                              "font-size : 12px;"
                              "color : #3686FF;"
                              "}");

    styleSheet.insert("禁用", "#statusButton{"
                              "border : none;"
                              "background-color : #FFF0F0;"
                              "border-radius : 10px;"
                              "border : 1px solid #EBEDF0;"
                              "font-size : 12px;"
                              "color : #FD6A6A;"
                              "}");

    styleSheet.insert("--", "#editBtn{"
                              "border : none;"
                              "background-color : #FFFFFF;"
                              "border-radius : 10px;"
                              "font-size : 12px;"
                              "color : #222222;"
                              "}");
}

bool RoleTableItem::logUserIsAdminSelf(const QString &text)
{
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->userId == adminInfo.userId){
        Toast::showMessage(text);
        return true;
    }

    return false;
}

void RoleTableItem::onEditBtnClicked()
{
    EditAdminDialog* editAdminDlg = new EditAdminDialog(nullptr, "编辑后台用户", adminInfo);
    editAdminDlg->setEmailEditReadOnly(true);
    editAdminDlg->exec();
    // 如果用户点击了提交按钮，向服务器发送编辑用户请求
    if(editAdminDlg->getCommitResult()){
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->editAdminAsync(adminInfo);

        // 管理员列表中昵称
        auto adminLists = dataCenter->getAdminList()->adminInfos;
        for(auto& admin : adminLists){
            if(admin.userId == adminInfo.userId){
                admin.nickname = adminInfo.nickname;
                break;
            }
        }
    }

    delete editAdminDlg;
}

void RoleTableItem::onStatusBtnClicked()
{
    // 注意：管理员不能自己禁用或启用自己 也不能修改超级管理员的状态
    if(logUserIsAdminSelf("管理员不能自己启用或禁用自己")){
        return;
    }

    // 平台管理员不能修改超级管理员的状态
    if(model::RoleType::SuperAdmin == adminInfo.roleType){
        Toast::showMessage("不能启用或禁用超级管理员");
        return;
    }

    QString statusBtnText = ui->statusButton->text();
    if("启用" == statusBtnText){
        adminInfo.userStatu = model::AdminStatus::eable;
        statusBtnText = "禁用";
    }else{
        adminInfo.userStatu = model::AdminStatus::disable;
        statusBtnText = "启用";
    }

    ui->statusButton->setText(statusBtnText);
    ui->statusButton->setStyleSheet(styleSheet[statusBtnText]);

    auto dataCenter = model::DataCenter::getInstance();
    auto adminListPtr = dataCenter->getAdminList();
    adminListPtr->setAdminStatus(adminInfo.userId, adminInfo.userStatu);

    // 修改管理员状态
    dataCenter->setAdminStatusAsync(adminInfo);
}

void RoleTableItem::onDeleteBtnClicked()
{
    // 要删除的管理员是否为当前登录用户自己：自己不能删除自己
    if(logUserIsAdminSelf("禁止自己删除自己")){
        return;
    }

    // 平台管理员不能删除超级管理员的状态
    if(model::RoleType::SuperAdmin == adminInfo.roleType){
        Toast::showMessage("不能删除超级管理员");
        return;
    }

    // 给服务器发送删除管理员的请求
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->delAdminAsync(adminInfo.userId);
}




