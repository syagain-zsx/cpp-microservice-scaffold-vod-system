#include "app_logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <sys/stat.h>

#include <cerrno>
#include <iostream>
#include <vector>

namespace
{
// 全局日志器
std::shared_ptr<spdlog::logger> g_logger;

// 逐级创建日志文件所在的目录，例如 "logs/app.log" -> 创建 "logs"
bool EnsureParentDirectory(const std::string &file_path)
{
    const std::string::size_type pos = file_path.find_last_of('/');
    if (pos == std::string::npos)
    {
        return true; // 没有目录部分
    }

    const std::string dir = file_path.substr(0, pos);
    std::string current;
    for (std::string::size_type i = 0; i <= dir.size(); ++i)
    {
        if (i == dir.size() || dir[i] == '/')
        {
            if (!current.empty() && current != "." &&
                ::mkdir(current.c_str(), 0755) != 0 && errno != EEXIST)
            {
                return false;
            }
        }
        if (i < dir.size())
        {
            current.push_back(dir[i]);
        }
    }
    return true;
}
} // namespace

bool InitLogger(const LoggerConfig &cfg)
{
    try
    {
        std::vector<spdlog::sink_ptr> sinks;

        const bool need_console =
            (cfg.target == OutputTarget::Console || cfg.target == OutputTarget::Both);
        const bool need_file =
            (cfg.target == OutputTarget::File || cfg.target == OutputTarget::Both);

        // 1. 控制台 sink
        if (need_console)
        {
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sink->set_level(cfg.level);
            sink->set_pattern(cfg.pattern);
            sinks.push_back(sink);
        }

        // 2. 普通日志文件 sink
        if (need_file)
        {
            if (!EnsureParentDirectory(cfg.file_path))
            {
                std::cerr << "创建日志目录失败: " << cfg.file_path << std::endl;
                return false;
            }
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(cfg.file_path, true);
            sink->set_level(cfg.level);
            sink->set_pattern(cfg.pattern);
            sinks.push_back(sink);
        }

        // 3. 错误日志文件 sink：只接收 error / critical
        if (!cfg.error_file_path.empty())
        {
            if (!EnsureParentDirectory(cfg.error_file_path))
            {
                std::cerr << "创建日志目录失败: " << cfg.error_file_path << std::endl;
                return false;
            }
            auto sink =
                std::make_shared<spdlog::sinks::basic_file_sink_mt>(cfg.error_file_path, true);
            sink->set_level(spdlog::level::err);
            sink->set_pattern(cfg.pattern);
            sinks.push_back(sink);
        }

        if (sinks.empty())
        {
            std::cerr << "没有可用的日志输出目标" << std::endl;
            return false;
        }

        // 4. 按类型创建同步 / 异步日志器
        std::shared_ptr<spdlog::logger> logger;
        if (cfg.type == LoggerType::Async)
        {
            // 初始化全局线程池（必须在创建 async_logger 之前）
            spdlog::init_thread_pool(cfg.queue_size, cfg.thread_count);
            logger = std::make_shared<spdlog::async_logger>(
                cfg.name,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block);
        }
        else
        {
            logger = std::make_shared<spdlog::logger>(cfg.name, sinks.begin(), sinks.end());
        }

        logger->set_level(cfg.level);         // 总闸门
        logger->flush_on(spdlog::level::err); // error 起立即刷盘

        g_logger = logger;
        spdlog::set_default_logger(logger); // spdlog::info(...) 也走这个日志器
        return true;
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
        return false;
    }
}

std::shared_ptr<spdlog::logger> GetLogger()
{
    if (!g_logger)
    {
        InitLogger(LoggerConfig{}); // 兜底，避免宏调用空指针
    }
    return g_logger ? g_logger : spdlog::default_logger();
}

void ShutdownLogger()
{
    if (!g_logger)
    {
        return;
    }

    // 异步模式下：
    //   1. flush 消息进入队列；
    //   2. shutdown 释放线程池，线程池析构会等待队列中已有消息处理完再 join；
    //   3. 期间 g_logger 必须保持有效，否则队列里的日志会因 weak_ptr 失效被丢弃。
    g_logger->flush();
    spdlog::shutdown();
    g_logger.reset();
}
