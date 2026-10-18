#include "checktable.h"
#include "ui_checktable.h"
#include "checktableitem.h"
#include "./model/datacenter.h"
#include "toast.h"

CheckTable::CheckTable(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CheckTable)
{
    ui->setupUi(this);
    ui->videoStatus->addItem("全部分类");
    ui->videoStatus->addItem("待审核");
    ui->videoStatus->addItem("审核通过");
    ui->videoStatus->addItem("审核驳回");
    ui->videoStatus->addItem("已下架");
    //ui->videoStatus->addItem("转码中");
    ui->videoStatus->setCurrentIndex(0);

    paginator = new Paginator(0, ui->paginatorArea);
    paginator->move(0, 15);
    paginator->show();

    // 视频默认向上对齐
    ui->layout->setAlignment(Qt::AlignTop);

    // 给视频用户编辑框添加限制
    QRegularExpression regExp("^[0-9a-zA-Z]{20}$");
    QValidator* validator = new QRegularExpressionValidator(regExp, this);
    // 将正则表达式校验器设置到编辑框中，用户在userIdEdit中输入的内容就会被验证是否符合正则表达式的规则
    ui->userIdEdit->setMaxLength(20);
    ui->userIdEdit->setValidator(validator);

    connect(ui->resetBtn, &QPushButton::clicked, this, &CheckTable::onResetBtnClicked);
    connect(ui->queryBtn, &QPushButton::clicked, this, &CheckTable::onQueryBtnClicked);

    // 获取用户视频列表成功
    auto dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getUserVideoListDone, this, &CheckTable::updateCheckTable);

    // 获取状态视频列表成功
    connect(dataCenter, &model::DataCenter::getStatusVideoListDone, this, [=](){
        updateCheckTable("", "checkPage");
    });
}

CheckTable::~CheckTable()
{
    delete ui;
}

void CheckTable::updateCheckTable(const QString& userId, const QString whichPage)
{
    if("checkPage" != whichPage){
        return;
    }

    // 清空界面上的旧视频内容
    QLayoutItem* item = nullptr;
    while(nullptr != (item = ui->layout->takeAt(0))){
        delete item->widget();
        delete item;
    }

    // 将DataCenter中保存到获取到的视频列表中的视频更新到界面
    auto dataCenter = model::DataCenter::getInstance();
    auto statusVideoList = dataCenter->getStatusVideoList();
    if(nullptr == statusVideoList){
        return;
    }

    // 如果获取的是第一页的视频，需要根据该种条件下视频总数 以及 一个页显示的视频总数，需要重新计算分页器上总的页数
    int totalVideoCount = statusVideoList->getVideoTotalCount();
    int videoCountOfPage = model::VideoList::PAGE_COUNT;
    if(1 == page){
        // 视频总数：40   每页显示：20   页数：(40+20-1)/20 = 2
        // 视频总数：39   每页显示：20   页数：(39+20-1)/20 = 2
        resetPaginator((totalVideoCount+videoCountOfPage-1)/videoCountOfPage);
    }

    auto videoList = statusVideoList->videoInofs;
    for(int i = 0; i < videoList.size(); ++i){
        CheckTableItem* videoItem = new CheckTableItem(this, videoList[i]);
        ui->layout->addWidget(videoItem);
    }
}

void CheckTable::getVideoList(int page)
{
    this->page = page;

    // 先清空DataCenter中保存的原有的旧视频，然后再保存新page中的视频
    // 因为在视频审核页面，界面中使用都加载一页的视频，无需缓存其他页面的视频
    auto dataCenter = model::DataCenter::getInstance();
    auto statusVideoList = dataCenter->getStatusVideoList();
    //statusVideoList->clearVideoList();
    statusVideoList->videoInofs.clear();

    // 如果用户id编辑框不空，优先按照用户id获取视频列表
    // 如果用户id编辑框是空，再按照视频状态获取视频列表
    QString userId = ui->userIdEdit->text();
    if(!userId.isEmpty()){
        // 获取指定用户视频列表
        if(!ui->userIdEdit->hasAcceptableInput()){
            Toast::showMessage("上传视频用户id格式不对!!!");
            return;
        }
        // 获取所有状态
        dataCenter->getUserVideoListAsync(userId, page, model::noStatus, "checkPage");
    }else{
        // 获取状态视频列表
        dataCenter->getStatusVideoListAsync(videoStatus, page);
    }
}

void CheckTable::resetPaginator(int pageCount)
{
    // 因为总页数发生变化之后，分页器上显示的页数需要重新更新，因此可以先将旧的分页器销毁掉重新创建
    if(paginator){
        delete paginator;
    }

    paginator = new Paginator(pageCount, ui->paginatorArea);
    paginator->move(0, 15);
    paginator->show();

    connect(paginator, &Paginator::pageChanged, this, [=](int page){
        getVideoList(page);
    });
}

void CheckTable::onResetBtnClicked()
{
    // 设置按钮样式高亮：重置按钮高亮、查询按钮背景白色
    ui->resetBtn->setStyleSheet("background-color:#3ECEFF;"
                                "border-radius : 4px;"
                                "font-family : 微软雅黑;"
                                "font-size : 14px;"
                                "color : #FFFFFF;");

    ui->queryBtn->setStyleSheet("background-color : #FFFFFF;"
                                "border-radius : 4px;"
                                "border: 1px solid #DCDEE0;"
                                "font-family : 微软雅黑;"
                                "font-size : 14px;"
                                "color : #222222;");
    ui->userIdEdit->setText("");
    ui->videoStatus->setCurrentIndex(0);

    // 获取视频列表
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isAdminDisable()){
        Toast::showMessage("您已经被禁用，无法进行操作!!!");
    }else{
        getVideoList(1);
    }
}

void CheckTable::onQueryBtnClicked()
{
    // 设置按钮样式高亮：查询按钮高亮、重置按钮背景白色
    ui->queryBtn->setStyleSheet("background-color:#3ECEFF;"
                                "border-radius : 4px;"
                                "font-family : 微软雅黑;"
                                "font-size : 14px;"
                                "color : #FFFFFF;");

    ui->resetBtn->setStyleSheet("background-color : #FFFFFF;"
                                "border-radius : 4px;"
                                "border: 1px solid #DCDEE0;"
                                "font-family : 微软雅黑;"
                                "font-size : 14px;"
                                "color : #222222;");

    // 获取视频列表
    auto dataCenter = model::DataCenter::getInstance();
    auto myselfInfo = dataCenter->getMyselfInfo();
    if(myselfInfo->isAdminDisable()){
        Toast::showMessage("您已经被禁用，无法进行操作!!!");
    }else{
        videoStatus = static_cast<model::VideoStatus>(ui->videoStatus->currentIndex());
        getVideoList(1);
    }
}
