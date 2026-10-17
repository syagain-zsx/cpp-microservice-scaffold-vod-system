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

void group_count(std::unique_ptr<odb::mysql::database> &handler) {
    try {
        odb::transaction tx(handler->begin());
        
        tx.tracer(odb::stderr_full_tracer);

        auto &db = tx.database();
        typedef odb::query<GroupCount> Query;
        typedef odb::result<GroupCount> Result;

        Result res(db.query<GroupCount>("avg_score>70"));
        for (auto it = res.begin(); it != res.end(); ++it) {
            std::cout << "班级:" << it->classes_id << "\t";
            std::cout << "总人数:" << it->total_stu << "\t";
            std::cout << "平均分:" << it->avg_score << std::endl;
        }
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
    group_count(handler);
    return 0;
}