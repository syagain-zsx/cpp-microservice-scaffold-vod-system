# 笺流（JianLiu）· 架构设计

> 本文说明系统由哪些进程组成、代码如何分层、数据如何流转，以及每个中间件在项目里承担的角色。
> 接口细节见 [02-api.md](02-api.md)，路线图见 [ROADMAP.md](ROADMAP.md)。

---

## 1. 进程视图

| 进程 | 端口 | 职责 | 依赖 |
| --- | --- | --- | --- |
| `gateway_server` | 10000 | HTTP 入口：参数/会话校验、鉴权、把请求转发给子服务、聚合响应 | etcd、MySQL、Redis |
| `file_server` | 10001 | 图片/视频文件的上传下载，FastDFS 读写，文件元信息落库 | FastDFS、MySQL、Redis、MQ |
| `user_server` | 10002 | 会话、验证码/密码登录、用户资料、关注、管理员身份 | MySQL、Redis、MQ、SMTP |
| `video_server` | 10003 | 视频元信息、列表、点赞、播放计数、弹幕、审核、搜索 | MySQL、Redis、MQ、ES |
| `transcode_server` | 无 | 消费转码消息：下载源文件 → FFmpeg 切片 → 上传 HLS → 更新状态 | MySQL、MQ、FastDFS、FFmpeg |

网关只做「入口 + 路由 + 鉴权」，业务逻辑全部下沉到子服务；子服务之间通过 brpc 直连（地址来自 etcd），
文件大数据走 HTTP 透传（网关 → 文件服务）。

```text
客户端 ──HTTP/JSON──▶ gateway ──brpc──▶ user / video / file
                        │
                        └──HTTP(protobuf attachment)──▶ file（大文件）
transcode ◀──MQ── video
```

---

## 2. 代码分层

```text
scaffold/JianLiuScaffold/     基础设施层：对中间件客户端做统一封装（与业务无关）
        log util rpc etcd mq redis odb elastic fdfs hls mail

server/common/                错误码（vp_error）、缓存同步抽象（vp_sync::CacheSync）
server/data/                 数据访问层：ODB 实体 + DAO + Redis 缓存 + ES 检索
server/proto/                接口定义（6 个 proto，生成 C++ 代码）
server/svc_*/                服务装配层：
        main.cc               解析 gflags、组装依赖
        svc_server.*          构建器（Builder）：创建数据库/缓存/MQ/注册中心并启动
        svc_rpc.*             RPC 实现：参数校验、调用 data 层、组织响应
        svc_data.*            业务编排：跨 DAO 组合、权限、状态机
        svc_mq.* / svc_sync.* 消息生产/消费、缓存同步
client/BitPlayer/            Qt6 客户端（UI + netclient + model）
```

**依赖方向**：`svc_*` → `data` → `JianLiuScaffold`；`data` 不感知 RPC，`svc_*` 不直接写 SQL。

### 请求在代码里的路径（以 `newVideo` 为例）

```text
BitPlayer::uploadVideo 页面
  → NetClient::newVideoAsync            HTTP POST /HttpService/newVideo
    → gateway/svc_rpc.cc: HttpServiceImpl::newVideo
      → gateway/svc_data.cc: judgePermission()          查 URL→角色 + 会话角色
      → biterpc channel 选一个 video 节点
        → video/svc_rpc.cc: VideoServiceImpl::newVideo
          → video/svc_data.cc: SvcData::newVideo()      生成 video_id、写库、发 MQ
            → data/video.cc: VideoData::insert()         MySQL + Redis
            → data/video_search.cc: insert()             ES（不可用则降级）
            → Publisher::publish()                       hls_transcode_queue
```

---

## 3. 数据模型

### 3.1 MySQL（`vptest`，13 张表，由 ODB 从 `server/data/data.h` 生成）

| 表 | 说明 |
| --- | --- |
| `tbl_user_meta` | 用户基础信息（邮箱/昵称/密码/状态/头像） |
| `tbl_user_identify_role_meta` | 用户身份 ↔ 角色（C 端/B 端/管理员） |
| `tbl_identify_meta`、`tbl_role_meta` | 身份、角色字典 |
| `tbl_operator_meta`、`tbl_operator_role_meta` | 接口 URL ↔ 允许角色（网关鉴权用） |
| `tbl_session_meta` | 会话（游客会话 user_id 为空） |
| `tbl_file_meta` | 文件元信息（file_id、FastDFS 路径、大小、MIME、上传者） |
| `tbl_video_meta` | 视频（标题/简介/封面/视频文件/时长/状态/播放数） |
| `tbl_video_catgory_tag_meta` | 视频 ↔ 分类/标签 |
| `tbl_video_like_meta` | 点赞关系 |
| `tbl_follow_meta` | 关注关系 |
| `tbl_barrage_meta` | 弹幕 |

### 3.2 Redis（缓存键）

| 前缀 | 内容 | 失效方式 |
| --- | --- | --- |
| `vp_session_<sid>` | 会话（含 user_id） | 退出登录时删除 |
| `cahce_video_<vid>` | 视频详情 JSON（注意：历史拼写为 `cahce`） | 延迟队列 3s 后删 |
| `video_count_<vid>` | 播放数/点赞数 | 延迟队列 30s 回写 MySQL |
| `user_count_<uid>` | 用户统计（获赞/关注/粉丝） | 同上 |
| `permission_role_<url>` | URL→角色 | 定时/按需 |
| `user_identify_role_<uid>` | 用户身份角色 | 变更时删除 |

### 3.3 消息队列（RabbitMQ）

| Exchange | 类型 | 队列 | 用途 |
| --- | --- | --- | --- |
| `hls_transcode_exchange` | direct | `hls_transcode_queue` | 视频转码任务 |
| `cache_sync_to_db_exchange` | delayed（TTL=30s + DLX） | `cache_sync_to_db_queue` | 缓存计数异步回写 MySQL |
| `delete_cache_exchange` | delayed（TTL=3s + DLX） | `delete_cache_queue` | 删除缓存 |
| `delete_file_exchange` | direct | `delete_file_queue` | 文件服务异步删除 FastDFS 文件 |

延迟队列的实现：声明一个 `x-delayed-message` 类型的交换机（需要 `rabbitmq_delayed_message_exchange` 插件），
或使用「普通队列 + TTL + 死信交换机」；`JianLiuMq` 的 `declare()` 同时创建配套的 `dlx_*` 交换机/队列。

### 3.4 注册中心（etcd）

- 键：`/<service_name>/<instance_id>`，值：`<host>:<port>`，绑定租约（KeepAlive 自动续约）；
- 网关启动时 `watch` 前缀 `/`，收到 put/delete 事件后维护本地 `服务名 → 节点列表` 映射；
- 节点选择用轮询（`JianLiuRpc::Channels::select()`）。

---

## 4. 关键流程

### 4.1 登录

```text
客户端               网关               user 服务             MySQL/Redis/邮件
  │ tempLogin  ───────▶ │ ──── RPC ────▶ │ 建会话(游客) ──────▶ │
  │ ◀── sessionId ───── │ ◀──────────── │                      │
  │ getCode(email) ────▶ │ ──── RPC ────▶ │ 生成验证码 ────────▶ │ Redis + SMTP
  │ vcodeLogin ────────▶ │ ──── RPC ────▶ │ 校验验证码、注册/登录 ─▶ │
  │ ◀── 成功 ─────────── │ ◀──────────── │ 绑定 session.user_id  │
```

### 4.2 上传 → 转码 → 播放

```text
1. uploadVideo  ──▶ 网关 ──HTTP──▶ file ──▶ FastDFS  (返回 file_id)
2. newVideo     ──▶ 网关 ──RPC───▶ video ──▶ MySQL (status=待审核)
                                      └──▶ MQ: hls_transcode_queue
3. transcode 消费 ──▶ FDFS 下载源文件 ──▶ FFmpeg 切片 ──▶ 上传 m3u8+ts ──▶ 更新 video.video_fid
4. 管理员 checkVideo 审核通过 ──▶ status=上架 ──▶ 进入 allVideoList
5. 客户端 downloadVideo ──▶ 拿到 m3u8 ──▶ 逐个 downloadVideo(ts) ──▶ mpv 播放
```

### 4.3 缓存一致性（写回）

- 读：先查 Redis，未命中查 MySQL 并回填；
- 写：先写 MySQL，再删缓存 / 更新缓存；计数类（播放、点赞）只改 Redis 并把 key 投递到
  `cache_sync_to_db_queue`（延迟 30s），由消费者批量回写 MySQL，降低数据库写压力；
- 删除：`delete_cache_exchange`（延迟 3s）保证「先删库、后删缓存」的顺序。

---

## 5. 中间件封装（`JianLiuScaffold`）

| 头文件 | 能力 | 关键实现点 |
| --- | --- | --- |
| `log.h` | 同步/异步日志、级别、格式 | spdlog + `DBG/INF/WRN/ERR` 宏带文件行号 |
| `util.h` | JSON 序列化、文件读写、字符串切分、随机串 | jsoncpp；失败返回 `std::optional` |
| `rpc.h` | brpc 信道池、轮询选节点、异步回调工厂、服务端工厂 | `Channels`/`SvcChannels`/`ClosureFactory` |
| `etcd.h` | 服务注册（租约保活）、服务发现（watch 前缀） | `SvcProvider`、`SvcWatcher` |
| `mq.h` | RabbitMQ 客户端、声明交换机/队列、发布/订阅、延迟队列 | libev + amqpcpp，声明同步等待 |
| `redis.h` | redis++ 连接池工厂 | `ConnectionPoolOptions` |
| `odb.h` | ODB + MySQL 连接池工厂 | `connection_pool_factory` |
| `elastic.h` | ES 索引/文档/查询 DSL 构建器 | elasticlient + cpr |
| `fdfs.h` | FastDFS 上传/下载/删除 | `fdfs_client_init_from_buffer`，免配置文件 |
| `hls.h` | m3u8 解析/写出、FFmpeg 转码 | 只做 remux（`avcodec_parameters_copy`），不重编码 |
| `mail.h` | 邮件验证码发送 | libcurl + SMTP over SSL |

---

## 6. 容错与降级

| 场景 | 行为 |
| --- | --- |
| Elasticsearch 不可用 | `VideoSearchData::init` 捕获异常 → `_enabled=false`，搜索返回空，其余业务正常（已实现） |
| 子服务下线 | etcd watch 移除节点；网关请求返回 `SERVICE_NOT_FOUND` |
| MQ 不可用 | 启动阶段声明失败会阻塞；运行期发布失败返回错误码 |
| 数据库连接失败 | 启动即失败（快速暴露配置问题） |
| 转码失败 | 视频状态置「转码失败(6)」，源文件保留 |

---

## 7. 已知问题 / 演进方向

1. **ES 缺失**：当前环境没有 Elasticsearch，搜索是降级状态；补上后需要重建 `video` 索引；
2. **密码**：`tbl_user_meta.password` 当前明文/简单处理，应改为加盐哈希（M6）；
3. **`cahce_video_` 拼写**：历史遗留，重命名需要迁移缓存，暂不动；
4. **转码状态机**：转码完成后直接进「待审核」，缺少失败重试与超时处理；
5. **可观测性**：目前只有日志，缺 metrics/trace（可作为 M8 优化项）；
6. **鉴权**：URL→角色表是手工维护，接口新增容易漏配。
