#include "paginator.h"
#include <QHBoxLayout>
#include <QLabel>

Paginator::Paginator(int pageCount, QWidget *parent)
    : QWidget{parent}
{
    this->pageCount = pageCount;

    // 设置页面尺寸
    setMinimumSize(1270, 32);

    // 创建水平布局器
    QHBoxLayout* layout = new QHBoxLayout();
    this->setLayout(layout);
    layout->setContentsMargins(0, 0, 3, 0);
    layout->setSpacing(8);

    // 创建上一页按钮
    prevPageBtn = new QPushButton();
    prevPageBtn->setIcon(QIcon(":/images/admin/arrow-left.png"));
    setBtnStyle(prevPageBtn);

    // 创建下一页按钮
    nextPageBtn = new QPushButton();
    nextPageBtn->setIcon(QIcon(":/images/admin/arrow-right.png"));
    setBtnStyle(nextPageBtn);

    // 创建中间页面按钮
    if(pageCount <= 7){
        // 页面数小于等于7，不需要折叠按钮，默认从第一页开始显示
        for(int i = 1; i <= pageCount; ++i){
            PageButton* pageBtn = new PageButton(i);
            if(1 == i){
                pageBtn->setActive(true);
            }

            pageBtns.append(pageBtn);
        }
    }else{
        // 如果超过7个页面，前5个按钮上文本显示页号
        // 第6个按钮设置为折叠按钮
        // 第7个按钮上的文本显示总页数 pageCount
        // 默认选中也一个页面按钮
        for(int i = 1; i <= 5; ++i){
            PageButton* pageBtn = new PageButton(i);
            if(1 == i){
                pageBtn->setActive(true);
            }

            pageBtns.append(pageBtn);
        }

        // 折叠按钮
        PageButton* foldedBtn = new PageButton(0);
        foldedBtn->setFolded(true);
        pageBtns.push_back(foldedBtn);

        // 添加最后一页的按钮
        PageButton* lastPageBtn = new PageButton(pageCount);
        pageBtns.push_back(lastPageBtn);
    }

    // 创建输入页号编辑框
    pageEdit = new QLineEdit();
    pageEdit->setFixedSize(48, 32);
    pageEdit->setAlignment(Qt::AlignCenter);
    pageEdit->setStyleSheet("QLineEdit{"
                            "background-color : #FFFFFF;"
                            "border : 1px solid #D9D9D9;"
                            "border-radius : 2px;}");

    // 调至 和 页的QLabel
    QLabel* jumpToLabel = new QLabel("跳至");
    QLabel* pageLabel = new QLabel("页");

    // 将元素添加到布局器中
    layout->addStretch(); // 在左侧添加空白区域
    layout->addWidget(prevPageBtn);
    for(int i = 0; i < pageBtns.size(); ++i){
        layout->addWidget(pageBtns[i]);
    }
    layout->addWidget(nextPageBtn);
    layout->addWidget(jumpToLabel);
    layout->addWidget(pageEdit);
    layout->addWidget(pageLabel);

    initSignalAndSlots();
}

void Paginator::setBtnStyle(QPushButton *btn)
{
    btn->setFixedSize(32, 32);
    btn->setIconSize(QSize(12, 12));
    btn->setStyleSheet("QPushButton{"
                       "color : #000000;"
                       "background-color : #FFFFFF;"
                       "border: 1px solid #D9D9D9;"
                       "border-radius : 2px;}");
}

void Paginator::initSignalAndSlots()
{
    // 绑定上一页 下一页按钮信号槽
    connect(prevPageBtn, &QPushButton::clicked, this, &Paginator::prevPage);
    connect(nextPageBtn, &QPushButton::clicked, this, &Paginator::nextPage);

    // 绑定 页面按钮 信号槽
    for(auto& pageBtn : pageBtns){
        connect(pageBtn, &QPushButton::clicked, this, &Paginator::pageBtnClicked);
    }

    // 绑定 编辑框 信号槽：编辑框输入完成之后，按回车
    connect(pageEdit, &QLineEdit::returnPressed, this, [=](){
        int pageNo = pageEdit->text().toInt();
        if(pageNo < 1){
            pageNo = 1;
        }
        if(pageNo > pageCount){
            pageNo = pageCount;
        }
        jumpToPage(pageNo);
    });
}

void Paginator::jumpToPage(int pageNo)
{
    if(pageNo < 1 || pageNo > pageCount){
        return;
    }

    if(pageCount <= 7){
        // 总页数不超过7，直接将pageNo设置为激活按钮，此时不需要折叠按钮
        for(auto& pageBtn : pageBtns){
            pageBtn->setActive(false);
        }

        // pageNo是从1开始的，而PageBtns的下标是从0开始的
        pageBtns[pageNo-1]->setActive(true);
        currentPage = pageNo;
    }else{
        if(pageNo <= 5){
            // 情况一，查看前5个页，有一个折叠按钮 6
            jumpToPageCase1(pageNo);
        }else if(pageNo >= pageCount - 4){
            // 情况二： 查看后5页，有一个折叠按钮 2
            jumpToPageCase2(pageNo);
        }else{
            // 情况三：查看中间页--有两个折叠按钮2 6
            jumpToPageCase3(pageNo);
        }
    }

    emit pageChanged(pageNo);
}

void Paginator::jumpToPageCase1(int pageNo)
{
    // page <= 5 的情况
    // 固定7个按钮，前5个按钮显示1~5的页号
    // 第6个按钮为折叠按钮，但是页号为6，
    // 第7个按钮显示最大页号
    // 将pageNo设置为激活按钮
    pageBtns[0]->setPage(1);
    pageBtns[1]->setPage(2);
    pageBtns[2]->setPage(3);
    pageBtns[3]->setPage(4);
    pageBtns[4]->setPage(5);
    pageBtns[5]->setPage(6);
    pageBtns[6]->setPage(pageCount);

    // 处理激活状态和折叠状态
    for(int i = 0; i < pageBtns.size(); ++i){
        // 设置当前显示页号对应的按钮为激活按钮
        if(pageBtns[i]->getPage() == pageNo){
            pageBtns[i]->setActive(true);
        }else{
            pageBtns[i]->setActive(false);
        }

        // 将第6个按钮设置为激活按钮,实际为数组中下标为5的按钮
        if(5 == i){
            pageBtns[i]->setFolded(true);
        }else{
            pageBtns[i]->setFolded(false);
        }
    }
}

void Paginator::jumpToPageCase2(int pageNo)
{
    // page >= pageCount - 4的情况
    // 固定7个按钮，第1个显示第1页
    // 第二个按钮为折叠按钮，其页号为page-5
    // 其余按钮上的页号从左往右依次为page-4递增
    pageBtns[0]->setPage(1);
    pageBtns[1]->setPage(pageCount-5);
    pageBtns[2]->setPage(pageCount-4);
    pageBtns[3]->setPage(pageCount-3);
    pageBtns[4]->setPage(pageCount-2);
    pageBtns[5]->setPage(pageCount-1);
    pageBtns[6]->setPage(pageCount);

    // 处理激活和折叠状态
    for(int i = 0; i < pageBtns.size(); ++i){
        // 处理激活按钮
        if(pageBtns[i]->getPage() == pageNo){
            pageBtns[i]->setActive(true);
        }else{
            pageBtns[i]->setActive(false);
        }

        // 处理折叠按钮
        if(1 == i){
            pageBtns[i]->setFolded(true);
        }else{
            pageBtns[i]->setFolded(false);
        }
    }
}

void Paginator::jumpToPageCase3(int pageNo)
{
    // 第一个按钮显示1页，第7个按钮显示最后一页 pageCount
    // 将中间按钮设置为当前显示页pageNo，并将其设置为激活状态
    // 将第2个和第6个页面按钮设置为折叠状态
    // 除第1个和第7个按钮外，其余按钮上的页号从左往右依次为：pageNo-2 pageNo-1 pageNo  pageNo+1 pageNo-1
    // 设置按钮上的页号
    pageBtns[0]->setPage(1);
    pageBtns[1]->setPage(pageNo-2);
    pageBtns[2]->setPage(pageNo - 1);
    pageBtns[3]->setPage(pageNo);
    pageBtns[4]->setPage(pageNo+1);
    pageBtns[5]->setPage(pageNo+2);
    pageBtns[6]->setPage(pageCount);

    // 处理激活和折叠状态
    for(int i = 0; i < pageBtns.size(); ++i){
        // 处理激活状态，第3个按钮
        if(3 == i){
            pageBtns[i]->setActive(true);
        }else{
            pageBtns[i]->setActive(false);
        }

        // 处理折叠状态：2 6
        if(1 == i || 5 == i){
            pageBtns[i]->setFolded(true);
        }else{
            pageBtns[i]->setFolded(false);
        }
    }
}

void Paginator::prevPage()
{
    // 当前显示的刚好是第一页
    if(1 == currentPage){
        return;
    }

    jumpToPage(currentPage - 1);
}

void Paginator::nextPage()
{
    // 当前显示的刚好是最后一页
    if(currentPage == pageCount){
        return;
    }

    jumpToPage(currentPage + 1);
}

void Paginator::pageBtnClicked()
{
    // 获取触发该信号的按钮
    // 肯定是有按钮点击了才进入到该函数 sender(): 可以获取该槽函数对应信号的触发者
    int pageNo = static_cast<PageButton*>(sender())->getPage();
    jumpToPage(pageNo);
}

