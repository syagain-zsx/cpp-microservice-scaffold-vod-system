#include "jianliumockserver.h"
#include "./ui_jianliumockserver.h"

JianLiuMockServer::JianLiuMockServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JianLiuMockServer)
{
    ui->setupUi(this);
}

JianLiuMockServer::~JianLiuMockServer()
{
    delete ui;
}
