#include "pageswitchbutton.h"
#include "util.h"


PageSwitchButton::PageSwitchButton(QWidget *parent)
    : QPushButton{parent}
{
    // 设置按钮的大小
    setFixedSize(48, 46);

    btnImage = new QLabel(this);
    btnImage->setGeometry((48-24)/2, 0, 24, 24);

    btnTittle = new QLabel(this);
    btnTittle->setGeometry(0, 30, 48, 16);
    btnTittle->setAlignment(Qt::AlignCenter);

    // 去掉按钮的边框
    setStyleSheet("border:none;");
}

void PageSwitchButton::setImageAndText(const QString &imagePath, const QString &text, int pageId)
{
    setImage(imagePath);
    btnTittle->setText(text);
    this->pageId = pageId;
}

void PageSwitchButton::setTextColor(const QString &textColor)
{
    btnTittle->setStyleSheet("font-family : 微软雅黑;"
                             "font-size : 12px;"
                             "font-weight : bold;"
                             "color : " + textColor + ";");
}

int PageSwitchButton::getPageId() const
{
    return pageId;
}

void PageSwitchButton::setImage(const QString &imagePath)
{
    btnImage->setPixmap(QPixmap(imagePath));
}

void PageSwitchButton::mousePressEvent(QMouseEvent *event)
{
    (void)event;
    setTextColor("#000000");
    emit switchPage(pageId);
    LOG()<<pageId<<"按钮被点击了";
}
