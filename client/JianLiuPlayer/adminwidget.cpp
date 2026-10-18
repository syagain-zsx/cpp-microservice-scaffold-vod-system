#include "adminwidget.h"
#include "ui_adminwidget.h"

AdminWidget::AdminWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminWidget)
{
    ui->setupUi(this);

    onCheckBtnClicked();

    connect(ui->checkBtn, &QPushButton::clicked, this, &AdminWidget::onCheckBtnClicked);
    connect(ui->roleBtn, &QPushButton::clicked, this, &AdminWidget::onRoleBtnClicked);
}

AdminWidget::~AdminWidget()
{
    delete ui;
}

void AdminWidget::onCheckBtnClicked()
{
    ui->checkBtn->setStyleSheet(activeTabStyle);
    ui->roleBtn->setStyleSheet(inactiveTabStyle);
    ui->checkTable->show();
    ui->roleTable->hide();
}

void AdminWidget::onRoleBtnClicked()
{
    ui->checkBtn->setStyleSheet(inactiveTabStyle);
    ui->roleBtn->setStyleSheet(activeTabStyle);
    ui->checkTable->hide();
    ui->roleTable->show();
}
