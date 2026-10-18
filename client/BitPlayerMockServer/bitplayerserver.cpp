#include "bitplayerserver.h"
#include "./ui_bitplayerserver.h"

BitPlayerServer::BitPlayerServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BitPlayerServer)
{
    ui->setupUi(this);
}

BitPlayerServer::~BitPlayerServer()
{
    delete ui;
}
