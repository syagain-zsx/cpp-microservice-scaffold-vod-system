#include <brpc/channel.h>
#include "cal.pb.h"

using callback_t = std::function<void()>;
struct Object {
    callback_t callback;
};
void Callback(const Object obj) {
    obj.callback();
}

int main(int argc, char *argv[])
{
    // 0. 实例化ChannelOptions进行参数配置
    brpc::ChannelOptions options;
    options.protocol = "baidu_std";
    // 1. 实例化Channel信道对象--
    brpc::Channel chennel;
    chennel.Init("192.168.65.130:9000", &options);
    // 3. 实例化CalService_stub对象--用于发起rpc请求。
    cal::CalService_Stub stub(&chennel);
    brpc::Controller *cntl = new brpc::Controller;
    cntl->set_timeout_ms(4000);
    cal::AddReq *request = new cal::AddReq;
    cal::AddRsp *response = new cal::AddRsp;
    request->set_num1(10);
    request->set_num2(20);
    // google::protobuf::Closure* closure = brpc::NewCallback(Callback, cntl, request, response);
    Object obj;
    obj.callback = [=]() {
        std::unique_ptr<brpc::Controller> cntl_guard(cntl);
        std::unique_ptr<cal::AddReq> req_guard(request);
        std::unique_ptr<cal::AddRsp> rsp_guard(response);
        if (cntl_guard->Failed() == true) {
            std::cout << "rpc请求失败: " << cntl_guard->ErrorText() << std::endl;
            return ;
        }
        std::cout << rsp_guard->result() << std::endl;
    };
    google::protobuf::Closure* closure = brpc::NewCallback(Callback, obj);
    stub.Add(cntl, request, response, closure);
    std::cout << "===============================\n";
    getchar();
    return 0;
}