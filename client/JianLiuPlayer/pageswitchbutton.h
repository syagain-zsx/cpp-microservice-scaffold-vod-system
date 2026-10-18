#ifndef PAGESWITCHBUTTON_H
#define PAGESWITCHBUTTON_H

#include <QPushButton>
#include <QLabel>

class PageSwitchButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PageSwitchButton(QWidget *parent = nullptr);

    void setImageAndText(const QString& imagePath, const QString& text, int pageId);
    void setTextColor(const QString& textColor);

    int getPageId()const;

    void setImage(const QString& imagePath);

private:
    void mousePressEvent(QMouseEvent *event);

private:
    QLabel* btnImage;   // 显示按钮上的图片
    QLabel* btnTittle;  // 显示按钮上的文本

    int pageId;

signals:
    void switchPage(int pageId);
};

#endif // PAGESWITCHBUTTON_H
