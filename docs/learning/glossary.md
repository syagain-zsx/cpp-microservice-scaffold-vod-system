# 术语白皮书（看不懂就查这里）

> 用法：每个词给三样东西——**大白话（带类比）→ 在本项目里指什么 → 去哪能看到**。
> 不要背定义，先把词和「项目里的一个文件/一条命令」对应起来。

## 先记住这 8 个（最容易卡住）

| 词 | 一句话 |
| --- | --- |
| **lease（租约）** | 服务在注册中心登记的“有效期”，到期不续就会被自动删掉。 |
| **watch（订阅）** | 网关订阅 etcd 的变化，服务上下线会主动通知它，不用一直轮询。 |
| **TTL** | 一条缓存/消息的“保质期”，到时间自动失效。 |
| **DLX + TTL 延迟队列** | 用“死信 + 过期时间”拼出来的定时任务：到点才进真正的队列。 |
| **cache-aside** | 读缓存，没有就查库并回填；写库时删缓存。 |
| **remux（转封装）** | 只换“包装盒”（mp4→ts），不重新压缩画面，快但码率不变。 |
| **m3u8** | HLS 的“播放清单”，里面列着一串 ts 分片和时长。 |
| **幂等** | 同一个操作做多次，结果和做一次一样（重试安全）。 |

---

## 一、架构与通信

### 微服务
**大白话**：把一个大系统拆成几个小服务，各自独立运行、各自存数据，通过网络互相调用。
**类比**：餐厅拆成前台、厨房、仓库，各干各的，靠传菜单/取货单沟通。
**在本项目**：gateway + user / video / file / transcode 五个进程。
**在哪看**：README 架构图、`server/svc_*/`。

### 网关（gateway）
**大白话**：所有请求的统一入口，先查权限，再转给对应的服务。
**类比**：公司前台，访客先登记，再被带到对应部门。
**在本项目**：`server/svc_gateway`，监听 10000，客户端只连它。
**在哪看**：`server/svc_gateway/source/svc_rpc.cc` 的 `tempLogin`。

### RPC
**大白话**：像调用本地函数一样调用另一台机器上的函数。
**类比**：打电话让同事帮你查个数，你只关心“报出数字”，不关心他怎么查。
**在本项目**：网关调用 user/video/file 服务；用 brpc + protobuf。
**在哪看**：`scaffold/JianLiuScaffold/rpc.cc`。

### HTTP / JSON
**大白话**：浏览器/客户端和服务器说话的格式；JSON 是其中一种“键值对”文本。
**在本项目**：客户端 ↔ 网关用 HTTP + JSON；例如 `POST /HttpService/tempLogin`。
**在哪看**：`curl` 命令、`docs/02-api.md`。

### protobuf
**大白话**：一种“数据表格模板”，先定义字段，再自动生成读写代码，比 JSON 更省空间。
**在本项目**：`server/proto/*.proto` 定义接口和消息；编译时自动生成 C++ 代码。
**在哪看**：`server/proto/video.proto` 的 `AllVideoListRsp`。

### brpc
**大白话**：百度开源的 C++ RPC 框架，负责网络收发、超时、负载均衡。
**在本项目**：每个服务用 brpc 起 RPC Server；网关用 brpc Channel 调子服务。
**在哪看**：`svc_server.cc` 里 `brpc::Server`。

### Channel / Closure（done）
**大白话**：Channel 是“通往某个服务的电话线”；Closure 是“打完电话后要执行的回调”。
**在本项目**：`_channels->getNode(...)` 拿 Channel；每个 RPC 调用带一个 `done` 回调。
**在哪看**：`svc_rpc.cc` 里的 `ClosureGuard done_guard(done)`。

### sessionId（会话）
**大白话**：登录成功后服务器发的一张“临时通行证”，之后每个请求都带上它。
**在本项目**：`tempLogin/passwdLogin` 返回；存在 MySQL 的会话表 + Redis。
**在哪看**：`docs/learning/cheatsheet.md` 第 3 节的取会话脚本。

### 鉴权
**大白话**：判断“你是谁、你能不能做这件事”。
**在本项目**：网关按 URL 查角色表，决定放行还是拒绝。
**在哪看**：`svc_rpc.cc` 的 `judgePermission`。

---

## 二、服务注册与发现

### 注册中心 / etcd
**大白话**：一个“电话簿”，服务启动时把自己的地址写进去，别人来查。
**类比**：公司通讯录，新同事入职登记，离职删除。
**在本项目**：etcd 监听 2379，存 `/user/...`、`/video/...`、`/file/...`。
**在哪看**：cheatsheet 第 5 节的 etcd 查询。

### lease（租约）/ KeepAlive（续约）
**大白话**：登记不是永久的，而是一张“10 秒有效期的门票”，服务要定期续期；进程挂了就不再续，10 秒后自动被删。
**在本项目**：`etcd.cc` 里 `leasegrant(10)` + `KeepAlive(..., 10, ...)`。
**在哪看**：`scaffold/JianLiuScaffold/etcd.cc` 第 36–55 行。

### watch（订阅）
**大白话**：不用一直问“有没有变化”，而是订阅；一有变化 etcd 主动通知。
**类比**：关注公众号，而不是每分钟刷新网页。
**在本项目**：网关 watch 服务节点，收到上线/下线事件就更新本地列表。
**在哪看**：`etcd.cc` 的 watch 逻辑、网关启动日志。

### 全量同步（resync）
**大白话**：除了订阅，还每 5 秒把整本电话簿重新拉一遍，防止漏掉通知。
**在本项目**：`etcd.cc` 的 5s 周期同步。
**在哪看**：`etcd.cc` 里 `resync`。

### 服务发现 / 负载均衡
**大白话**：调用方通过注册中心找到可用地址，有多个就轮流选。
**在本项目**：`_channels->getNode("video")` 返回一个地址；`rpc.cc` 里轮询。
**在哪看**：`scaffold/JianLiuScaffold/rpc.cc`。

---

## 三、缓存与数据库

### Redis
**大白话**：内存里的“超快字典”，用来存热点数据，读写比 MySQL 快很多。
**在本项目**：端口 6379，密码 123456；存列表缓存、会话、点赞计数。
**在哪看**：cheatsheet 第 6 节的 Redis 查询。

### TTL
**大白话**：缓存/消息的“保质期”，时间一到自动删除。
**在本项目**：首页列表缓存 3 秒（`setex(key, 3, ...)`）；审核后最多 3 秒才可见。
**在哪看**：`server/svc_video/source/svc_data.cc` 第 539 行。

### cache-aside（旁路缓存）
**大白话**：读：先查缓存，没有就查数据库并把结果放回缓存；写：改数据库，再删缓存。
**类比**：先看冰箱有没有菜，没有再买菜并放回冰箱；买了新菜就把旧的扔掉。
**在本项目**：`getMainPageList` 就是这套流程。
**在哪看**：`svc_data.cc` 第 468 行起。

### 缓存穿透 / 击穿 / 雪崩
**大白话**：
- 穿透：查一个**根本不存在**的数据，每次都打到数据库；
- 击穿：某个**热点 key 过期**的瞬间，大量请求同时打到数据库；
- 雪崩：**大量 key 同时过期**，数据库被压垮。
**在本项目**：列表缓存很短（3s），靠“回源 + 回填”兜底；空值缓存/随机 TTL 还没做，可作为改进点。
**在哪看**：`docs/notes/01-redis-cache.md`。

### 延迟双删 / 回写
**大白话**：
- 延迟双删：改库后删一次缓存，过一会儿再删一次，减少“旧数据被读回缓存”的窗口；
- 回写：计数先改 Redis，过 30 秒再用消息批量写回 MySQL。
**在本项目**：`delete_cache_queue`（3s 延迟）、`cache_sync_to_db_queue`（30s 延迟）。
**在哪看**：`run-dev.sh` 的四段队列 JSON、`svc_sync.cc`。

### MySQL / 表 / 主键
**大白话**：关系型数据库，数据存在“表”里，每行有唯一主键。
**在本项目**：库 `vptest`，13 张表（用户、会话、视频、文件、点赞、弹幕…）。
**在哪看**：`deploy/sql/data.sql`、Navicat/DBeaver。

### ODB / ORM / DAO
**大白话**：
- ORM：用 C++ 类/对象操作数据库，不用手写 SQL；
- ODB：C++ 的 ORM 工具，从 `data.h` 自动生成建表和读写代码；
- DAO：专门负责“读写数据库”的那一层代码。
**在本项目**：`server/data/data.h` 定义实体，`server/data/*.cc` 是 DAO。
**在哪看**：`server/data/video.cc`、`docs/notes/04-odb.md`。

### 事务 / 连接池
**大白话**：
- 事务：一组操作要么全成功、要么全回滚；
- 连接池：预先建好一批数据库连接，反复用，避免每次新建。
**在本项目**：`odb::transaction mtx(_mysql->begin())`；连接池大小 5。
**在哪看**：`svc_data.cc` 里的 `odb::transaction`。

---

## 四、消息队列与异步

### RabbitMQ / exchange / queue / binding
**大白话**：消息中间人。生产者把消息发给**交换机（exchange）**，交换机按规则（binding）投到**队列（queue）**，消费者从队列取。
**类比**：寄快递：交到网点（exchange），按地址分到对应货架（queue），收件人去取。
**在本项目**：端口 5672，账号 admin/123456；四类队列见 `run-dev.sh`。
**在哪看**：`scaffold/JianLiuScaffold/mq.cc`。

### DLX + TTL（延迟队列）
**大白话**：消息先进入一个“死信队列”（DLX），设置过期时间（TTL）；到期后自动转到真正的业务队列，从而实现“延迟 3 秒/30 秒再处理”。
**类比**：定时快递柜，时间到了才通知取件。
**在本项目**：删缓存的 3s、计数回写的 30s。
**在哪看**：`docs/notes/02-rabbitmq-delay.md`。

### ack / confirm / 幂等
**大白话**：
- ack：消费者处理完告诉 MQ“可以删这条消息了”；
- confirm：生产者确认 MQ 真的收到了消息；
- 幂等：同一条消息处理多次，结果不变（比如按 video_id 更新，而不是“加一”）。
**在本项目**：消费后 `ack`；confirm 暂未开启；转码按视频更新，天然接近幂等。
**在哪看**：`mq.cc` 的 `consume`/`ack`。

### 异步 / 解耦
**大白话**：把耗时的事（转码）丢进队列后台做，请求先返回，不用干等。
**在本项目**：上传 → 写库 → 投递 MQ → transcode 慢慢转。
**在哪看**：`svc_transcode/source/svc_mq.cc`。

---

## 五、文件与视频

### FastDFS / tracker / storage / fileId
**大白话**：分布式文件系统。tracker 是“索引台”，告诉你文件该存到哪台 storage；storage 是真正“书架”。上传后得到一个 fileId，以后用它下载。
**在本项目**：上传视频/封面/ts 分片；fileId 也用于播放地址。
**在哪看**：`scaffold/JianLiuScaffold/fdfs.cc`；storage 必须启动（23000 端口）。

### FFmpeg
**大白话**：音视频处理的“瑞士军刀”，可以转码、切片、截帧。
**在本项目**：转码服务用它把 mp4 切成 HLS；客户端用它截首帧封面。
**在哪看**：`scaffold/JianLiuScaffold/hls.cc`、`mpv/mpvplayer.cpp`。

### remux（转封装）
**大白话**：只换“外壳”（mp4 → ts），不重新压缩画面，速度快、画质不变、文件大小差不多。
**在本项目**：`HLSTranscoder::transcode` 复制编码参数，只写 HLS 容器。
**在哪看**：`hls.cc` 第 61 行起。

### HLS / m3u8 / ts / 分片 / hls_time
**大白话**：HLS 是把视频切成很多小段（ts），再用一个清单（m3u8）按顺序播放；`hls_time` 是每段最长多少秒。
**类比**：把一部电影切成多集短视频，播放器按清单逐集播放。
**在本项目**：`hls_time=10`；每个 ts 上传 FastDFS 并生成一个 fileId，m3u8 里记录这些 fileId。
**在哪看**：`hls.cc`、`run-dev.sh` 的 transcode 启动参数。

---

## 六、工程、测试与排错

### 冒烟测试 / 单元测试 / E2E
**大白话**：
- 冒烟：最快确认“系统还活着”（4 项）；
- 单元测试：测一个小函数/小模块（8 项 gtest）；
- E2E：端到端，从上传到播放全跑一遍（18 项）。
**在本项目**：`smoke-test.sh`、`ctest`、`e2e-test.py`。
**在哪看**：`scripts/`。

### 压测 / QPS / p99
**大白话**：QPS 是每秒处理多少请求；p99 是“99% 的请求都在多少毫秒内完成”。
**在本项目**：`bench.py` 对列表接口压测；优化后约 800 QPS、p99 94ms。
**在哪看**：`docs/05-performance.md`。

### 降级
**大白话**：某个依赖坏了，先关掉相关功能，保证主流程还能用，而不是整个服务崩。
**在本项目**：ES 不可用时搜索返回空；MQ 声明超时会记录并继续运行。
**在哪看**：`server/data/video_search.cc`、`mq.cc`。

### 状态机（视频状态）
**大白话**：一个东西在几个状态之间按规则流转。
**在本项目**：5 转码中 → 1 待审核 → 2 已上架（或 3 驳回 / 4 下架）；只有 2 会在首页显示。
**在哪看**：`server/data/data.h` 的 `VideoStatus`。

### 除零 → inf → 未定义行为（UB）
**大白话**：除以 0 会得到“无穷大”，把无穷大塞进整数是未定义行为（程序可能崩/乱），本项目就因此崩过。
**在本项目**：`duration=0` 时算进度比例，得到 inf，再传给进度条宽度，Windows 下崩溃。
**在哪看**：`playerpage.cpp`、`playslider.cpp` 的修复。

### Qt 信号槽 / libmpv（客户端）
**大白话**：
- 信号槽：Qt 的“事件通知”机制，一个对象发信号，另一个对象接；
- libmpv：把 mpv 播放器嵌进 Qt 窗口的库。
**在本项目**：`netclient` 发 `xxxDone` 信号，界面槽函数更新 UI；`mpvplayer.cpp` 封装 libmpv。
**在哪看**：`client/JianLiuPlayer/netclient/netclient.cpp`、`mpv/mpvplayer.cpp`。

### CMake / MinGW / 构建
**大白话**：CMake 是“构建脚本”，描述怎么编译；MinGW 是 Windows 上的 GCC 工具链。
**在本项目**：服务端 `./scripts/build.sh`；客户端用 Qt Creator + MinGW。
**在哪看**：根目录 `CMakeLists.txt`、`client/JianLiuPlayer/CMakeLists.txt`。

---

## 遇到新词怎么办

1. 先在这里搜；没有就搜 `docs/notes/`；
2. 把词和**一条命令/一个文件**绑定（比如 lease → `etcd.cc` 第 37 行）；
3. 用自己的话写进周记，写不出来就是还没懂。
