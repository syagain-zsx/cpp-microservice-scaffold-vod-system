#include "bitplayerserver.h"

#include <QApplication>
#include "httpserver.h"
#include "util.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    HttpServer* httpServer = HttpServer::getInstance();
    if(!httpServer->init()){
        LOG()<<"Http服务器启动失败!";
        return -1;
    }

    LOG()<<"Http服务器启动成功";

    BitPlayerServer w;
    w.show();
    return a.exec();
}
