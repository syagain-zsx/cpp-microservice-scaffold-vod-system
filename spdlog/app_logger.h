#pragma once

#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <string>

// 日志器类型：同步 / 异步
enum class LoggerType
{
    Sync,
    Async
};

// 输出目标：控制台 / 文件 / 两者
enum class OutputTarget
{
    Console,
    File,
    Both
};

// 日志器参数结构体
struct LoggerConfig
{
    std::string name{"app"};                                   // 日志器名称
    spdlog::level::level_enum level{spdlog::level::info};      // 日志输出等级
    std::string pattern{"[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"}; // 输出格式
    LoggerType type{LoggerType::Async};                        // 日志器类型
    OutputTarget target{OutputTarget::Both};                   // 输出目标
    std::string file_path{"logs/app.log"};                     // 普通日志文件
    std::string error_file_path{"logs/error.log"};             // 错误日志文件（留空则不创建）
    std::size_t queue_size{8192};                              // 异步队列长度
    std::size_t thread_count{1};                               // 异步后台线程数
};

// 全局日志器初始化接口
bool InitLogger(const LoggerConfig &cfg = LoggerConfig{});

// 获取全局日志器（未初始化时会用默认配置兜底）
std::shared_ptr<spdlog::logger> GetLogger();

// 刷新并释放全局日志器
// 异步模式下必须调用，否则队列中尚未处理的日志可能丢失
void ShutdownLogger();

// ---------------- 二次封装：自动附带 文件名:行号 ----------------
// 注意：fmt 必须是字符串字面量，这里依赖相邻字符串字面量在编译期拼接，
//       不要写成 "[{}:{}] " + fmt。
#define LOG_TRACE(fmt, ...) \
    ::GetLogger()->trace("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    ::GetLogger()->debug("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    ::GetLogger()->info("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    ::GetLogger()->warn("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    ::GetLogger()->error("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_CRITICAL(fmt, ...) \
    ::GetLogger()->critical("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

// 与课件写法保持一致的简写
#define ERR(fmt, ...) LOG_ERROR(fmt, ##__VA_ARGS__)
