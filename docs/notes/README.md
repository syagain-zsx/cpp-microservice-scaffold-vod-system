# 学习笔记 · 索引

> 用法：每个主题按「原理 → 本项目实现 → 面试问答 → 动手实验」的顺序过一遍，
> 然后合上文档，试着在纸上画出链路、说出取舍。

| 笔记 | 主题 | 对应代码 |
| --- | --- | --- |
| [01-redis-cache.md](01-redis-cache.md) | Redis 缓存设计、延迟双删、计数回写 | `server/data/video.cc`、`server/svc_video/source/svc_sync.cc` |
| [02-rabbitmq-delay.md](02-rabbitmq-delay.md) | RabbitMQ 延迟队列（DLX + TTL） | `scaffold/JianLiuScaffold/mq.cc`、`server/svc_transcode` |
| [03-brpc-etcd.md](03-brpc-etcd.md) | 服务注册发现与 RPC | `JianLiuScaffold/rpc.cc`、`etcd.cc` |
| [04-odb.md](04-odb.md) | ODB 实体/事务/连接池 | `server/data/data.h`、`JianLiuScaffold/odb.cc` |
| [05-hls-ffmpeg.md](05-hls-ffmpeg.md) | HLS 切片与 FFmpeg remux | `JianLiuScaffold/hls.cc`、`svc_transcode` |

## 通用学习方法

1. **先跑起来**：`./scripts/run-dev.sh start` + `./scripts/smoke-test.sh`，看日志；
2. **读一条链路**：从 `svc_gateway/source/svc_rpc.cc` 的某个接口出发，跟到 `data/` 再到中间件；
3. **改一个点**：比如把 `hls_time` 改成 5、把延迟队列 TTL 改成 10s，观察行为；
4. **写出来**：用自己的话写 3 个问答，放进对应笔记的「面试问答」里。
