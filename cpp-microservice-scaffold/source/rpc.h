#pragma once
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/channel.h>
#include <mutex>
#include <optional>

namespace biterpc {
using ChannelPtr = std::shared_ptr<brpc::Channel>;
class Channels {
    public:
        using ptr = std::shared_ptr<Channels>;
        Channels();
        //1. 新增节点
        void insert(const std::string &addr);
        //2. 删除节点
        void remove(const std::string &addr);
        //3. 获取节点信道
        ChannelPtr select();
        //4. 获取节点地址
        std::optional<std::string> selectAddr();
    private:
        std::mutex _mtx;
        std::string _svc_name; // 服务名称
        uint32_t _idx; //轮询下标
        std::vector<std::pair<std::string, ChannelPtr>> _channels;
};

class SvcChannels {
    public:
        using ptr = std::shared_ptr<SvcChannels>;
        SvcChannels() = default;
        void setWatch(const std::string &svc_name);
        void addNode(const std::string &svc_name, const std::string &addr);
        void delNode(const std::string &svc_name, const std::string &addr);
        ChannelPtr getNode(const std::string &svc_name);
        std::optional<std::string> getNodeAddr(const std::string &svc_name);
    private:
        Channels::ptr _Channels(const std::string &svc_name);
    private:
        std::mutex _mtx;
        std::unordered_map<std::string, Channels::ptr> _maps;
};

class ClosureFactory {
    public:
        using callback_t = std::function<void()>;
        static google::protobuf::Closure* create(callback_t &&cb);
    private:
        struct Object{
            using ptr = std::shared_ptr<Object>;
            callback_t callback;
        };
        static void asyncCallback(const Object::ptr obj);
};

class RpcServerFactory {
    public:
        // 默认svc是堆上new出来的对象，将管理权移交给rpc服务器进行管理
        static std::shared_ptr<brpc::Server> create(int port, google::protobuf::Service *svc);
};

}