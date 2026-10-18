#include "toast.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include "util.h"

Toast::Toast(const QString& text)
{
    initUI(text);

    // 定义定时器:2s中之后窗口自动关闭
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]{
        timer->stop();
        this->close();
        this->deleteLater();
    });

    timer->start(2000);
}

Toast::Toast(const QString &text, QWidget *widget)
{
    initUI(text);

    // 定义定时器:2s中之后窗口自动关闭
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]{
        timer->stop();
        this->close();
        this->deleteLater();

        if(widget){
            widget->show();
        }
    });

    timer->start(2000);
}

void Toast::showMessage(const QString &text)
{
    Toast* toast = new Toast(text);
    toast->show();
}

void Toast::showMessage(const QString &text, QWidget *widget)
{
    Toast* toast = new Toast(text, widget);
    toast->show();
}

void Toast::initUI(const QString &text)
{
    // 1. 去除窗口标题，并将窗口设置为透明背景
    // Qt::Tool: 窗口浮动在主窗口上， 窗口图标也不会在任务栏显示
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(800, 60);

    // 2. 窗口添加背景
    QWidget* toastBg = new QWidget(this);
    toastBg->setFixedSize(800, 60);
    toastBg->setStyleSheet("background-color : rgba(102, 102, 102, 0.5);"
                           "border-radius : 4px");

    // 3. 界面设置
    QVBoxLayout* layout = new QVBoxLayout(toastBg);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    toastBg->setLayout(layout);

    // 创建QLabel显示文本，并将其添加到layout中
    QLabel* label = new QLabel(toastBg);
    label->setText(text);
    label->setAlignment(Qt::AlignCenter);
    // 设置控件的大小策略，Expanding：控件的大小会随着父窗口的大小变化而扩展
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setStyleSheet("font-family : 微软雅黑;"
                         "font-size : 14px;"
                         "color : #FFFFFF;");
    layout->addWidget(label);

    // 4. 设置Toast窗口的位置：尽量在屏幕的中间靠下的位置
    // 获取主屏幕
    QScreen* screen = QApplication::primaryScreen();
    int width = screen->size().width();
    int height = screen->size().height();
    int x = (width - this->width())/2;
    int y = height - this->height() - 100;
    LOG()<<width<<","<<height<<"-"<<x<<","<<y;
    this->move(x, y);
}


