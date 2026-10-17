#include "child.h"

#include <gflags/gflags.h>

#include <iostream>

// 在其它编译单元引用 main.cc 里定义的 flag，必须用 DECLARE_xxx
DECLARE_string(listen_ip);

void print()
{
    std::cout << "listen_ip = " << FLAGS_listen_ip << std::endl;
}