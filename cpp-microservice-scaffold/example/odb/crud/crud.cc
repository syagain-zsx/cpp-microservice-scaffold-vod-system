#include <odb/database.hxx>
#include <odb/mysql/transaction.hxx>
#include <odb/mysql/database.hxx>

#include "student.h"
#include "student-odb.hxx"


const std::string HOST = "192.168.65.130";
const std::string USER = "root";
const std::string PASS = "123456";
const std::string DBNAME = "mytest";
const unsigned int PORT = 3306;
const std::string CSET = "utf8";

void insert(std::unique_ptr<odb::mysql::database> &handler) {
    try {
        //1. 通过句柄构造事务对象； 
        odb::transaction tx(handler->begin());
        //2. 通过事务对象构造database对象；
        auto &db = tx.database();
        //3. 数据操作
        Student stu("wanwu");
        stu.set_age(19);
        stu.set_birthday(boost::posix_time::time_from_string("2007-05-21 12:35:32"));
        db.persist(stu);
        //4. 提交事务
        tx.commit();
    }catch (odb::exception &e) {
        std::cout << "odb error: " << e.what() << std::endl;
    }
}

void select(std::unique_ptr<odb::mysql::database> &handler){
    try {
        //1. 通过句柄构造事务对象； 
        odb::transaction tx(handler->begin());
        //2. 通过事务对象构造database对象；
        auto &db = tx.database();
        //3. 数据操作
        typedef odb::query<Student> Query;
        auto stu = db.query_one<Student>(Query::sn == 2);
        if (!stu) {
            std::cout << "not found" << std::endl;
            return;
        }
        std::cout << "sn: " << stu->sn() << std::endl;
        std::cout << "name: " << stu->name() << std::endl;
        if (stu->age()) std::cout << "age: " << stu->age().get() << std::endl;
        if (stu->score()) std::cout << "score: " << stu->score().get() << std::endl;
        if (stu->birthday()) std::cout << "birthday: " << boost::posix_time::to_simple_string(stu->birthday().get()) << std::endl;
        //4. 提交事务
        tx.commit();
    }catch (odb::exception &e) {
        std::cout << "odb error: " << e.what() << std::endl;
    }
}

void update(std::unique_ptr<odb::mysql::database> &handler) {
    try {
        //1. 通过句柄构造事务对象； 
        odb::transaction tx(handler->begin());
        //2. 通过事务对象构造database对象；
        auto &db = tx.database();
        //3. 数据操作
        typedef odb::query<Student> Query;
        auto stu = db.query_one<Student>(Query::sn == 2);
        if (!stu) {
            std::cout << "not found" << std::endl;
            return;
        }
        stu->set_score(99.99);
        db.update(stu);
        //4. 提交事务
        tx.commit();
    }catch (odb::exception &e) {
        std::cout << "odb error: " << e.what() << std::endl;
    }
}

void select_all(std::unique_ptr<odb::mysql::database> &handler) {
    try {
        //1. 通过句柄构造事务对象； 
        odb::transaction tx(handler->begin());
        //2. 通过事务对象构造database对象；
        auto &db = tx.database();
        //3. 数据操作
        typedef odb::query<Student> Query;
        typedef odb::result<Student> Result;
        Result stus(db.query<Student>());
        for (auto it = stus.begin(); it != stus.end(); ++it) {
            std::cout << "sn: " << it->sn() << std::endl;
            std::cout << "name: " << it->name() << std::endl;
            if (it->age()) std::cout << "age: " << it->age().get() << std::endl;
            if (it->score()) std::cout << "score: " << it->score().get() << std::endl;
            if (it->birthday()) std::cout << "birthday: " << boost::posix_time::to_simple_string(it->birthday().get()) << std::endl;
        }
        //4. 提交事务
        tx.commit();
    }catch (odb::exception &e) {
        std::cout << "odb error: " << e.what() << std::endl;
    }
}

void remove(std::unique_ptr<odb::mysql::database> &handler) {
    try {
        //1. 通过句柄构造事务对象； 
        odb::transaction tx(handler->begin());
        //2. 通过事务对象构造database对象；
        auto &db = tx.database();
        //3. 数据操作
        typedef odb::query<Student> Query;
        db.erase_query<Student>(Query::sn == 3);
        //4. 提交事务
        tx.commit();
    }catch (odb::exception &e) {
        std::cout << "odb error: " << e.what() << std::endl;
    }
}

int main()
{
    // 1. 构造连接池
    std::unique_ptr<odb::mysql::connection_factory> pool(new odb::mysql::connection_pool_factory(5));
    // 2. 构造数据库操作句柄
    auto handler = std::make_unique<odb::mysql::database>(USER, PASS, DBNAME, HOST, PORT, nullptr, CSET, 0, std::move(pool));
    // 3. 数据操作 
    // insert(handler);
    // select(handler);
    // update(handler);
    // select_all(handler);
    remove(handler);
    return 0;
}