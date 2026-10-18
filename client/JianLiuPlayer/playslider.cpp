#include "playslider.h"
#include "ui_playslider.h"
#include <QMouseEvent>
#include <cmath>
#include "util.h"

PlaySlider::PlaySlider(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaySlider)
{
    ui->setupUi(this);
    playGrogress = 0;
}

PlaySlider::~PlaySlider()
{
    delete ui;
}

void PlaySlider::setPlayStep(double stepRatio)
{
    // 防御：视频时长为 0 时调用方可能算出 inf/nan，直接赋给 int 属于未定义行为
    // （在 Windows 上会把进度条宽度设成极端值，进而导致界面异常甚至崩溃）
    if(!std::isfinite(stepRatio) || stepRatio < 0){
        stepRatio = 0;
    }
    if(stepRatio > 1.0){
        stepRatio = 1.0;
    }
    playGrogress = static_cast<int>(stepRatio*ui->inLine->width());
    moveSlider();
}

void PlaySlider::mousePressEvent(QMouseEvent *event)
{
    // 当鼠标左键按下时，需要记录当前鼠标的x位置，即播放当前进度
    if(event->button() == Qt::LeftButton){
        playGrogress = event->pos().x();
        moveSlider();
        return;
    }

    QWidget::mousePressEvent(event);
}

void PlaySlider::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        playGrogress = event->pos().x();
        moveSlider();

        emit setPlayProgress((double)playGrogress/ui->inLine->width());
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void PlaySlider::mouseMoveEvent(QMouseEvent *event)
{
    // 检测鼠标的位置是否在播放进度条窗口中
    if(!this->geometry().contains(event->pos())){
        LOG()<<event->pos();
        return;
    }

    // 鼠标的位置在进度条窗口中
    if(event->buttons() == Qt::LeftButton){
        playGrogress = event->pos().x();

        if(playGrogress < 0){
            playGrogress = 0;
        }

        int maxWidth = this->width();
        if(playGrogress > maxWidth){
            playGrogress = maxWidth;
        }

        moveSlider();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void PlaySlider::moveSlider()
{
    // 更改outLine的宽度到playProgress即可（再做一次范围保护，避免负数/超宽）
    int width = playGrogress;
    if(width < 0){
        width = 0;
    }
    if(width > ui->inLine->width()){
        width = ui->inLine->width();
    }
    ui->outLine->setGeometry(ui->outLine->x(), ui->outLine->y(), width, ui->outLine->height());
}
