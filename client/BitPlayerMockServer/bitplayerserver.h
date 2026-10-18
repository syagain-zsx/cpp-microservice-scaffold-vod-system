#ifndef BITPLAYERSERVER_H
#define BITPLAYERSERVER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class BitPlayerServer;
}
QT_END_NAMESPACE

class BitPlayerServer : public QWidget
{
    Q_OBJECT

public:
    BitPlayerServer(QWidget *parent = nullptr);
    ~BitPlayerServer();

private:
    Ui::BitPlayerServer *ui;
};
#endif // BITPLAYERSERVER_H
