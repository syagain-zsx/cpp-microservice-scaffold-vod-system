#ifndef PAGEBUTTON_H
#define PAGEBUTTON_H

#include <QWidget>
#include <QPushButton>

class PageButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PageButton(int pageNo = 1, QWidget *parent = nullptr);
    void setActive(bool active);
    bool getActive()const;

    void setFolded(bool folded);
    bool getFolded()const;

    void setPage(int pageNo);
    int getPage()const;

private:
    int pageNo;    // 按钮上的页号
    bool isActiveBtn = false;   // 按钮是否被点击，默认未被选中
    bool isFoldedBtn = false;  // 按钮是否被折叠(...)，如果是折叠按钮，则不显示页号
};

#endif // PAGEBUTTON_H
