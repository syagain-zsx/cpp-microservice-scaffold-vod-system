#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>

#define TEST_UI


// 从文件路径中提供文件名
static inline QString getFileName(const QString& filePath){
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}


// 封装日志宏
#define TAG QString("[%1:%2]").arg(getFileName(__FILE__), QString::number(__LINE__))

// 默认情况下，qDebug在输出字符串的时候会自动加上""， 可能有时候会干扰调试信息的可读性
// noquote()是QDebug中的一个成员函数，用于制定在输出时不会自动为字符串添加引号
#define LOG() qDebug().noquote()<<TAG

/*
 * qDebug()<<"输出信息";
 * LOG()<<"输出信息
*/



// 读文件操作：从指定文件中，读取所有的二进制内容，将其保存到QByteArray中
static inline QByteArray loadFileToByteArray(const QString& fileName){
    QFile file(fileName);
    bool ok = file.open(QFile::ReadOnly);
    if(!ok){
        LOG()<<"文件打开失败";
        return QByteArray();
    }

    QByteArray content = file.readAll();
    file.close();
    return content;
}

// 写文件操作：将QByteArray中的内容，写到指定的文件中
static inline void writeByteArrayToFile(const QString fileName, const QByteArray& content){
    QFile file(fileName);
    bool ok = file.open(QFile::WriteOnly);
    if(!ok){
        LOG()<<"打开文件失败!";
        return;
    }

    file.write(content);
    file.flush();
    file.close();
}

static inline QIcon makeIcon(const QByteArray& imageData){
    QPixmap pixmap;
    pixmap.loadFromData(imageData);
    QIcon icon(pixmap);
    return icon;
}

static inline QIcon makeCircleIcon(const QByteArray& imageData, int radius){
    QPixmap pixmap;
    pixmap.loadFromData(imageData);
    if(pixmap.isNull()){
        return QIcon();
    }

    // 图片：40*20-->宽高比：2:1，Qt::IgnoreAspectRatio作用：忽略缩放过程中的宽高比  60*60-->1:1
    // 此处忽略没有任何问题，因为头像18*18，现在想起放到到60*60，宽高比没有发生变化
    // Qt::SmoothTransformation: 平滑的转换算法，缩放之后的图片质量更高，缺陷：可能会牺牲一些速度
    pixmap = pixmap.scaled(radius*2, radius*2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


    // 将pixmap裁剪成原型的
    // 先要创建绘图设备(画布)--透明
    QPixmap output = QPixmap(pixmap.size());
    output.fill(Qt::transparent);   // 将绘图设备的背景设置成透明
    QPainter painter(&output);
    painter.setRenderHint(QPainter::Antialiasing);

    // 创建圆形裁剪路径
    QPainterPath path;
    path.addEllipse(0, 0, radius*2, radius*2);
    // 设置裁剪路径，裁剪路径的作用：限制绘图操作的范围，直摇头在裁剪路径内的区域才会被裁剪
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, pixmap);

    QIcon icon(output);
    return icon;
}

static QString intToString(int64_t value)
{
    if(value < 10000){
        return QString::number(value);
    }else{
        return QString::asprintf("%.1lf万", value/10000.0);
    }
}

static QString intToString2(int64_t value)
{
    if(value < 10000){
        return QString::number(value);
    }else{
        return QString::asprintf("%.2lfw", value/10000.0);
    }
}

static QString hidePhoneNumber(const QString& phoneNum)
{
    if(phoneNum.length() < 11){
        return phoneNum;
    }

    // 150****5678
    return phoneNum.left(3) + "****" + phoneNum.right(4);
}

// 569334855@qq.com ---> 569******@qq.com
static QString hideEmail(const QString& email)
{
    int pos = email.indexOf('@');

    // 150****5678
    return email.left(3) + QString(pos - 3, '*') + email.mid(pos);
}

static QString formatDate(const QString &dateStr)
{
    // 定义输入格式
    QString inputFormat = "yyyy-MMM-dd";

    // 定义输出格式
    QString outputFormat = "M-dd";

    // 使用 QDate::fromString 解析日期
    QDate date = QDate::fromString(dateStr, inputFormat);

    QString newDate;
    if (date.isValid()) {
        // 使用 toString 转换为新的格式
        newDate = date.toString(outputFormat);
        LOG()<<dateStr<<"-"<<newDate;
    } else {
        qDebug() << "Invalid date format";
    }

    return newDate;
}
#endif // UTIL_H



