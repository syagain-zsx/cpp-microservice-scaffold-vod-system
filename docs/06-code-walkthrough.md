# 代码导读（新人/面试复习用）

> 目标：30 分钟内知道「代码在哪、请求怎么走、要改一个功能该动哪些文件」。

---

## 1. 先跑起来

```bash
./scripts/build.sh
./scripts/run-dev.sh start
./scripts/smoke-test.sh
python3 scripts/e2e-test.py --session <sid> --admin   # 端到端
```

日志在 `logs/`，网关状态页 `http://127.0.0.1:10000/status`。

---

## 2. 代码地图

| 目录 | 职责 | 先看哪个文件 |
| --- | --- | --- |
| `scaffold/JianLiuScaffold/` | 中间件封装，与业务无关 | `log.h` → `mq.cc` / `etcd.cc` |
| `server/proto/` | 接口与消息定义 | `http.proto`（网关对外接口） |
| `server/data/data.h` | 全部数据库实体 | 从 `class Video` 开始 |
| `server/data/*.cc` | DAO：DB + Redis + ES | `video.cc`、`session.cc` |
| `server/common/` | 错误码、缓存同步接口 | `error.h` |
| `server/svc_*/source/main.cc` | 进程入口：解析 gflags、组装依赖 | `svc_video/source/main.cc` |
| `server/svc_*/source/svc_server.cc` | Builder：建连接、起 brpc、注册 etcd | `VideoServerBuilder::build()` |
| `server/svc_*/source/svc_rpc.cc` | RPC 实现：校验、调 data、组响应 | `VideoServiceImpl::allVideoList` |
| `server/svc_*/source/svc_data.cc` | 业务编排（跨 DAO） | `SvcData::newVideo` |
| `server/svc_*/source/svc_sync.cc` | 缓存同步/MQ 回调 | `CacheToDB::callback` |
| `client/BitPlayer/` | Qt 客户端 | `netclient/netclient.cpp` |

---

## 3. 三条阅读主线

### 3.1 登录（最简单的 RPC 链路）

```text
proto/http.proto: rpc tempLogin
 → svc_gateway/source/svc_rpc.cc: HttpServiceImpl::tempLogin
   → SvcChannels::getNode("user")            （etcd 维护的节点）
   → brpc stub.tempLogin
     → svc_user/source/svc_rpc.cc: UserServiceImpl::tempLogin
       → SvcData::tempLogin()                （生成 session_id）
         → data/session.cc: SessionData::insert   （MySQL + Redis）
```

跟着这条线，能看到：网关转发模式、Builder 注入、DAO 双写、错误码转换。

### 3.2 上传 → 转码（异步链路）

```text
uploadVideo（HTTP 原始 body）
 → gateway: 取 file 节点地址，HTTP POST /FileService/uploadVideo
   → file: FDFSClient::upload_from_buff → 写 tbl_file_meta
newVideo
 → gateway → video: SvcData::newVideo → VideoData::insert → Publisher(hls_transcode_queue)
transcode 消费
 → svc_transcode/source/svc_mq.cc: TranscodeMQ::transcode
   → FDFS 下载 → JianLiuHls::HLSTranscoder::transcode → 上传 ts/m3u8 → 更新 video
```

### 3.3 列表 / 缓存（读链路）

```text
allVideoList
 → video: SvcData::getMainPageList
   → VideoIdListData::getMainPageList   （Redis 缓存 id 列表，未命中查 DB）
   → getDetailVideoList                 （每个 id 取 Video 缓存/DB + 用户信息）
 → 组装 VideoInfo 列表
```

看这条线时，重点理解 `VideoData::get()` 的 cache-aside 与 `_cache_delete_sync` 的作用。

---

## 4. 常见改动场景

### 加一个 HTTP 接口

1. `proto/http.proto` 里 `HttpService` 加 rpc + 请求/响应消息；
2. `svc_gateway/source/svc_rpc.cc` 实现（复制相邻方法，改 RPC 调用）；
3. 子服务 `proto/*.proto` 加服务方法，`svc_*/source/svc_rpc.cc` + `svc_data.*` 实现；
4. `tbl_operator_role_meta` 加一行 URL→角色（否则网关鉴权会拒绝）；
5. `cmake --build build`（proto 自动重新生成）。

### 加一张表

1. `data/data.h` 加实体（`#pragma db object table(...)`）；
2. `data/xxx.cc/h` 加 DAO；
3. 重新构建，生成的 `build/server/generated/data.sql` 即建表语句；
4. 在 `deploy/sql/data.sql` 同步一份（或直接用生成文件）。

### 加一个子服务

1. 复制 `svc_user/` 目录改名，改 `main.cc` 的 flags 与 `svc_server.cc` 的 Builder；
2. `server/CMakeLists.txt` 的 `VOD_SERVICES` 加名字（构建与 install 自动生效）；
3. `run-dev.sh` 加启动分支；`docker-compose.yml` 加一个 service。

---

## 5. 调试技巧

| 场景 | 手段 |
| --- | --- |
| 请求没到子服务 | 网关日志 + `curl 子服务:端口/status` 看 QPS 计数 |
| 服务没被发现 | etcd v3 API 查 key；网关日志看「上线/下线节点」 |
| 缓存不对 | Redis `KEYS 前缀*` / 看 `del` 消息日志 |
| MQ 不消费 | 看 `订阅队列消息成功` 日志、队列积压（管理台） |
| 数据库报错 | `mysql` 客户端看表结构；ODB 异常会打 `what()` |
| 进程挂了 | `logs/*.log` 末尾、`core`、`dmesg`（OOM） |

---

## 6. 建议的阅读顺序

1. `scaffold/JianLiuScaffold/*.h`（先看接口，不看实现）；
2. `proto/http.proto`（接口全貌）；
3. `svc_gateway/source/svc_rpc.cc` 的一个方法（转发模式）；
4. `svc_video/source/svc_rpc.cc` + `svc_data.cc`（业务实现）；
5. `data/video.cc`（缓存 + ORM）；
6. `svc_transcode/source/svc_mq.cc`（异步消费）；
7. 回头再看 `svc_server.cc` 的 Builder（依赖装配）。
