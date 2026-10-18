#include "pagebutton.h"

PageButton::PageButton(int pageNo, QWidget *parent)
    : QPushButton{parent}
{
    this->pageNo = pageNo;

    // 设置按钮的尺寸以及图标的尺寸
    setFixedSize(32, 32);
    setIconSize(QSize(16, 16));

    // 设置按钮上的文本
    if(isFoldedBtn){
        setText("...");
    }else{
        setText(QString::number(pageNo));
    }

    // 设置按钮的选中状态
    setActive(isActiveBtn);
}

void PageButton::setActive(bool active)
{
    this->isActiveBtn = active;
    if(active){
        setStyleSheet("QPushButton{"
                      "color : #FFFFFF;"
                      "background-color : #3ECEFE;"
                      "border : 1px solid #3ECEFE;"
                      "border-radius : 2px;}");
    }else{
        setStyleSheet("QPushButton{"
                      "color : #000000;"
                      "background-color : #FFFFFF;"
                      "border : 1px solid #D9D9D9;"
                      "border-radius : 2px;}");
    }
}

bool PageButton::getActive() const
{
    return isActiveBtn;
}

void PageButton::setFolded(bool folded)
{
    this->isFoldedBtn = folded;
    if(isFoldedBtn){
        setText("...");
    }else{
        setText(QString::number(pageNo));
    }
}

bool PageButton::getFolded() const
{
    return isFoldedBtn;
}

void PageButton::setPage(int pageNo)
{
    this->pageNo = pageNo;
    setText(QString::number(pageNo));
}

int PageButton::getPage() const
{
    return pageNo;
}
