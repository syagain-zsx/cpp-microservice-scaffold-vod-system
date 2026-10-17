# gflags 入门教程（零基础版）

本文以本目录的 demo（`main.cc` + `child.cc` + `main.conf`）为例，一行行讲清楚。

---

## 0. 先说人话：gflags 是干啥的？

你写的程序经常需要一些「可以调的开关」，比如：

- 服务器监听哪个端口？`8080` 还是 `9090`？
- 监听哪个 IP？
- 要不要开启「地址重用」？

**土办法**是每次想改就打开代码改数字、再重新编译一遍，很烦。

**gflags 的办法**是：在代码里给这些开关起个名字（叫 **flag**，中文叫「标志」或「命令行参数」），
然后运行时在终端里直接加就行，**不用改代码、不用重新编译**：

```bash
./main --listen_port=9090      # 这次用 9090 端口
./main --listen_port=7070      # 下次用 7070 端口
```

就这么简单。下面开始动手。

---

## 1. 装好 gflags

Ubuntu / Debian：

```bash
sudo apt-get install libgflags-dev
```

（如果已经有 `gflags/gflags.h` 这个头文件，说明装好了，跳过。）

---

## 2. 最小例子：逐行解释

```cpp
#include <gflags/gflags.h>   // ← 用 gflags 必须先包这个头文件

// 定义 4 个开关（flag）。格式固定是：DEFINE_类型(名字, 默认值, "说明")
DEFINE_bool  (reuse_addr,  true,      "是否使用地址重用选项");  // 真假开关
DEFINE_int32 (listen_port, 8080,      "服务器监听端口号");      // 整数
DEFINE_string(listen_ip,   "0.0.0.0", "服务器监听IP地址");      // 字符串
DEFINE_double(pi,          3.14159,   "圆周率");                // 小数
```

注意三件事：

1. **类型写在宏名字里**：`DEFINE_bool` / `DEFINE_int32` / `DEFINE_string` / `DEFINE_double`。
2. **名字不带引号**：写 `listen_port`，不是 `"listen_port"`。
3. **字符串默认值要带引号**：`"0.0.0.0"`；数字不用。

然后是 `main` 函数：

```cpp
int main(int argc, char* argv[])
{
    // 设置版本号：运行时敲 --version 会打印它
    gflags::SetVersionString("1.0.0");

    // 设置帮助信息：运行时敲 --help 会显示这句话
    gflags::SetUsageMessage("用法: ./main [flags] [剩余参数...]");

    // 第 1 步：先读配置文件（可选，见第 5 节）
    gflags::ReadFromFlagsFile("main.conf", argv[0], /*errors_are_fatal=*/true);

    // 第 2 步：解析命令行，把用户敲的参数"翻译"进各个 flag 里
    // 第 3 个参数写 true：把识别到的 flag 从 argv 里删掉，剩下的就是你的业务参数
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // 第 3 步：读值。格式固定是 FLAGS_ + 名字（注意全大写）
    std::cout << "reuse_addr  = " << FLAGS_reuse_addr  << std::endl;
    std::cout << "listen_port = " << FLAGS_listen_port << std::endl;
    std::cout << "listen_ip   = " << FLAGS_listen_ip   << std::endl;
    std::cout << "pi          = " << FLAGS_pi          << std::endl;

    return 0;
}
```

> **记忆口诀：先用 `DEFINE_` 定义，中间用 `ParseCommandLineFlags` 解析，最后用 `FLAGS_` 读取。**

⚠️ 最容易踩的坑：`ParseCommandLineFlags` **必须先调用**，否则你读到的永远是代码里的默认值，别人传的参数不生效。

---

## 3. 在终端里怎么传参

| 写法 | 含义 |
| --- | --- |
| `./main --listen_port=9090` | 最标准写法，两个减号 |
| `./main -listen_port=9090` | 一个减号 gflags 也认，效果一样 |
| `./main --listen_port 9090` | 用空格分开，也可以 |
| `./main --reuse_addr` | bool 开关：这样写 = 打开（true） |
| `./main --noreuse_addr` | bool 开关：名字前加 `no` = 关闭（false） |
| `./main --listen_ip="1.2.3.4"` | 值里有符号时加引号，更保险 |

不传任何参数，就全部用代码里的默认值。

---

## 4. 在别的文件里用同一个 flag

程序变大后，代码会拆成好几个文件。`listen_ip` 是在 `main.cc` 里 **定义** 的，
`child.cc` 想读它，就不能再定义一遍，而是用 `DECLARE_` **声明** 一下：

```cpp
// child.cc
#include "child.h"
#include <gflags/gflags.h>
#include <iostream>

DECLARE_string(listen_ip);   // 注意：这里是 DECLARE，不是 DEFINE

void print()
{
    std::cout << "listen_ip = " << FLAGS_listen_ip << std::endl;
}
```

**区别记牢：**

- `DEFINE_xxx` = 「这里有一个开关」，**整个程序只能有一个地方写**（写在 `main.cc`）。
- `DECLARE_xxx` = 「我知道别处有个这个开关」，可以写在 n 个文件里。
- 两者的 **类型和名字必须一模一样**：`DEFINE_int32` 对应 `DECLARE_int32`。

好处：**flag 是全局共享的**，在 `main.cc` 改一次，`child.cc` 里读到的也是新值（第 6 节运行结果能看到）。

---

## 5. 配置文件：一次写一堆参数

命令行只能一次传几个参数，参数多了就写不动了。这时用配置文件 `main.conf`：

```
--listen_ip=0.0.0.0
--reuse_addr=false
--listen_port=9090
--pi=3.1415926
```

格式和命令行完全一样：**一行一个 flag，`--名字=值`**。
代码里用 `gflags::ReadFromFlagsFile("main.conf", argv[0], true)` 读进来。

这样做的意义：**默认值写在配置文件里（不用重新编译），临时需要改动就用命令行覆盖。**

---

## 6. 编译

```makefile
CXXFLAGS = -Wall -std=c++14
LDFLAGS  = -lgflags      # ← 关键：链接 gflags 库

main : main.cc child.cc child.h
	g++ $(CXXFLAGS) main.cc child.cc -o main $(LDFLAGS)

.PHONY : clean
clean :
	rm -f main
```

终端里执行：

```bash
make        # 编译，生成可执行文件 main
./main      # 运行
make clean  # 删掉编译产物
```

---

## 7. 运行结果：看参数到底听谁的

**第一次：命令行只指定 `listen_ip`**

```bash
$ ./main -listen_ip=1.2.3.4
reuse_addr  = 0
listen_port = 9090
listen_ip   = 1.2.3.4
pi          = 3.1415926
listen_ip = 1.2.3.4
```

怎么读这个结果：

- `listen_port = 9090`、`pi = 3.1415926`、`reuse_addr = 0` → 都来自 **`main.conf`**（默认值是 8080 / 3.14159 / true，被配置文件改掉了）。
- `listen_ip = 1.2.3.4` → 来自 **命令行**，把配置文件里的 `0.0.0.0` 覆盖了。
- 最后一行 `listen_ip = 1.2.3.4` 是 `child.cc` 打印的，值一样 → 证明 **flag 全局共享**。
- `reuse_addr = 0` 是 `bool` 类型被 `cout` 打印出来的样子（`0` 就是 `false`，`1` 就是 `true`）。

**第二次：换个参数覆盖**

```bash
$ ./main --listen_port=7070
reuse_addr  = 0
listen_port = 7070
listen_ip   = 0.0.0.0
pi          = 3.1415926
listen_ip = 0.0.0.0
```

这次 `listen_port` 被命令行改成 7070，`listen_ip` 没人管，就回落到配置文件里的 `0.0.0.0`。

### 结论：优先级从高到低

$$\text{命令行参数} \;>\; \text{配置文件} \;>\; \text{代码里的默认值}$$

---

## 8. 报错了怎么办

| 现象 | 原因 / 解决 |
| --- | --- |
| `unknown command line flag 'listenPort'` | flag 名字打错了，注意下划线和小写 |
| `ERROR: failed to open ... main.conf` | 配置文件不在当前目录，或路径写错 |
| 传了参数但值没变 | 忘了调用 `ParseCommandLineFlags`，或写在它之前 |
| `undefined reference to ... FLAGS_xxx` | 漏了 `DECLARE_xxx`，或忘了链接 `-lgflags` |
| 编译错误 `... was not declared` | 忘了 `#include <gflags/gflags.h>` |

---

## 9. 常用内置参数（不用自己定义，gflags 自带）

| 参数 | 作用 |
| --- | --- |
| `./main --help` | 打印所有 flag 和说明 |
| `./main --helpshort` | 只打印当前文件的 flag |
| `./main --version` | 打印 `SetVersionString` 设的版本号 |
| `./main --flagfile=xxx.conf` | 读取指定配置文件（等价于自己调 `ReadFromFlagsFile`） |
| `./main --undefok=name1,name2` | 忽略这几个不认识的参数，不报错退出 |

---

## 10. 一页速查表

| 想做的事 | 代码怎么写 |
| --- | --- |
| 定义开关 | `DEFINE_int32(port, 8080, "说明");` |
| 别的文件用它 | `DECLARE_int32(port);` |
| 启动解析 | `gflags::ParseCommandLineFlags(&argc, &argv, true);` |
| 读值 | `FLAGS_port` |
| 配套类型 | `bool` / `int32` / `int64` / `uint64` / `double` / `string` |

**整体流程：**

```mermaid
flowchart LR
    A["代码里 DEFINE_xxx<br/>写默认值"] --> B["ReadFromFlagsFile<br/>读配置文件"]
    B --> C["ParseCommandLineFlags<br/>解析命令行"]
    C --> D["用 FLAGS_xxx 读值"]
```
