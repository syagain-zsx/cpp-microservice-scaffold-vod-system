#ifndef PLAYSLIDER_H
#define PLAYSLIDER_H

#include <QWidget>

namespace Ui {
class PlaySlider;
}

class PlaySlider : public QWidget
{
    Q_OBJECT

public:
    explicit PlaySlider(QWidget *parent = nullptr);
    ~PlaySlider();
    void setPlayStep(double stepRatio);

private:
    // 重写基类中的鼠标按下、释放、移动事件
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void moveSlider();

signals:
    void setPlayProgress(double playRatio);

private:
    Ui::PlaySlider *ui;
    int playGrogress;      // 记录视频的当前播放进度
};

#endif // PLAYSLIDER_H
