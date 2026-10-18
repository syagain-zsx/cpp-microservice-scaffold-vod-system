#include "roletable.h"
#include "ui_roletable.h"
#include "util.h"
#include "roletableitem.h"
#include "editadmindialog.h"
#include "./model/datacenter.h"
#include "toast.h"

RoleTable::RoleTable(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RoleTable)
{
    ui->setupUi(this);

    ui->userStatus->addItem("全部分类");
    ui->userStatus->addItem("启用");
    ui->userStatus->addItem("停用");
    ui->userStatus->setCurrentIndex(0);

    paginator = new Paginator(0, ui->paginatorArea);
    paginator->move(0, 15);
    paginator->show();

    ui->layout->setAlignment(Qt::AlignTop);

    // 限制编辑框只能输入邮箱
    QRegularExpression regExp(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QValidator* validator = new QRegularExpressionValidator(regExp, this);
    ui->email->setValidator(validator);

    initStyle();

    connect(ui->resetBtn, &QPushButton::clicked, this, &RoleTable::onResetBtnClicked);
    connect(ui->queryBtn, &QPushButton::clicked, this, &RoleTable::onQueryBtnClicked);
    connect(ui->insertBtn, &QPushButton::clicked, this, &RoleTable::onInsertBtnClicked);

    // 获取管理员信息--通过邮箱
    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getAdminInfoByEmailDone, this, &RoleTable::updateRoleTable);

    // 获取管理员信息失败--通过邮箱，邮箱不存在
    connect(dataCenter, &model::DataCenter::getAdminInfoByEmailFailed, this, [=]{
        auto adminList = dataCenter->getAdminList();
        adminList->clearAdminList();
        updateRoleTable();
    });

    // 获取管理员信息--通过状态
    connect(dataCenter, &model::DataCenter::getAdminInfoByStatusDone, this, &RoleTable::updateRoleTable);

    // 新增管理员
    connect(dataCenter, &model::DataCenter::newAdminDone, this, [=]{
        getAdminList(1);
    });

    // 修改管理员状态成功
    // connect(dataCenter, &model::DataCenter::setAdminStatusDone, this, [=]{
    //     getAdminList(page);
    // });

    // 删除管理员成功
    connect(dataCenter, &model::DataCenter::delAdminDone, this, [=]{
        getAdminList(1);
    });
}

RoleTable::~RoleTable()
{
    delete ui;
}

void RoleTable::initStyle()
{
    styleSheet.insert("选中","background-color:#3ECEFF;"
                              "border-radius : 4px;"
                              "font-family : 微软雅黑;"
                              "font-size : 14px;"
                              "color : #FFFFFF;");
    styleSheet.insert("未选中", "background-color : #FFFFFF;"
                                "border-radius : 4px;"
                                "border: 1px solid #DCDEE0;"
                                "font-family : 微软雅黑;"
                                "font-size : 14px;"
                                "color : #222222;");
}

void RoleTable::updateRoleTable()
{
    // 1. 清空界面中旧的管理员信息
    QLayoutItem* item = nullptr;
    while(nullptr != (item = ui->layout->takeAt(0))){
        delete item->widget();
        delete item;
    }

    // 2. 获取管理员列表
    auto dataCenter = model::DataCenter::getInstance();
    auto adminListPtr = dataCenter->getAdminList();
    if(nullptr == adminListPtr){
        return;
    }

    // 3. 重置分页器
    auto adminList = adminListPtr->adminInfos;
    int adminCountPage = model::AdminList::PAGE_COUNT;
    int totalCount = adminListPtr->totalCount;
    if(1 == page){
        resetPaginator((totalCount+adminCountPage-1)/adminCountPage);
    }

    // 4. 将管理员信息添加到界面中
    int seqNum = (page - 1)*adminCountPage + 1;
    for(int i = 0; i < adminList.size(); ++i){
        RoleTableItem* roleTableItem = new RoleTableItem(this, adminList[i], seqNum++);
        ui->layout->addWidget(roleTableItem);
    }
}

void RoleTable::getAdminList(int page)
{
    this->page = page;
    auto dataCenter = model::DataCenter::getInstance();
    auto adminList = dataCenter->getAdminList();
    adminList->adminInfos.clear();

    // 如果邮箱不空，优先按照邮箱获取管理员信息，否则按照状态获取
    QString email = ui->email->text();
    if(!email.isEmpty()){
        if(!ui->email->hasAcceptableInput()){
            Toast::showMessage("输入邮箱有误!!!");
            return;
        }

        dataCenter->getAdminInfoByEmailAsync(email);
    }else{
        // 通过状态获取管理员信息
        dataCenter->getAdminInfoByStatusAsync(page, adminStatus);
    }
}

void RoleTable::resetPaginator(int pageCount)
{
    if(paginator){
        delete paginator;
    }

    paginator = new Paginator(pageCount, ui->paginatorArea);
    paginator->move(0, 15);
    paginator->show();

    // 分页器信号
    connect(paginator, &Paginator::pageChanged, this, [=](int page){
        getAdminList(page);
    });
}

void RoleTable::onResetBtnClicked()
{
    // 让重置按钮高亮，查询按钮非高亮
    ui->resetBtn->setStyleSheet(styleSheet["选中"]);
    ui->queryBtn->setStyleSheet(styleSheet["未选中"]);
    ui->email->setText("");
    ui->userStatus->setCurrentIndex(0);

    // 获取管理员列表
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isAdminDisable()){
        Toast::showMessage("您已被禁止，无法进行操作!!!");
    }else{
        getAdminList(1);
    }
}

void RoleTable::onQueryBtnClicked()
{
    // 让重置按钮非高亮，查询按钮高亮
    ui->resetBtn->setStyleSheet(styleSheet["未选中"]);
    ui->queryBtn->setStyleSheet(styleSheet["选中"]);

    // 获取管理员列表
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isAdminDisable()){
        Toast::showMessage("您已被禁止，无法进行操作!!!");
    }else{
        LOG()<<"查询按钮点击";
        adminStatus = static_cast<model::AdminStatus>(ui->userStatus->currentIndex());
        getAdminList(1);
    }
}

void RoleTable::onInsertBtnClicked()
{
    model::AdminInfo adminInfo;
    EditAdminDialog* editAdminDlg = new EditAdminDialog(nullptr, "新增后台用户", adminInfo);
    editAdminDlg->setEmailEditReadOnly(false);
    editAdminDlg->exec();
    if(editAdminDlg->getCommitResult()){
        // 如果用户点击了提交按钮，需要向服务器发送新增管理员信息的请求
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->newAdminAsync(adminInfo);
    }

    delete editAdminDlg;
}
