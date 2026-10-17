# 编译选项：
#   -Wall                   打开常用警告
#   -std=c++14              使用 C++14 标准
#   -DSPDLOG_COMPILED_LIB   使用已编译好的 libspdlog（而不是 header-only 模式）
#   -DSPDLOG_FMT_EXTERNAL   使用系统独立的 fmt 库（Debian 的 spdlog 包就是这样编的）
CXXFLAGS = -Wall -std=c++14 -DSPDLOG_COMPILED_LIB -DSPDLOG_FMT_EXTERNAL

# 链接选项：
#   -lspdlog   spdlog 库
#   -lfmt      spdlog 依赖的格式化库
#   -pthread   线程支持
LDFLAGS  = -lspdlog -lfmt -pthread

stdout : stdout.cc app_logger.cc app_logger.h
	$(CXX) $(CXXFLAGS) stdout.cc app_logger.cc -o stdout $(LDFLAGS)

.PHONY : clean
clean :
	rm -f stdout
