#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QFile>


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

#endif // UTIL_H
