#include "homepagewidget.h"
#include "ui_homepagewidget.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include "util.h"
#include "videobox.h"
#include "./model/datacenter.h"

HomePageWidget::HomePageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePageWidget)
{
    ui->setupUi(this);

    initKindsAndTags();

    initRefreshAndTopBtn();

    connectSignalAndSlot();

    initVideos();
}

HomePageWidget::~HomePageWidget()
{
    delete ui;
}

void HomePageWidget::initKindsAndTags()
{
    // 分类
    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    auto kinds = kindAndTagPtr->getAllKinds();

    // 创建分类按钮
    QPushButton* kindBtn = buildSelectBtn(ui->classifys, "#3ECEFF", "分类");
    kindBtn->setObjectName("kindBtn");
    ui->classifyHLayout->addWidget(kindBtn);
    connect(kindBtn, &QPushButton::clicked, this, &HomePageWidget::onKindsBtnClicked);

    // 具体的分类按钮
    for(auto& kind : kinds){
        QPushButton* kindItemBtn = buildSelectBtn(ui->classifys, "#222222", kind);
        ui->classifyHLayout->addWidget(kindItemBtn);

        // 给分类按钮绑定槽函数
        connect(kindItemBtn, &QPushButton::clicked, this, [=]{
            onKindBtnClicked(kindItemBtn);
        });
    }

    ui->classifyHLayout->setSpacing(8);
}

QPushButton* HomePageWidget::buildSelectBtn(QWidget *parent, const QString &color, const QString &text)
{
    QPushButton* btn = new QPushButton(text, parent);
    btn->setStyleSheet("color : " + color);
    btn->setMinimumHeight(26);
    // 18:文本到按钮边界的距离
    btn->setFixedWidth(text.size()*16+18+18);
    return btn;
}

void HomePageWidget::resetTags(const QList<QString> &tags)
{
    // 创建标签的说明
    QPushButton* tag = buildSelectBtn(ui->labels, "#3ECEFF", "标签");
    tag->setObjectName("tagBtn");
    ui->labelHLayout->addWidget(tag);
    connect(tag, &QPushButton::clicked, this, &HomePageWidget::onTagsBtnClicked);

    // 创建具体每个标签
    for(auto& tagText : tags){
        QPushButton* tagItem = buildSelectBtn(ui->labels, "#666666", tagText);
        ui->labelHLayout->addWidget(tagItem);

        connect(tagItem, &QPushButton::clicked, this, [=]{
            onTagBtnClicked(tagItem);
        });
    }

    ui->labelHLayout->setSpacing(4);
}

void HomePageWidget::initRefreshAndTopBtn()
{
    // 先创建一个QWidget
    QWidget* refreshTopWidget = new QWidget(this);
    refreshTopWidget->setFixedSize(42, 94);
    refreshTopWidget->setStyleSheet("QPushButton{"
                                    "border-radius : 21px;"
                                    "border : none;"
                                    "background-color : rgba(221, 221, 221, 0.4);}"
                                    "QPushButton:hover{background-color : rgba(102, 102, 102, 0.4);}");
    QVBoxLayout* layout = new QVBoxLayout(refreshTopWidget);

    // 创建按钮
    QPushButton* topBtn = new QPushButton(refreshTopWidget);
    topBtn->setFixedSize(42, 42);
    topBtn->setStyleSheet("border-image : url(:/images/homePage/zhiding.png);");
    layout->addWidget(topBtn);

    QPushButton* refreshBtn = new QPushButton(refreshTopWidget);
    refreshBtn->setFixedSize(42, 42);
    refreshBtn->setStyleSheet("border-image : url(:/images/homePage/shuaxin.png);");
    layout->addWidget(refreshBtn);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    refreshTopWidget->move(1278, 618);

    connect(topBtn, &QPushButton::clicked, this, &HomePageWidget::onTopBtnClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &HomePageWidget::onRefreshBtnClicked);
}

void HomePageWidget::initVideos()
{
    // QGrideLayout中的视频以左上角对齐
    ui->videoGLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->videoScroll->verticalScrollBar()->setValue(0);

    // 初始情况下，应该从服务器获取所有视频列表中的 第一页的视频，默认获取20个视频
    // 默认获取的是所有视频列表
    videoListStyle = AllStyle;
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->getAllVideoListAsync();
}

void HomePageWidget::connectSignalAndSlot()
{
    auto dataCenter = model::DataCenter::getInstance();
    // 获取所有视频列表完成信号槽绑定
    connect(dataCenter, &model::DataCenter::getAllVideoListDone, this, &HomePageWidget::updateVideoList);

    // 获取分类视频列表完成信号槽绑定
    connect(dataCenter, &model::DataCenter::getAllVideoListInKindDone, this, &HomePageWidget::updateVideoList);

    // 获取标签视频列表完成信号槽绑定
    connect(dataCenter, &model::DataCenter::getAllVideoListInTagDone, this, &HomePageWidget::updateVideoList);

    // 搜索框信号槽绑定
    connect(ui->search, &SearchLineEdit::searchVideos, this, &HomePageWidget::onSearchVideos);

    // 获取搜索视频列表完成信号槽绑定
    connect(dataCenter, &model::DataCenter::getAllVideoListSearchTextDone, this, &HomePageWidget::updateVideoList);

    // QScrollArea中QScrollBar的垂直滚动条发生变化的信号槽绑定
    connect(ui->videoScroll->verticalScrollBar(), &QScrollBar::valueChanged, this, &HomePageWidget::OnScrollBarValueChanged);
}

void HomePageWidget::clearLayoutVidoes()
{
    // 清空界面QGideLayout中的所有视频
    QLayoutItem* videoItem = nullptr;
    while((videoItem = ui->videoGLayout->takeAt(0)) != nullptr){
        delete videoItem->widget();
        delete videoItem;
    }

    // 注意：还需要清空DataCenter中保存的旧数据
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->getVideoListPtr()->clearVideoList();

    // 视频清空之后，将滚动条恢复到最上面
    ui->videoScroll->verticalScrollBar()->setValue(0);
}

void HomePageWidget::onKindBtnClicked(QPushButton *clickedKindBtn)
{
    // 如果相同分类按钮连续点击，将不再重新获取
    const QString kindText = clickedKindBtn->text();
    if(curKind == kindText){
        return;
    }

    curKind = kindText;
    curTag = "";

    // 设置当前被点击分类按钮的高亮显示
    clickedKindBtn->setStyleSheet("background-color : #F1FDFF;"
                                  "color : #3ECEFF;");

    // 清楚之前点击过的分类按钮上的高亮显示
    // 获取所有的分类按钮--即获取classifys中所有类型为QPushButton*的孩子.因为所有分类按钮的父元素都是ui->classifys
    QList<QPushButton*> kindBtns = ui->classifys->findChildren<QPushButton*>();
    for(auto& kindBtn : kindBtns){
        if(kindBtn != clickedKindBtn){
            kindBtn->setStyleSheet("color : #222222;");
        }
    }

    // 先删除分类布局器中的之前分类按钮下中的所有标签
    QList<QPushButton*> tagBtns = ui->labels->findChildren<QPushButton*>();
    for(auto& tagBtn : tagBtns){
        ui->labelHLayout->removeWidget(tagBtn);
        delete tagBtn;
    }

    // 重新添加当前点击分类按钮对应的所有标签
    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    resetTags(kindAndTagPtr->getTagsByKind(kindText).keys());

    // 界面上可能已经有了其他条件查询之后的视频数据
    // 再获取分类视频数据时，先将其他条件的视频清空掉
    clearLayoutVidoes();

    // 从服务器中获取该分类下的所有视频，此时和标签无关
    videoListStyle = KindStyle;
    dataCenter->getAllVideoListInKindAsync(kindAndTagPtr->getKindId(clickedKindBtn->text()));
}

void HomePageWidget::onTagBtnClicked(QPushButton *clickedTagBtn)
{
    // 防止相同的标签按钮重复点击
    QString tagText = clickedTagBtn->text();
    if(curTag == tagText){
        return;
    }

    curTag = tagText;

    // 将被点击的标签按钮设置为高亮显示
    clickedTagBtn->setStyleSheet("background-color : #F1FDFF;"
                                 "color : #3ECEFF;");

    // 获取当前分类下的所有标签，并清除之前点击过的标签上的高亮显示
    QList<QPushButton*> tagBtns = ui->labels->findChildren<QPushButton*>();
    for(auto& tagBtn : tagBtns){
        if(tagBtn != clickedTagBtn){
            tagBtn->setStyleSheet("color : #666666;");
        }
    }

    // 回去标签视频之前，需要先请求界面上旧的视频数据
    clearLayoutVidoes();

    // 到服务器获取该标签下的所有视频
    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    // 注意：curKind可能是空字符串
    auto kinds = kindAndTagPtr->getAllKinds();
    if(curKind.isEmpty()){
        curKind = kinds[0];
    }
    videoListStyle = TagStyle;
    dataCenter->getAllVideoListInTagAsync(kindAndTagPtr->getTagId(curKind, curTag));
}

void HomePageWidget::onKindsBtnClicked()
{
    // 只展示分类按钮不展示标签按钮，将所有标签按钮删除掉
    QList<QPushButton*> tagBtns = ui->labels->findChildren<QPushButton*>();
    for(auto& tagBtn : tagBtns){
        ui->labelHLayout->removeWidget(tagBtn);
        delete tagBtn;
    }

    // 先清空界面上的旧视频数据
    clearLayoutVidoes();

    // 将所有分类按钮样式设置为未点击样式
    QList<QPushButton*> kindBtns = ui->classifys->findChildren<QPushButton*>();
    for(auto& kindBtn : kindBtns){
        kindBtn->setStyleSheet("color : #222222;");
    }
    // 让文本为分类的按钮高亮
    QPushButton* kind = ui->classifys->findChild<QPushButton*>("kindBtn");
    kind->setStyleSheet("background-color : #F1FDFF;"
                        "color : #3ECEFF;");

    // 分类按钮点击，获取所有视频列表
    curKind = "";
    curTag = "";
    videoListStyle = AllStyle;
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->getAllVideoListAsync();
}

void HomePageWidget::onTagsBtnClicked()
{
    // 文本为标签的按钮点击，获取该标签所在分类下的是有视频列表
    // 先清空界面上的旧视频数据
    clearLayoutVidoes();

    // 重置所有标签按钮的样式，选中文本为标签的按钮
    QList<QPushButton*> tagBtns = ui->labels->findChildren<QPushButton*>();
    for(auto& tagBtn : tagBtns){
        tagBtn->setStyleSheet("color : #666666;");
    }
    QPushButton* tag = ui->labels->findChild<QPushButton*>("tagBtn");
    tag->setStyleSheet("background-color : #F1FDFF;"
                       "color : #3ECEFF;");

    // 获取分类视频列表
    curTag = "";
    videoListStyle = KindStyle;
    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    dataCenter->getAllVideoListInKindAsync(kindAndTagPtr->getKindId(curKind));
}

void HomePageWidget::onTopBtnClicked()
{
    // 将垂直滚动条设置到开始即可
    ui->videoScroll->verticalScrollBar()->setValue(0);
}

void HomePageWidget::onRefreshBtnClicked()
{
    // 获取之前先将界面上旧的视频内容清空掉
    clearLayoutVidoes();

    // 重新到服务器上去获取视频
    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    switch (videoListStyle) {
    case AllStyle:
        dataCenter->getAllVideoListAsync();
        break;
    case KindStyle:
        dataCenter->getAllVideoListInKindAsync(kindAndTagPtr->getKindId(curKind));
        break;
    case TagStyle:
        dataCenter->getAllVideoListInTagAsync(kindAndTagPtr->getTagId(curKind, curTag));
        break;
    case SearchStyle:
        dataCenter->getAllVideoListSearchTextAsync(ui->search->text());
        break;
    default:
        LOG()<<"暂不支持的视频列表";
        break;
    }
}

void HomePageWidget::updateVideoList()
{
    // 获取视频列表
    auto dataCenter = model::DataCenter::getInstance();
    auto videoList = dataCenter->getVideoListPtr()->getVideoList();
    LOG()<<"从服务器获取"<<videoList.size()<<"个视频";
    for(int i = ui->videoGLayout->count(); i < videoList.size(); ++i){
        // 构建视频显示框VideoBox
        VideoBox* videoBox = new VideoBox(videoList[i],this);
        ui->videoGLayout->addWidget(videoBox, i/4, i%4);
    }
    LOG()<<"添加到layout中的视频个数为："<<ui->videoGLayout->count();
}

void HomePageWidget::onSearchVideos(const QString &searchText)
{
    // 先清空界面上的旧视频数据
    clearLayoutVidoes();

    // 清空分类和标签的标记
    curKind = "";
    curTag = "";

    // 重置所有分类按钮状态，并选中文本为分类的按钮
    QList<QPushButton*> kindBtns = ui->classifys->findChildren<QPushButton*>();
    for(auto& kindBtn : kindBtns){
        kindBtn->setStyleSheet("color : #222222;");
    }
    QPushButton* kind = ui->classifys->findChild<QPushButton*>("kindBtn");
    kind->setStyleSheet("background-color : #F1FDFF;"
                        "color : #3ECEFF");

    // 重置所有标签按钮状态，并选中文本为标签的按钮
    QList<QPushButton*> tagBtns = ui->labels->findChildren<QPushButton*>();
    for(auto& tagBtn : tagBtns){
        tagBtn->setStyleSheet("color : #666666;");
    }
    QPushButton* tag = ui->labels->findChild<QPushButton*>("tagBtn");
    tag->setStyleSheet("background-color : #F1FDFF;"
                        "color : #3ECEFF");

    // 向服务器获取搜索视频列表
    videoListStyle = SearchStyle;
    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->getAllVideoListSearchTextAsync(searchText);
}

void HomePageWidget::OnScrollBarValueChanged(int value)
{
    // 如果该分类下的所有视频已经全部获取了，就不用再去获取了
    auto dataCenter = model::DataCenter::getInstance();
    auto videoListPtr = dataCenter->getVideoListPtr();
    if(videoListPtr->getVideoTotalCount() == videoListPtr->getVideoCount()){
        return;
    }

    // 当垂直滚动条滑动到最下面时，自动获取下一页视频
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    if(value == ui->videoScroll->verticalScrollBar()->maximum()){
        switch (videoListStyle) {
        case AllStyle:
            dataCenter->getAllVideoListAsync();
            break;
        case KindStyle:
            dataCenter->getAllVideoListInKindAsync(kindAndTagPtr->getKindId(curKind));
            break;
        case TagStyle:
            dataCenter->getAllVideoListInTagAsync(kindAndTagPtr->getTagId(curKind, curTag));
            break;
        case SearchStyle:
            dataCenter->getAllVideoListSearchTextAsync(ui->search->text());
            break;
        default:
            LOG()<<"暂不支持的视频列表";
            break;
        }
    }
}











