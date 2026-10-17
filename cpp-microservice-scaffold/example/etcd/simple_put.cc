/*
    编写样例代码，向etcd服务器添加键值对数据
     1. 实例化客户端对象
     2. 创建租约
     3. 向服务器添加数据，并设置租约ID
     4. 实例化保活对象，对租约进行保活
*/

#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>

int main()
{
    const std::string url = "http://192.168.65.130:2379";
    //1. 实例化客户端对象
    etcd::Client  client(url);
    //2. 创建租约
    auto lease_resp = client.leasegrant(3).get();
    if (lease_resp.is_ok() == false) {
        std::cout << "创建租约失败: " << lease_resp.error_message() << std::endl;
        return -1;
    }
    //2.5 获取租约ID
    auto lease_id = lease_resp.value().lease();
    //3. 添加数据，并设置租约
    auto resp = client.put("name", "xiaoming", lease_id).get();
    if (resp.is_ok() == false) {
        std::cout << "添加数据失败: " << resp.error_message() << std::endl;
        return -1;
    }
    auto handler = [](const std::exception_ptr &eptr){
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch(const std::runtime_error& e) {
            std::cerr << "Connection failure \"" << e.what() << "\"\n";
        } catch(const std::out_of_range& e) {
            std::cerr << "Lease expiry \"" << e.what() << "\"\n";
        }
    };
    //4. 租约保活
    etcd::KeepAlive alive(url, handler, 3, lease_id);
    std::cout << "回车退出，租约失效...\n";
    getchar();
    return 0;
}