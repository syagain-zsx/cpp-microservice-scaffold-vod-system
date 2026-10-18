#ifndef PLAYSPEED_H
#define PLAYSPEED_H

#include <QWidget>

namespace Ui {
class PlaySpeed;
}

class PlaySpeed : public QWidget
{
    Q_OBJECT

public:
    explicit PlaySpeed(QWidget *parent = nullptr);
    ~PlaySpeed();

private slots:
    void onPlay20SpeedClicked();
    void onPlay15SpeedClicked();
    void onPlay10SpeedClicked();
    void onPlay05SpeedClicked();

signals:
    void setPlaySpeed(double speed);
private:
    void resetStyle(int speed);

private:
    Ui::PlaySpeed *ui;
};

#endif // PLAYSPEED_H
