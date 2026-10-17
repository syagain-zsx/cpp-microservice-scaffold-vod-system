/*
    gflags使用
        1.包头文件<gflags/gflags.h>
        2.定义要捕获的信息，以及设置默认值
        3，解析参数，初始化参数数据
        4，访问参数
*/

#include <gflags/gflags.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

#include "child.h"

//定义参数：类型、名字、默认值、帮助信息
DEFINE_bool(reuse_addr, true, "是否使用地址重用选项");
DEFINE_int32(listen_port, 8080, "服务器监听端口号");
DEFINE_string(listen_ip, "0.0.0.0", "服务器监听IP地址");
DEFINE_double(pi, 3.14159, "圆周率");


int main(int argc, char* argv[])
{
    // 3. 版本号与用法说明（--version / --helpshort 会用到）
    gflags::SetVersionString("1.0.0");
    gflags::SetUsageMessage("用法: ./main [flags] [剩余参数...]");

    // 4. 先读取配置文件，再解析命令行（命令行参数可覆盖配置文件里的值）
    gflags::ReadFromFlagsFile("main.conf", argv[0], /*errors_are_fatal=*/true);

    // 5. 解析参数：remove_flags=true 会把已识别的 flag 从 argv 中移除
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // 6. 访问参数：直接用 FLAGS_<名字> 读取
    std::cout << "reuse_addr = " << FLAGS_reuse_addr << std::endl;
    std::cout << "listen_port = " << FLAGS_listen_port << std::endl;
    std::cout << "listen_ip   = " << FLAGS_listen_ip << std::endl;
    std::cout << "pi          = " << std::setprecision(8) << FLAGS_pi << std::endl;

    // 7. 在其它编译单元里通过 DECLARE_string + FLAGS_listen_ip 访问同一个 flag
    print();

    return 0;
}
