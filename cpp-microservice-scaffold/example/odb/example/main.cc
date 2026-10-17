#include <odb/database.hxx>
#include <odb/mysql/database.hxx>
#include <gflags/gflags.h>

#include "person.hxx"
#include "person-odb.hxx"

DEFINE_int32(msyql_port, 0, "mysql服务器端口");
DEFINE_string(msyql_host, "192.168.65.128", "mysql服务器地址");
DEFINE_string(msyql_db, "mytest", "mysql默认连接库名称");
DEFINE_string(msyql_user, "root", "mysql默认连接用户名");
DEFINE_string(msyql_passwd, "123456", "mysql默认连接密码");
DEFINE_int32(msyql_conn_pool, 3, "mysql连接池中连接最大数量");

class MysqlClient
{
public:
    static std::shared_ptr<odb::database> create(
        const std::string& user,
        const std::string& passwd,
        const std::string& db_name,
        const std::string& host,
        int port,
        size_t conn_pool_count) {
        //初始化连接池
        std::unique_ptr<odb::mysql::connection_factory> pool(
            new odb::mysql::connection_pool_factory(conn_pool_count));
        //构造mysql操作句柄
        std::shared_ptr<odb::database> db(new odb::mysql::database(
            user.c_str(), passwd.c_str(), db_name.c_str(), host.c_str(), port, 0, "utf8", 0, std::move(pool)));
        return db;
    }
    //开始事务，并返回事务对象
    static std::shared_ptr<odb::transaction> transaction(const std::shared_ptr<odb::database> &db) {
        return std::make_shared<odb::transaction>(db->begin());
    }
    //通过事务对象提交事务
    static void commit(const std::shared_ptr<odb::transaction> &t) {
        return t->commit();
    }
    //通过事务对象回滚事务
    static void rollback(const std::shared_ptr<odb::transaction> &t) {
        return t->rollback();
    }
};

class StudentDao {
    public:
        StudentDao(const std::string& user,
        const std::string& passwd,
        const std::string& db_name,
        const std::string& host,
        int port,
        size_t conn_pool_count):_db(MysqlClient::create(
            user, passwd, db_name, host, port, conn_pool_count))
        {}
        size_t append(Student& stu) {
            try{
                auto t = MysqlClient::transaction(_db);
                auto &db = t->database();
                db.persist(stu);
                MysqlClient::commit(t);
            } catch(std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            return stu.id();
        }
        void update(const Student& stu) {
            try{
                auto t = MysqlClient::transaction(_db);
                auto &db = t->database();
                db.update(stu);
                MysqlClient::commit(t);
            } catch(std::exception &e) {
                std::cout << e.what() << std::endl;
            }
        }
        void remove(const std::string &name) {
            try{
                auto t = MysqlClient::transaction(_db);
                auto &db = t->database();
                typedef odb::query<Student> query;
                db.erase_query<Student>(query::name == name);
                MysqlClient::commit(t);
            } catch(std::exception &e) {
                std::cout << e.what() << std::endl;
            }
        }
        std::vector<std::string> select(int n) {
            std::vector<std::string> res;
            try{
                auto t = MysqlClient::transaction(_db);
                auto &db = t->database();
                typedef odb::query<all_name> query;
                typedef odb::result<all_name> result;
                std::string cond = "order by age desc limit ";
                cond += std::to_string(n);
                result r(db.query<all_name>(cond));
                for (auto i(r.begin()); i != r.end(); ++i) {
                    res.push_back(i->name);
                }
                MysqlClient::commit(t);
            } catch(std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            return res;
        }
    private:
        std::shared_ptr<odb::database> _db;
};

class ClassesDao {
    public:
        ClassesDao(const std::string& user,
        const std::string& passwd,
        const std::string& db_name,
        const std::string& host,
        int port,
        size_t conn_pool_count):_db(MysqlClient::create(
            user, passwd, db_name, host, port, conn_pool_count))
        {}
        size_t append(const std::string &desc) {
            Classes c(desc);
            try{
                auto t = MysqlClient::transaction(_db);
                auto &db = t->database();
                db.persist(c);
                MysqlClient::commit(t);
            } catch(std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            return c.id();
        }
        std::vector<classes_student> select(const std::string &name) {
            std::vector<classes_student> res;
            try{
                auto t = MysqlClient::transaction(_db);
                auto &db = t->database();
                typedef odb::query<classes_student> query;
                typedef odb::result<classes_student> result;
                result r(db.query<classes_student>(query::classes::name == name));
                for (auto i(r.begin()); i != r.end(); ++i) {
                    res.push_back(*i);
                }
                MysqlClient::commit(t);
            } catch(std::exception &e) {
                std::cout << e.what() << std::endl;
            }
            return res;
        }
    private:
        std::shared_ptr<odb::database> _db;
};

int main(int argc, char *argv[])
{
    
    ClassesDao classes_tb(FLAGS_msyql_user, FLAGS_msyql_passwd, 
        FLAGS_msyql_db, FLAGS_msyql_host, 
        FLAGS_msyql_port, FLAGS_msyql_conn_pool);
    auto cid1 = classes_tb.append("一年级一班");
    auto cid2 = classes_tb.append("一年级二班");


    StudentDao student_tb(FLAGS_msyql_user, FLAGS_msyql_passwd, 
        FLAGS_msyql_db, FLAGS_msyql_host, 
        FLAGS_msyql_port, FLAGS_msyql_conn_pool);

    Student stu1(1, "张三", 14, cid1);
    Student stu2(2, "李四", 15, cid1);
    Student stu3(3, "王五", 12, cid1);
    Student stu4(4, "赵六", 13, cid2);
    Student stu5(5, "刘七", 14, cid2);
    student_tb.append(stu1);
    student_tb.append(stu2);
    student_tb.append(stu3);
    student_tb.append(stu4);
    student_tb.append(stu5);

    auto res = classes_tb.select("一年级一班");
    for (auto r : res) {
        std::cout << r.name << "\t" << r.classes_name << std::endl;
    }
    return 0;
}