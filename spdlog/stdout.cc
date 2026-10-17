/*
    异步全局日志器 + 二次封装示例

        InitLogger(cfg)   -> 创建全局异步日志器（控制台 + 两个文件）
        LOG_XXX(...)      -> 自动附带 文件名:行号
        ShutdownLogger()  -> 等待异步队列排空后释放日志器
*/

#include "app_logger.h"

#include <iostream>

int main()
{
    LoggerConfig cfg;
    cfg.name = "app";
    cfg.level = spdlog::level::trace; // 演示全部级别，生产环境通常设为 info
    cfg.type = LoggerType::Async;     // 异步日志器
    cfg.target = OutputTarget::Both;  // 控制台 + 文件
    cfg.file_path = "logs/app.log";
    cfg.error_file_path = "logs/error.log"; // error / critical 单独存放
    cfg.queue_size = 8192;
    cfg.thread_count = 1;

    if (!InitLogger(cfg))
    {
        std::cerr << "日志初始化失败" << std::endl;
        return 1;
    }

    // 宏会自动加 [文件名:行号]
    LOG_TRACE("trace：控制台 + app.log");
    LOG_DEBUG("debug：控制台 + app.log");
    LOG_INFO("info：控制台 + app.log");
    LOG_WARN("warn：控制台 + app.log");
    LOG_ERROR("error：控制台 + app.log + error.log");
    LOG_CRITICAL("critical：控制台 + app.log + error.log");

    // 无参数调用，验证 ##__VA_ARGS__ 能正确处理空参数
    LOG_INFO("无参数日志");

    ShutdownLogger();
    return 0;
}
