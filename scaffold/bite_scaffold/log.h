/*
    日志操作封装：
        1. 防止头文件重复包含
        2. 包含头文件
        3. 声明命名空间
        4. 声明全局日志器
        5. 声明日志配置结构体
        6. 声明全局日志器初始化接口
        7. 封装日志输出宏
*/

// 防止头文件重复包含
#pragma once
// 包含头文件
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

// 声明命名空间
namespace bitelog {
    // 声明日志配置结构体
    struct log_settings {
        bool async = false;     //  是否启用异步日志
        int level = 1;      //  日志输出等级: 1-debug;2-info;3-warn;4-error; 6-off
        std::string format = "[%H:%M:%S][%-7l]: %v";   //  日志输出格式 [%H:%M:%S][%-7l]: %v
        std::string path = "stdout";     //  日志输出目标 stdout
    };
    // 声明全局日志器
    extern std::shared_ptr<spdlog::logger> g_logger;
    // 声明全局日志器初始化接口
    extern void bitelog_init(const log_settings &settings = log_settings());
    // 封装日志输出宏
    #define FMT_PREFIX std::string("[{}:{}]: ")
    #define DBG(fmt, ...) bitelog::g_logger->debug(FMT_PREFIX + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define INF(fmt, ...) bitelog::g_logger->info(FMT_PREFIX + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define WRN(fmt, ...) bitelog::g_logger->warn(FMT_PREFIX + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define ERR(fmt, ...) bitelog::g_logger->error(FMT_PREFIX + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
}