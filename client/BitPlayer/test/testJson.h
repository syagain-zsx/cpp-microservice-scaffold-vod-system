#ifndef TESTJSON_H
#define TESTJSON_H
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "./../util.h"

/*
 * // 一个学生对象的JSon表示
{
    "name" : "张三",
    "age" : 18,
    "gender" : "男",
    "hobby" : ["篮球", "足球" , "编程"],
    "score" : {
        "C语言" : 82,
        "C++" : 76,
        "数据结构" : 80
    }
}
*/

QByteArray serialize()
{
    QJsonObject student;
    student.insert("name", "张三");
    student.insert("age", 18);
    student["gender"] = "男";

    // 创建hobby的json数组
    QJsonArray hobby;
    hobby.append("篮球");
    hobby.append("足球");
    hobby.append("编程");
    student.insert("hobby", hobby);

    // 构造score的json对象
    QJsonObject score;
    score.insert("C语言", 82);
    score.insert("C++", 76);
    score.insert("数据结构", 80);
    student.insert("score", score);

    LOG()<<"student Json对象中键值对的个数："<<student.count();
    LOG()<<student;

    // 对student的json对象进行序列化
    QJsonDocument jsonDoc(student);
    return jsonDoc.toJson();
}

void Deserialize(QByteArray studentArray)
{
    // 对 studentArray进行发序列化
    QJsonDocument jsonDoc = QJsonDocument::fromJson(studentArray);
    QJsonObject student = jsonDoc.object();
    LOG()<<student;

    // 解析Json对象：name age gender
    LOG()<<"name : "<<student["name"].toString();
    LOG()<<"age : "<<student["age"].toInt();
    LOG()<<"gender : "<<student["gender"].toString();

    // 解析hobby，hobby是Json数组
    QJsonArray hobby = student["hobby"].toArray();
    LOG()<<"hobby :";
    for(int i = 0; i < hobby.count(); ++i){
        LOG()<<hobby[i].toString();
    }

    // 解析score，score是JSon对象
    QJsonObject score = student["score"].toObject();
    LOG()<<"score : ";
    LOG()<<"C语言 : "<<score["C语言"].toInt();
    LOG()<<"C++ : "<<score["C++"].toInt();
    LOG()<<"数据结构 : "<<score["数据结构"].toInt();
}
#endif // TESTJSON_H






