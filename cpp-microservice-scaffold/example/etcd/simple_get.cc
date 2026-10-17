/*
    编写样例获取etcd服务器上的name字段的数据
        1. 实例化客户端对象
        2. 获取数据
        3. 输出数据
        4. 进行数据监控 -- 编写数据改变的回调处理函数， 长连接异常的处理回调函数
*/
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>

void callback(const etcd::Response &resp) {
    if (resp.is_ok() == false) {
        std::cout << "监控出错: " << resp.error_message() << std::endl;
        return;
    }
    const auto &events = resp.events();
    for (auto &e : events) {
        if (e.event_type() == etcd::Event::EventType::PUT) {
            std::cout << e.prev_kv().key() << "数据改变:" << e.prev_kv().as_string() << "->" << e.kv().as_string() << std::endl;
        }else if (e.event_type() == etcd::Event::EventType::DELETE_) {
            std::cout << e.prev_kv().key() << "数据删除!" << std::endl;
        }else {
            std::cout << "无效类型\n";
        }
    }
}

int main()
{
    const std::string url = "http://192.168.65.130:2379";
    etcd::Client  client(url);
    auto resp = client.get("name").get();
    if (resp.is_ok() == false) {
        std::cout << "获取数据失败: " << resp.error_message() << std::endl;
        return -1;
    }
    std::cout << resp.value().as_string() << std::endl;

    //数据监控
    etcd::Watcher watcher(url, "name", callback, false);
    watcher.Wait();
    return 0;
}