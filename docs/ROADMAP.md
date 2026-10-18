# 笺流（JianLiu）· 重构路线图

> 版本：2026-10-18 ｜ 当前分支：`feature/vod-system` ｜ 当前阶段：**M3 接口联调**
>
> 这份文档是整个项目的"总纲"：先定义长远目标，再拆成可验收的里程碑，最后落到每周任务。
> 所有阶段性进展都记录在文末的「进度日志」里。

---

## 一、长远目标（North Star）

**把这个课程版视频点播系统，重构成一个「跑得起来、讲得清楚、写进简历」的个人作品。**

拆成三个层次：

| 层次 | 目标 | 判断标准 |
| --- | --- | --- |
| 工程 | 单一仓库、分层清晰、一条命令构建 | 新人拿到仓库，看 README 能在 30 分钟内跑起来 |
| 能力 | 吃透 11 个中间件封装 + 5 个微服务 + 1 个 Qt 客户端 | 每个模块都能白板画出调用链，说清"为什么这么选" |
| 求职 | 形成可复述的项目故事 | 简历 5 行描述 + 8 个能展开讲 5 分钟的技术点 |

**最终验收（Definition of Done）**

1. `cmake -S . -B build && cmake --build build` 一次通过，产出 5 个服务端可执行文件；
2. `deploy/` 下的脚本能一键拉起依赖中间件并启动全部服务；
3. 客户端能完成：登录 → 首页推荐 → 搜索 → 播放（HLS）→ 上传 → 转码 → 弹幕/点赞/关注；
4. `docs/` 里有架构图、接口说明、部署手册、学习笔记、面试问答；
5. 至少 1 组数据证明做过优化（如缓存命中率、RPC 压测 QPS、转码耗时对比）。

---

## 二、现状盘点（2026-10-18）

### 2.1 已有资产

| 资产 | 位置 | 状态 |
| --- | --- | --- |
| 脚手架基础库（11 个模块） | `scaffold/JianLiuScaffold/` | ✅ 已编译通过（静态库） |
| 服务端 5 个微服务 | `server/svc_*/` | 🟡 已导入，构建中 |
| 数据层（ODB 实体 + DAO，16 个类） | `server/data/` | 🟡 参与构建 |
| protobuf 接口定义（6 个 .proto） | `server/proto/` | ✅ protoc 生成通过 |
| Qt6 客户端 BitPlayer | `client/BitPlayer/` | ⬜ 待联调（仅能在 Windows + Qt6.7.3 构建） |
| 部署配置 | `deploy/`（compose/Dockerfile/global.conf/SQL） | 🟡 待适配新目录 |
| 中间件环境 | Docker 网络 `dev-environment_dev-network` | ✅ mysql/redis/etcd/rabbitmq/fastdfs 在跑；❌ ES 未启动 |

### 2.2 与课程原版的差异（本次重构做的事）

- **合并为一个仓库**：原课程 `client/` + `server/` + 脚手架库三方分离，现在 `scaffold + server + client + deploy + docs` 同仓；
- **统一构建**：删掉 5 个服务各自的 `CMakeLists.txt`（内容几乎复制粘贴），改成顶层一个函数批量生成；
- **重新组织目录**：`sql.d → deploy/sql`、`dockerfile → deploy/Dockerfile`、`global.conf → deploy/conf/`、`svc_file/client → tools/file-client`；
- **补上脚手架工程**：原课程的 `JianLiuScaffold` 是预装在镜像里的，仓库里只有源码没有构建；现在补了 `scaffold/CMakeLists.txt`（导出 `JianLiuScaffold::JianLiuScaffold`）。

### 2.3 已知缺口 / 风险

| 问题 | 影响 | 处理方案 |
| --- | --- | --- |
| 无 root 权限（sudo 需密码） | 不能 `apt install ffmpeg` | ✅ 已解决：`apt-get download` + `dpkg -x` 到 `~/local/ffmpeg`，CMake 通过 `JIANLIU_FFMPEG_ROOT` 引用 |
| `dev-icsearch` 不解析/9200 不通 | 搜索功能不可用 | ✅ 已降级：video 服务捕获连接异常，`_enabled=false`，搜索返回空，其余业务正常；待 ES 恢复后自动启用 |
| 客户端依赖 mpv DLL（108 MB，未入库） | 客户端无法直接运行 | 仓库提供 `client/README.md` 说明下载方式；`.gitignore` 排除 |
| 课程遗留 `data_test`（16 个独立小工程） | 目录噪音、无法统一构建 | M6 阶段改写为 gtest 用例，移入 `server/tests/` |
| 课程代码里注释掉的死代码、重复 `INSTALL()` | 可读性差 | 逐服务清理（M6） |

---

## 三、目标架构

### 3.1 全景图

```text
                        ┌──────────────────────────────┐
                        │   BitPlayer (Qt6 + mpv)      │
                        │  登录/首页/搜索/播放/上传/后台 │
                        └──────────────┬───────────────┘
                                       │ HTTP + protobuf
                                       ▼
                        ┌──────────────────────────────┐
                        │  gateway_server  :10000      │
                        │  HttpService（brpc HTTP）     │
                        └───┬──────────┬───────────┬───┘
              brpc RPC      │          │           │
        ┌───────────────────┘          │           └──────────────────┐
        ▼                              ▼                              ▼
┌───────────────┐            ┌───────────────┐              ┌───────────────┐
│ user_server   │            │ video_server  │              │ file_server   │
│ :10002        │            │ :10003        │              │ :10001        │
└──┬────┬───┬───┘            └──┬───┬───┬───┘              └──┬───┬───┬────┘
   │    │   │                   │   │   │                     │   │   │
   ▼    ▼   ▼                   ▼   ▼   ▼                     ▼   ▼   ▼
 MySQL Redis 邮件           MySQL Redis ES(搜索)          FastDFS  MySQL Redis
                              │    │    │                    │
                              │    │    └── MQ: hls_transcode_queue
                              │    │                  │
                              │    └── MQ: cache_sync_to_db_queue
                              │                       ▼
                              │            ┌──────────────────────┐
                              │            │ transcode_server     │
                              │            │ 拉取 FDFS 视频 → FFmpeg│
                              │            │ → 切片上传 → 更新状态 │
                              │            └──────────────────────┘
                              └── etcd（服务注册/发现，所有服务共用）
```

### 3.2 请求链路（以"上传视频"为例）

1. 客户端把视频分片 + 元信息 POST 给网关 `uploadVideo`；
2. 网关转发 `FileService.uploadVideo` 到 file 服务（brpc，服务地址从 etcd 拿）；
3. file 服务把文件写进 FastDFS，返回 `file_id`，元信息落 MySQL/Redis；
4. 客户端再调 `newVideo`，网关转 `VideoService.newVideo`；
5. video 服务写视频元信息，向 MQ 发一条 `hls_transcode_queue` 消息；
6. transcode 服务消费消息：FDFS 下载 → HLS 切片 → 上传 ts/m3u8 → 回调更新视频状态；
7. 视频状态变为"上架"，客户端首页/列表即可见，播放时拉 m3u8 播放。

### 3.3 目标目录树

```text
vod-system/
├── CMakeLists.txt              # 顶层构建入口
├── README.md                   # 项目介绍 / 快速开始 / 架构速览
├── scaffold/                   # JianLiuScaffold 基础库
│   ├── CMakeLists.txt
│   └── JianLiuScaffold/          # log/util/rpc/etcd/mq/redis/odb/elastic/fdfs/hls/mail
├── server/
│   ├── CMakeLists.txt          # 公共库 + 5 个服务的统一构建
│   ├── common/                 # 错误码、缓存同步抽象
│   ├── data/                   # ODB 实体 + DAO（16 张表/类）
│   ├── proto/                  # 6 个 proto 文件
│   ├── svc_gateway/            # 网关（HTTP 入口）
│   ├── svc_user/               # 用户服务
│   ├── svc_video/              # 视频服务
│   ├── svc_file/               # 文件服务
│   └── svc_transcode/          # 转码服务（FFmpeg）
├── client/                     # Qt6 客户端（Windows 构建）
│   ├── BitPlayer/
│   └── BitPlayerMockServer/    # 客户端 Mock 服务（可选）
├── deploy/
│   ├── docker-compose.yml
│   ├── Dockerfile
│   ├── conf/global.conf
│   └── sql/                    # 建表 SQL
├── docs/                       # 架构 / 接口 / 部署 / 学习笔记 / 面试
├── examples/                   # 脚手架 16 个模块的示例（学习用，可选构建）
├── scripts/                    # 构建、启动、初始化脚本
└── tools/                      # file_client 等联调工具
```

---

## 四、里程碑计划

> 时间按"业余每周 10–15 小时"估算，总计约 8 周。

| 阶段 | 目标 | 关键任务 | 验收标准（DoD） | 预计 |
| --- | --- | --- | --- | --- |
| **M0** 盘点与骨架 | 摸清家底，定目标 | 源码导入、目录重构、路线图 | 仓库结构落定，本文档完成 | ✅ 已完成 |
| **M1** 构建系统 | 一次构建出全部产物 | 脚手架 CMake、统一服务端构建、FFmpeg 本地前缀 | `cmake --build build` 产出 5 个可执行文件 | ✅ 已完成 |
| **M2** 服务端可运行 | 5 个服务能启动、注册到 etcd | 配置文件、启动脚本、端口、日志 | `etcdctl get --prefix /` 能看到 4 个注册节点；`curl :10000/` 有响应 | ✅ 已完成 |
| **M3** 核心接口联调 | 登录/视频/文件主链路通 | 用 `file_client`、curl 或脚本验证 RPC | 能完成"注册登录 → 上传 → 建视频 → 列表查询" | ✅ 已完成（含转码/HLS/点赞/弹幕） |
| **M4** 客户端打通 | BitPlayer 能连真实后端 | 接口地址配置、编译、修 bug | 登录、首页、播放页可用 | 1.5 周 |
| **M5** 业务闭环 | 转码 + HLS 播放 + 互动 | FFmpeg 转码链路、弹幕、点赞、关注、搜索 | 上传的 mp4 能自动转 HLS 并在客户端播放，互动数据正确 | 1.5 周 |
| **M6** 质量与部署 | 可交付 | 清理死代码、补测试、docker-compose、一键脚本 | `docker compose up` 起全栈；gtest 通过 | 1 周 |
| **M7** 文档与求职 | 能讲出来 | 架构文档、学习笔记、面试题、简历 | docs 完整；能脱稿讲 10 分钟 | 1 周 |
| **M8** 优化与压测 | 有数据支撑 | 缓存命中、连接池、RPC 压测、转码耗时 | 一张压测/优化对比表 | 可穿插 |

### M1 任务清单

- [x] `scaffold/CMakeLists.txt`：11 个模块 → `JianLiuScaffold::JianLiuScaffold`
- [x] 顶层 `CMakeLists.txt` + `server/CMakeLists.txt`：protoc/ODB 生成 + 公共库 + 5 服务
- [x] 移除 5 个复制的服务级 CMakeLists，改为 `add_vod_service()` 统一生成
- [x] FFmpeg 依赖落到 `~/local/ffmpeg`（无 root 方案），transcode 参与构建
- [x] 构建产物验证：`build/bin/{gateway,user,video,file,transcode}_server`
- [x] `scripts/build.sh` 固化构建命令

### M2 任务清单

- [x] `scripts/run-dev.sh`：一键启动/停止 5 个服务（按服务裁剪 gflags）
- [x] etcd 注册验证：`/file`、`/user`、`/video` 三个节点 + 租约
- [x] 网关服务发现：watch 上线/下线事件正常
- [x] `curl :10000/status` 返回 HttpService 状态页
- [ ] `deploy/conf/local.conf`（改用脚本传参后暂不需要，保留课程配置做参考）

---

## 五、近期两周计划（M1 → M3）

**第 1 周**

1. 完成 M1 全部验收（5 个可执行文件 + transcode）；
2. 写 `scripts/build.sh`、`scripts/fetch-ffmpeg.sh`（记录无 root 依赖方案）；
3. 本地起服务：先 gateway + user，验证 etcd 注册与 HTTP 响应；
4. 补 `deploy/conf/local.conf`（把 `dev-mysql` 等容器名换成 127.0.0.1/容器名可配）。

**第 2 周**

5. 起全 5 个服务，跑通 `tempLogin → sessionLogin → getCode/regOrLogin`；
6. 用 `tools/file-client` 验证 FastDFS 上传/下载；
7. 用 SQL 初始化 `vptest` 库，验证 ODB 建表与读写；
8. 输出 `docs/02-api.md` 的第一版（登录 + 文件 + 视频接口）。

---

## 六、学习路线（与工程并行，求职导向）

### 6.1 每个模块的"四步法"

> 不要只"看懂"，要能**默写调用链**。

1. **原理**：这个中间件解决什么问题？核心机制是什么？
2. **本项目**：`scaffold/JianLiuScaffold/xxx.h` 封装了哪些能力？服务端怎么用？
3. **动手**：改一个 demo（`examples/` 里有 16 个），或者加一个功能验证理解；
4. **输出**：写一页笔记 + 3 个面试问答，放进 `docs/notes/`。

### 6.2 模块学习顺序（由浅入深）

| # | 模块 | 学习重点 | 面试高频 |
| --- | --- | --- | --- |
| 1 | log（spdlog） | 同步/异步、日志级别、轮转、格式 | 为什么异步日志？丢了怎么办？ |
| 2 | util（jsoncpp） | 序列化/反序列化、optional 返回值 | 为什么用 optional 而不是异常？ |
| 3 | redis | 连接池、string/hash/zset 选型、缓存更新 | 缓存穿透/击穿/雪崩、双写一致性 |
| 4 | odb | ORM 映射、事务、连接池、视图 | ODB vs 手写 SQL 的取舍 |
| 5 | rpc（brpc） | Channel、超时、重试、负载均衡 | brpc 为什么快？bthread 是什么？ |
| 6 | etcd | 租约保活、watch、服务发现 | 为什么不用 ZooKeeper？ |
| 7 | mq（RabbitMQ） | 交换机类型、TTL、死信队列、延迟消息 | 如何用 DLX 实现延迟队列？消息幂等 |
| 8 | elastic | 倒排索引、分词、查询 DSL | ES 和 MySQL like 的区别 |
| 9 | fdfs | Tracker/Storage、文件 ID | 为什么要用分布式文件系统 |
| 10 | hls/FFmpeg | 封装格式、HLS 切片、m3u8 | HLS 与 RTMP 区别、为什么点播用 HLS |
| 11 | 微服务 | 网关、服务拆分、配置、部署 | 一次请求怎么串起来？如何排障？ |

### 6.3 面试作品讲述模板（5 分钟版）

1. **背景**：一个 C++ 视频点播系统，微服务架构，支持上传、转码、播放、弹幕；
2. **我的职责**：脚手架封装 + 服务端 + 客户端全链路（可强调你重构的部分）；
3. **架构**：客户端 → 网关 → 3 个业务服务 → 中间件（画图）；
4. **难点**：① 延迟队列做缓存同步；② 转码异步化；③ 服务发现与容错；
5. **数据**：QPS / 缓存命中率 / 转码耗时（M8 补）；
6. **反思**：哪里设计得不好，如果重做会怎么改。

---

## 七、风险与决策记录

| 日期 | 决策 | 原因 |
| --- | --- | --- |
| 2026-10-18 | 合并为单仓库 `vod-system` | 课程三方分离，构建/联调成本高 |
| 2026-10-18 | 服务端统一 CMake | 5 份几乎相同的 CMakeLists，改一处要改五处 |
| 2026-10-18 | FFmpeg 走本地前缀 | 容器内无 root，不能 apt 安装 |
| 2026-10-18 | `data_test` 暂不参与构建 | 16 个独立工程，先保证主链路，M6 再改 gtest |

---

## 八、进度日志

### 2026-10-18（M0 / M1 / M2 完成，M3 进行中）

**仓库与构建**

- 导入 `server`（150 文件）、`client`（源码，剔除 build / mpv DLL / 测试视频）；
- 目录重构：`cpp-microservice-scaffold/source → scaffold/JianLiuScaffold`，`example → examples`；
- 新增 `scaffold/CMakeLists.txt`（11 模块 → `JianLiuScaffold::JianLiuScaffold`，自动探测依赖）；
- 重写 `server/CMakeLists.txt`：protoc + ODB 自定义命令 → `libvideoplayer.a` → 5 个服务；
- 统一 LF 换行 + `.gitattributes`；`.gitignore` 重写；
- **`cmake --build build` 通过，产出 5 个可执行文件**（含 FFmpeg 转码服务）。

**依赖**

- 无 root 安装 FFmpeg 成功：`apt-get download`（含依赖解析）+ `dpkg -x` → `~/local/ffmpeg`，
  运行时 `LD_LIBRARY_PATH`；脚本 `scripts/fetch-ffmpeg.sh` 固化流程；
- 补上 `cpprest`、`odb-boost` 两个隐藏链接依赖（链接期才暴露）。

**运行（M2）**

- MySQL 初始化：`deploy/sql/data.sql` → 13 张表（库 `vptest`）；
- Redis（AUTH 123456）/ etcd / RabbitMQ / FastDFS 连通性验证；
- `scripts/run-dev.sh` 启动 5 服务；etcd 注册 `/file`、`/user`、`/video`；
- 网关 watch 到服务上线/下线；`curl :10000/status` 正常。

**接口（M3 完成）**

```bash
curl -X POST http://127.0.0.1:10000/HttpService/tempLogin \
     -H 'Content-Type: application/json' -d '{"requestId":"R1"}'
# -> {"errorCode":0,"result":{"sessionId":"..."}}

curl -X POST .../HttpService/sessionLogin   # -> isGuest=true（未登录）/ false（已登录）
curl -X POST .../HttpService/allVideoList   # -> 视频列表
```

**端到端联调结果（2026-10-18）**

- 造数：SQL 直插普通用户 + 绑定会话（绕过邮件验证码），清 Redis 会话缓存；
- 链路：`tempLogin → sessionLogin(isGuest=false) → uploadPhoto(6480B) → downloadPhoto(字节一致)`
  → `uploadVideo(318KB) → newVideo → MQ 转码 → HLS(1 个 m3u8 + 2 个 ts 上传 FastDFS)`
  → `allVideoList（返回视频，playCount=1）→ setLike/judgeLike → newBarrage/getBarrage`；
- 转码后视频状态为「待审核(1)」，手工置为「上架(2)」后出现在首页列表（审核流程属 M5）。

**命名重构（品牌：笺流 / JianLiu）**

- `bite* → JianLiu*`：11 个命名空间（`JianLiuLog`、`JianLiuUtil`、`JianLiuRpc`、`JianLiuSvc`、
  `JianLiuMq`、`JianLiuRedis`、`JianLiuOdb`、`JianLiuIcs`、`JianLiuFdfs`、`JianLiuHls`、`JianLiuCode`）；
- 基础库 `bite_scaffold → JianLiuScaffold`（目录、target、头文件路径、CMake 选项 `JIANLIU_*`）；
- 客户端品牌：窗口标题与应用名改为「笺流」，资源图 `biteshipin.png → jianliushipin.png`。

**过程中修掉的问题**

1. `pplx::get_ambient_scheduler` 未定义 → 链接 `cpprest`；
2. `odb::boost::date_time` 未定义 → 链接 `odb-boost`；
3. `hls.h` 被 file 服务包含 → HLS 必须随 FFmpeg 一起启用（不再"可选关闭"）；
4. ES 不可用时 video 服务 `abort()` → 改为告警 + 搜索降级；
5. 转码服务不认识 redis/etcd 参数 → 启动脚本按服务裁剪 gflags。

---

## 九、下一步（交接）

1. **M4**：在 Windows 侧用 Qt 6.7 构建 BitPlayer，`config.ini` 指向网关 `http://<host>:10000`，跑登录/首页/播放；
2. **M5**：补管理员审核链路（checkVideo/saleVideo/haltVideo）、关注、搜索（待 ES 可用）；
3. 写 `docs/01-architecture.md`、`docs/02-api.md`（含本次联调的 curl 样例）；
4. 清理 `server/data_test`（16 个旧测试工程）→ 改为 gtest 用例。
