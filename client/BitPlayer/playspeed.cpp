#include "playspeed.h"
#include "ui_playspeed.h"

PlaySpeed::PlaySpeed(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaySpeed)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 默认正常速度播放
    ui->speed10->setStyleSheet("background-color : rgba(255, 255, 255, 0.41);");

    connect(ui->speed20, &QPushButton::clicked, this, &PlaySpeed::onPlay20SpeedClicked);
    connect(ui->speed15, &QPushButton::clicked, this, &PlaySpeed::onPlay15SpeedClicked);
    connect(ui->speed10, &QPushButton::clicked, this, &PlaySpeed::onPlay10SpeedClicked);
    connect(ui->speed05, &QPushButton::clicked, this, &PlaySpeed::onPlay05SpeedClicked);
}

PlaySpeed::~PlaySpeed()
{
    delete ui;
}

void PlaySpeed::onPlay20SpeedClicked()
{
    resetStyle(20);
    emit setPlaySpeed(2.0);
}

void PlaySpeed::onPlay15SpeedClicked()
{
    resetStyle(15);
    emit setPlaySpeed(1.5);
}

void PlaySpeed::onPlay10SpeedClicked()
{
    resetStyle(10);
    emit setPlaySpeed(1.0);
}

void PlaySpeed::onPlay05SpeedClicked()
{
    resetStyle(5);
    emit setPlaySpeed(0.5);
}

void PlaySpeed::resetStyle(int speed)
{
    ui->speed20->setStyleSheet("");
    ui->speed15->setStyleSheet("");
    ui->speed10->setStyleSheet("");
    ui->speed05->setStyleSheet("");

    switch(speed){
    case 20:
        ui->speed20->setStyleSheet("background-color : rgba(255, 255, 255, 0.41);");
        break;
    case 15:
        ui->speed15->setStyleSheet("background-color : rgba(255, 255, 255, 0.41);");
        break;
    case 10:
        ui->speed10->setStyleSheet("background-color : rgba(255, 255, 255, 0.41);");
        break;
    case 5:
        ui->speed05->setStyleSheet("background-color : rgba(255, 255, 255, 0.41);");
        break;
    }
}
