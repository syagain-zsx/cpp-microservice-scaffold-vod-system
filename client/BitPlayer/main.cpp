#include "bitplayer.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include "startuppage.h"
#include <QStyleFactory>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 不使用系统的默认样式，使用Fusion样式来创建元素
    a.setStyle(QStyleFactory::create("Fusion"));

    QSharedMemory sharedMemoty("BitPlayer");
    // 当程序第一次运行时，由于共享内存的空间还没有创建，此处attch一定会失败，即返回false
    // 当程序第二次运行时，由于共享内存的空间已经被前一次的运行申请好，此时第二个实例再去关联就能成功
    // 由于只能运行一个实例，让第二个实例直接退出，即让第一个进程结束
    if(sharedMemoty.attach())
    {
        QMessageBox::information(nullptr, "BitPlayer提示", "BitPlayer已经在运行...");
        return 0;
    }

    sharedMemoty.create(1);

    // 先显示启动页
    StartupPage startupPage;
    startupPage.startTimer();
    startupPage.exec();

    // 程序的主界面
    BitPlayer* bitPlayer = BitPlayer::getInstance();
    bitPlayer->show();
    return a.exec();
}
