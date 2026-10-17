# cpp-microservice-scaffold-vod-system

基于 C++ 的微服务脚手架实践仓库：将视频点播（VOD）系统中常用的中间件客户端封装为可复用模块，并为每个模块提供可直接编译运行的示例代码。

## 目录结构

```text
.
├── .gitignore                      # 忽略编译产物、日志、生成代码等
├── .vscode/                        # VS Code 构建任务与 C++ 配置
└── cpp-microservice-scaffold/
    ├── source/                     # 脚手架封装库（头文件 + 实现）
    └── example/                    # 各中间件的独立示例，每个目录自带 makefile
```

## 模块一览（`source/`）

| 模块 | 命名空间 | 说明 | 主要依赖 |
| --- | --- | --- | --- |
| `log.h/.cc` | `bitelog` | spdlog 日志封装：同步/异步、文件轮转、`DBG/INF/WRN/ERR` 宏 | spdlog、fmt |
| `util.h/.cc` | `biteutil` | JSON 序列化/反序列化，失败时返回 `std::optional` | jsoncpp |
| `rpc.h/.cc` | `biterpc` | brpc 信道管理：节点增删、负载选择 | brpc、protobuf |
| `etcd.h/.cc` | `bitesvc` | 基于 etcd 的服务注册与发现（`SvcProvider`、`wait_for_connection`） | etcd-cpp-apiv3 |
| `mq.h/.cc` | `bitemq` | RabbitMQ 客户端：交换机类型、死信队列、消息 TTL | amqpcpp、libev |
| `redis.h/.cc` | `biteredis` | redis++ 连接池封装（`RedisFactory`） | redis++、hiredis |
| `odb.h/.cc` | `biteodb` | ODB + MySQL 数据库工厂（`DBFactory`） | ODB、MySQL Client |
| `elastic.h/.cc` | `biteics` | Elasticsearch 客户端封装与查询文档构建 | elasticlient、cpr |
| `fdfs.h/.cc` | `bitefdfs` | FastDFS 客户端封装：上传/下载/删除、连接池 | fastdfs、fastcommon |
| `hls.h/.cc` | `bitehls` | 基于 FFmpeg 的 HLS 切片与播放列表生成 | FFmpeg（avformat/avcodec/avutil） |
| `mail.h/.cc` | `bitecode` | 邮箱验证码发送（基于 curl，服务商地址可配置） | libcurl、OpenSSL |

## 示例一览（`example/`）

| 示例 | 内容 |
| --- | --- |
| `amqpcpp` | 简单发布/订阅、死信队列发布/订阅 |
| `brpc` | 同步/异步/HTTP RPC 服务端与客户端（`cal.proto`） |
| `elasticsearch` | ES 客户端基本使用 |
| `etcd` | 服务注册与发现（单键、目录两种方式） |
| `fastdfs` | 文件上传、下载、删除 |
| `ffmpeg` | HLS 切片与 HTTP 播放服务 |
| `gflags` | 命令行参数解析（含父子进程示例） |
| `gtest` | 单元测试：基础、全局环境、局部测试 |
| `httplib` | HTTP 服务端/客户端与静态资源托管 |
| `jsoncpp` | JSON 序列化与反序列化 |
| `mail` | 邮件验证码发送 |
| `odb` | ORM 增删改查（`crud`/`example`/`group`/`limit`/`native`/`view`） |
| `protobuf` | Protobuf 序列化与反序列化 |
| `redis` | string/hash/list/set/zset/pipeline/transaction/watch |
| `spdlog` | 同步与异步日志输出 |
| `websocketpp` | WebSocket 服务端 |

## 环境要求

- Linux（Ubuntu 20.04+ / Debian 11+ 验证），g++ 支持 C++14 及以上
- 常用依赖可尝试通过包管理器安装：

```bash
sudo apt install -y build-essential cmake \
  libgflags-dev libprotobuf-dev protobuf-compiler \
  libjsoncpp-dev libspdlog-dev libfmt-dev libgtest-dev \
  libcurl4-openssl-dev libssl-dev libhiredis-dev \
  libboost-system-dev libmysqlclient-dev \
  libavformat-dev libavcodec-dev libavutil-dev
```

以下库通常需要源码编译安装：brpc、etcd-cpp-apiv3、cpr、elasticlient、fastdfs、amqpcpp、redis++、ODB、cpprestsdk、websocketpp。

## 编译与运行

每个示例目录相互独立，直接使用自带的 makefile：

```bash
cd cpp-microservice-scaffold/example/redis
make          # 编译当前示例
make clean    # 清理生成的可执行文件
```

需要生成代码的示例（如 `brpc`、`protobuf`、`odb`），请先执行 `protoc` / `odb-compiler` 生成对应源码，再运行 `make`。

## 使用约定

- 各模块统一使用 `bite` 前缀命名空间（如 `bitelog`、`biteredis`）。
- 接口失败时优先返回 `std::optional`，由调用方判断并处理。
- 连接参数集中放在 `xxx_settings` 结构体中，便于统一配置。
- 日志统一通过 `bitelog_init()` 初始化，再使用 `DBG/INF/WRN/ERR` 宏输出。

## 注意事项

- 部分示例依赖外部服务（MySQL、Redis、etcd、RabbitMQ、Elasticsearch、FastDFS），运行前需先启动对应服务，并在示例代码中修改连接地址与账号密码。
- 编译生成的可执行文件、目标文件、日志及代码生成产物已被 `.gitignore` 忽略，请勿手动提交。
- 本仓库内容仅用于学习与技术交流。
