#include "etcd.h"
#include "log.h"
#include "util.h"
#include <thread>
#include <sstream>
#include <chrono>

namespace JianLiuSvc {


void wait_for_connection(etcd::Client &client) {
    // wait until the client connects to etcd server
    while (!client.head().get().is_ok()) {
        WRN("连接etcd服务器失败...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

SvcProvider::SvcProvider(const std::string &reg_center_addr,
    const std::string &svc_name, const std::string &svc_addr)
    : _reg_center_addr(reg_center_addr)
    , _instance_id(JianLiuUtil::Random::code())
    , _svc_name(svc_name)
    , _svc_addr(svc_addr) {}
std::string SvcProvider::make_key() {
    std::stringstream ss;
    ss << "/" << _svc_name << "/" << _instance_id;
    return ss.str();
}
// bool ret = registry("user", "192.168.65.130:9000");
// -> key = /user/instance_id;  val = 192.168.65.130:9000
bool SvcProvider::registry() {
    //1. 实例化etcd客户端对象
    etcd::Client  client(_reg_center_addr);
    wait_for_connection(client); //等待连接服务器成功
    //2. 创建10s租约（3s 在负载高时容易过期，导致服务在注册中心反复上下线）
    auto lease_resp = client.leasegrant(10).get();
    if (lease_resp.is_ok() == false) {
        ERR("创建租约失败: {}", lease_resp.error_message());
        return false;
    }
    auto lease_id = lease_resp.value().lease();
    //3. 重组key值, 根据租约向服务器添加数据
    auto resp = client.put(make_key(), _svc_addr, lease_id).get();
    if (resp.is_ok() == false) {
        ERR("添加数据失败: {}", lease_resp.error_message());
        return false;
    }
    DBG("服务注册成功: {} = {}", make_key(), _svc_addr);
    //5. 实例化保活对象，对租约进行保活
    auto handler = [this](const std::exception_ptr &eptr){
        WRN("etcd 租约保活失败，重新注册服务: {}", _svc_name);
        this->registry();
    };
    _keepalive.reset(new etcd::KeepAlive(_reg_center_addr, handler, 10, lease_id));
    return true;
}

SvcWatcher::SvcWatcher(const std::string &reg_center_addr,
    SvcWatcher::ModCallbck &&online_callback, 
    SvcWatcher::ModCallbck &&offline_callback)
    : _reg_center_addr(reg_center_addr)
    , _online_callback(std::move(online_callback))
    , _offline_callback(std::move(offline_callback)){ }
    
std::string SvcWatcher::parse_key(const std::string &key) {
    // /user/instance_id
    std::vector<std::string> arry;
    JianLiuUtil::STR::split(key, "/", arry);
    return arry[0];
}
void SvcWatcher::callback(const etcd::Response &resp) {
    if (resp.is_ok() == false) {
        ERR("监控出错: {}", resp.error_message());
        return;
    }
    const auto &events = resp.events();
    for (auto &e : events) {
        if (e.event_type() == etcd::Event::EventType::PUT) {
            std::string svc_name = parse_key(e.kv().key());
            std::string svc_addr = e.kv().as_string();
            DBG(" {} 服务上线节点: {}", svc_name, svc_addr);
            if (_online_callback) _online_callback(svc_name, svc_addr);
        }else if (e.event_type() == etcd::Event::EventType::DELETE_) {
            std::string svc_name = parse_key(e.prev_kv().key());
            std::string svc_addr = e.prev_kv().as_string();
            DBG(" {} 服务下线节点: {}", svc_name, svc_addr);
            if (_offline_callback) _offline_callback(svc_name, svc_addr);
        }else {
            WRN("无效通知类型");
        }
    }
}
// 数据监控并不针对单独数据，而是针对 / 目录
bool SvcWatcher::watch() {
    //1. 先实例化etcd客户端对象，并等待连接成功
    etcd::Client  client(_reg_center_addr);
    wait_for_connection(client); //等待连接服务器成功
    //2. 先建立 watch，再拉取全量：避免「ls 之后、watch 建立之前」注册的服务丢失
    auto cb = std::bind(&SvcWatcher::callback, this, std::placeholders::_1);
    _watcher.reset( new etcd::Watcher(_reg_center_addr, "/", cb, true)); //true表示递归监控目录下所有数据
    _watcher->Wait([this](bool cond) {
        if (cond == true) { return; }
        this->watch();
    });
    //3. 浏览根目录，获取当前所有已经提供服务的节点信息（与 watch 事件重复的部分由 addNode 去重）
    auto resp = client.ls("/").get();
    if (resp.is_ok() == true) {
        auto values = resp.values(); //获取结果数据
        for (auto &v : values) {
            std::string svc_name = parse_key(v.key());
            std::string svc_addr = v.as_string();
            DBG("已有 {} 节点: {}", svc_name, svc_addr);
            if (_online_callback) _online_callback(svc_name, svc_addr);
        }
    }
    //4. 周期性全量同步：修复链路抖动漏掉的事件，以及「同地址的新旧实例」下线误删
    if (!_resync_thread.joinable()) {
        _resync_thread = std::thread([this]() {
            while (!_resync_stop) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                if (_resync_stop) {
                    break;
                }
                this->resync();
            }
        });
    }
    return true;
}

void SvcWatcher::resync() {
    try {
        etcd::Client client(_reg_center_addr);
        auto resp = client.ls("/").get();
        if (resp.is_ok() == false) {
            WRN("周期性同步服务列表失败: {}", resp.error_message());
            return;
        }
        for (auto &v : resp.values()) {
            if (_online_callback) {
                _online_callback(parse_key(v.key()), v.as_string());
            }
        }
    } catch (const std::exception &e) {
        WRN("周期性同步服务列表异常: {}", e.what());
    }
}

SvcWatcher::~SvcWatcher() {
    _resync_stop = true;
    if (_resync_thread.joinable()) {
        _resync_thread.join();
    }
}

}