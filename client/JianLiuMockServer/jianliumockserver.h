#ifndef JIANLIUPLAYERSERVER_H
#define JIANLIUPLAYERSERVER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class JianLiuMockServer;
}
QT_END_NAMESPACE

class JianLiuMockServer : public QWidget
{
    Q_OBJECT

public:
    JianLiuMockServer(QWidget *parent = nullptr);
    ~JianLiuMockServer();

private:
    Ui::JianLiuMockServer *ui;
};
#endif // JIANLIUPLAYERSERVER_H
