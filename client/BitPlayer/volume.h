#ifndef VOLUME_H
#define VOLUME_H

#include <QWidget>

namespace Ui {
class Volume;
}

class Volume : public QWidget
{
    Q_OBJECT

public:
    explicit Volume(QWidget *parent = nullptr);
    ~Volume();
    int getVolume()const;

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void calcVolume();

signals:
    void setVolume(int volume);

private:
    Ui::Volume *ui;
    int volume = 34;
};

#endif // VOLUME_H
