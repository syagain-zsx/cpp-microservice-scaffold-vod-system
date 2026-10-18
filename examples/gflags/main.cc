/*
    gflags使用：
        1. 包含头文件 <gflags/gflags.h>
        2. 定义要捕获的参数信息，以及设置默认值
        3. 解析参数，初始化参数数据
        4. 访问参数
*/

#include <gflags/gflags.h>
#include <iostream>
#include <string>
#include "child.h"

DEFINE_bool(reuse_addr, true, "使用启用地址重用选项");
DEFINE_int32(listen_port, 8080, "服务器监听端口");
DEFINE_string(listen_ip, "0.0.0.0", "服务器监听IP");
DEFINE_double(pi, 3.14, "圆周率");

int main(int argc, char *argv[])
{
    // 解析命令行参数
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 访问参数
    std::cout << FLAGS_reuse_addr << std::endl;
    std::cout << FLAGS_listen_port << std::endl;
    std::cout << FLAGS_listen_ip << std::endl;
    std::cout << FLAGS_pi << std::endl;

    print();
    return 0;
}