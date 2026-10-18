#include "playslider.h"
#include "ui_playslider.h"
#include <QMouseEvent>
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
    playGrogress = stepRatio*ui->inLine->width();
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
    // 更改outLine的宽度到playProgress即可
    ui->outLine->setGeometry(ui->outLine->x(), ui->outLine->y(), playGrogress, ui->outLine->height());
}
