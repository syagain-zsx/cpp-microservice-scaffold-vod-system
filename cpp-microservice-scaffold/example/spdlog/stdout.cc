/*
    标准输出的日志：
        1. 包含头文件 <spdlog/spdlog.h>
        2. 创建日志对象
        3. 设置日志输出等级
        4. 设置日志输出格式
        5. 输出日志
*/

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <iostream>
int main(int argc, char *argv[])
{
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("stdout_logger");
    // auto logger = spdlog::basic_logger_mt("file_logger", "./file.dat");
    // auto logger = spdlog::rotating_logger_mt("file_logger", "./rotating.dat", 1024, 3);
    logger->set_level(spdlog::level::err);
    logger->set_pattern("[%H:%M:%S][%-7l]: %v");
    for (int i = 0; i < 10000; i++) {
        logger->error("hello world - {}", i);
    }
    return 0;
}