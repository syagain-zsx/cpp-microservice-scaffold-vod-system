#include <butil/logging.h>
#include <brpc/server.h>
#include "cal.pb.h"

class CalServiceImpl : public cal::CalService {
    public:
        CalServiceImpl() {}
        ~CalServiceImpl() {}
        void Add(::google::protobuf::RpcController* controller,
            const ::cal::AddReq* request,
            ::cal::AddRsp* response,
            ::google::protobuf::Closure* done) override {
            //当done_guard被释放的时候执行done->Run()来完成本次rpc调用
            brpc::ClosureGuard done_guard(done); 
            int result = request->num1() + request->num2();
            response->set_result(result);
        }
};

int main(int argc, char *argv[])
{
    // 1. 实例化计算服务对象
    CalServiceImpl cal_service;
    // 2. 定义服务器配置对象
    brpc::ServerOptions options;
    options.idle_timeout_sec = -1;
    // 3. 实例化服务器对象 
    brpc::Server server;
    // 4. 向服务器添加服务
    int ret = server.AddService(&cal_service, brpc::SERVER_DOESNT_OWN_SERVICE);
    if (ret == -1) {
        std::cout << "AddService failed" << std::endl;
        return -1;
    }
    // 4. 启动服务器
    ret = server.Start(9000, &options);
    if (ret == -1) {
        std::cout << "Start failed" << std::endl;
        return -1;
    }
    // 5. 等待服务器停止
    server.RunUntilAskedToQuit();
    return 0;
}
