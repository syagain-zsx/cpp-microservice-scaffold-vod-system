# ============================================================
# Google Test 示例工程的 Makefile
#   用法：make        编译全部测试程序
#         make test   编译并运行全部测试
#         make clean  删除生成的可执行文件
# ============================================================

# 编译选项（CXXFLAGS = C++ compiler FLAGS，编译阶段传给 g++ 的参数）
#   -Wall         打开常用编译警告（不改变程序行为，只帮助发现隐患）
#   -std=c++14    使用 C++14 标准
CXXFLAGS = -Wall -std=c++14

# 链接选项（LDFLAGS = Linker FLAGS，链接阶段传给 g++ 的参数）
#   -lgtest        链接 Google Test 主体库（断言、TEST 宏的运行支撑）
#   -lgtest_main   链接 gtest 自带的 main()，测试源码里就不用自己写 main
#                  （源码里已经写了 main 时也没关系，静态库不会重复引入）
#   -lpthread      链接 pthread 线程库，gtest 内部会用到
LDFLAGS  = -lgtest -lgtest_main -lpthread

# .PHONY 声明这些是“假目标”：只执行命令，不检查同名文件是否存在
# 否则目录里一旦出现名为 All/test/clean 的文件，make 会误以为目标已完成
.PHONY: All test clean

# 默认目标：直接执行 make 时，依次构建 global 和 simple
All: global simple

# 目标 global：用 global.cc 生成可执行文件 global
#   $@ = 目标名（global）
#   $< = 第一个依赖（global.cc）
#   $(CXX) 是 make 内置变量，默认值为 g++
global: global.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# 目标 simple：用 simple.cc 生成可执行文件 simple
#   说明：simple.cc 里没有写 main，入口由 -lgtest_main 提供
simple: simple.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# make test：先确保已编译（依赖 All），再依次运行两个测试程序
#   注意：命令行前必须是 Tab，不能用空格，否则 make 会报 missing separator
test: All
	./global
	./simple

# make clean：删除编译产物（只删可执行文件，不动源码）
clean:
	rm -f global simple