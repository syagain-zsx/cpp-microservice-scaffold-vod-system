# 代码阅读主线

> 目标：**一次只追一条请求**。每追完一条，就写一张调用链表（组件 + 文件 + 关键函数），以后面试前只看这些表。
> 不要从 `svc_rpc.cc` 第一行读到第 1496 行——先挑一条链路。

## 0. 开始之前

先把服务跑起来，边跑边读：

```bash
./scripts/run-dev.sh start
./scripts/smoke-test.sh
tail -f logs/gateway.log
```

日志里的 `[xx:xx:xx][debug] 收到xxx请求...` 是阅读时最好的锚点：发一次请求，看日志出现了哪几行。

## 1. 入口：客户端怎么发出第一个请求

| 文件 | 看什么 |
| --- | --- |
| `client/JianLiuPlayer/main.cpp` | 单实例（QSharedMemory）、启动页、主窗口 |
| `client/JianLiuPlayer/startuppage.cpp` | 启动页如何等 `tempLogin/sessionLogin` 成功才关闭 |
| `client/JianLiuPlayer/netclient/netclient.cpp` 的 `tempLogin`/`sendHttpRequest` | HTTP 请求怎么拼、JSON 怎么解析、信号怎么发 |
| `client/JianLiuPlayer/model/datacenter.cpp` | `serverUrl`、`sessionId`、用户信息从哪来 |

要回答的问题：`config.ini` 在哪读？`HTTP_URL` 怎么拼出来？为什么启动页要等登录成功？

## 2. 网关：HTTP 如何变成 brpc RPC

| 文件 | 看什么 |
| --- | --- |
| `server/svc_gateway/source/svc_server.cc` | brpc Server 启动、注册了哪些服务、端口 |
| `server/svc_gateway/source/svc_rpc.cc` L17 `tempLogin` | 最小 HTTP handler：取 sessionId/参数 → 找 user 服务 → 转发 → 组 JSON |
| 同一文件 L855 `downloadVideo` | 文件下载：免会话鉴权，fileId 即凭证 |
| 同一文件 L1349 `allVideoList` | 列表：鉴权 + 路由 + 转发 |
| `server/proto/http.proto`、`video.proto` | 对外接口与消息字段 |

要回答的问题：网关怎么知道 user 服务在哪？为什么 handler 里要有 `brpc::ClosureGuard done_guard(done)`？

## 3. 服务发现：etcd + brpc 信道

| 文件 | 看什么 |
| --- | --- |
| `scaffold/JianLiuScaffold/etcd.cc` L36-L55 | 10s 租约、put 注册、KeepAlive、watch、5s 周期 resync |
| `scaffold/JianLiuScaffold/rpc.cc` | 节点管理、`getNode`/`getNodeAddr`、轮询负载均衡 |
| `server/svc_user/source/svc_server.cc` | Builder：建 MySQL/Redis 连接、起 brpc、注册 etcd |

要回答的问题：为什么 watch 要先于 list？实例地址怎么去重？kill -9 之后节点多久消失？

## 4. 业务：一个接口背后的三段

以 `allVideoList` 为例：

```text
gateway::allVideoList → video 服务::allVideoList → svc_data::getMainPageList → DAO/Redis/MySQL
```

| 文件 | 看什么 |
| --- | --- |
| `server/svc_video/source/svc_rpc.cc` | 参数校验、错误码、组响应 |
| `server/svc_video/source/svc_data.cc` L468 `getMainPageList` | 3s 缓存、回源、缓存写入 |
| `server/data/data.h` | 13 张表的实体（先看 `Video`） |
| `server/data/video.cc`、`video_idlist.cc` | DAO：ODB 查询 + Redis 缓存 |
| `server/data/session.cc` | 会话与用户校验 |

要回答的问题：缓存 key 是什么？什么时候写缓存？分页参数怎么变成 SQL？

## 5. 异步：上传 → 转码 → HLS → 审核

```text
client 上传 → file 服务落 FastDFS → newVideo(status=5) → MQ 投递
      → transcode 消费 → remux 切片 → 上传 ts/m3u8 → 更新 video_fid + status=1
```

| 文件 | 看什么 |
| --- | --- |
| `server/svc_file/source/svc_rpc.cc` | 图片/视频上传、file_meta |
| `server/svc_video/source/svc_data.cc` `newVideo` | 建视频记录、投递 MQ |
| `scaffold/JianLiuScaffold/mq.cc` | 四类交换机/队列、延迟队列 DLX+TTL、声明超时降级 |
| `server/svc_transcode/source/svc_mq.cc` | 消费、下载、转码、上传、更新状态、删源文件 |
| `scaffold/JianLiuScaffold/hls.cc` | libavformat remux、`hls_time`、m3u8 |
| `scaffold/JianLiuScaffold/fdfs.cc` | 上传/下载/删除 |
| `server/svc_video/source/svc_sync.cc` + `server/data/*` | 计数回写、缓存失效 |

要回答的问题：为什么用两个队列做延迟？转码失败状态停在哪？为什么审核通过后最多 3s 才可见？

## 6. 中间件封装（按需读）

| 文件 | 一句话 |
| --- | --- |
| `scaffold/JianLiuScaffold/log.h` | spdlog 封装、错误 handler |
| `redis.cc` | 连接池、string/hash 操作 |
| `odb.cc` | 连接池、事务工厂 |
| `elastic.cc` | ES 客户端 + 降级开关 |
| `hls.cc` / `fdfs.cc` | 转码与文件存储 |

## 7. 客户端再深入（第 5 周）

| 文件 | 看什么 |
| --- | --- |
| `netclient/netclient.cpp` | 每个 `xxxAsync` 对应一个 HTTP 接口；上传/下载 |
| `model/datacenter.cpp` | 全局状态（session、用户、列表） |
| `playerpage.cpp` | 构造 m3u8 URL、播放进度、弹幕 |
| `uploadvideopage.cpp` | 上传流程、封面截帧、提交 |
| `mpv/mpvplayer.cpp` | libmpv 初始化、事件循环、截帧 |
| `playpage`/`videobox` | 列表与详情 UI |

要回答的问题：播放 URL 怎么拼？为什么 `downloadVideo` 不需要 sessionId？

## 8. 测试与脚本（第 6 周）

| 文件 | 看什么 |
| --- | --- |
| `server/tests/*.cc` | 8 个 gtest 覆盖了什么 |
| `scripts/smoke-test.sh` | 4 项冒烟 |
| `scripts/e2e-test.py` | 18 项端到端（上传→转码→HLS→审核→互动） |
| `scripts/bench.py` | 压测方法与指标 |
| `deploy/Dockerfile`、`docker-compose.yml`、`conf/global.conf` | 部署方式与配置注入 |

## 9. 三条链路的“一张表”（读完后自己默写）

```text
登录：  client → gateway/tempLogin → user → MySQL(session) → 返回 sessionId
列表：  client → gateway/allVideoList → video → Redis(3s) → MySQL → 返回列表
上传：  client → gateway/uploadVideo → file → FastDFS → newVideo → MQ
播放：  client → gateway/downloadVideo?fileId=m3u8 → file → FastDFS → mpv
```

## 不要做的事

- 不要一开始读 `svc_rpc.cc` 全部 40+ 接口，只看入口函数；
- 不要背代码，记“数据从哪来、到哪去、失败会怎样”；
- 不要跳过运行直接读文档——日志会告诉你真实顺序。
