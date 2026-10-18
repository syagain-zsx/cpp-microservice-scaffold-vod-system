# RabbitMQ 延迟队列（本项目实现）

> 代码：`scaffold/JianLiuScaffold/mq.cc`（客户端封装）、`server/svc_*/source/svc_*.cc`（业务队列）、
> `server/svc_video/source/svc_sync.cc`（缓存回写）、`server/svc_transcode`（转码消费）

---

## 1. 原理速览

### 1.1 消息队列解决什么

- **解耦**：上传视频不需要等转码完成，投一条消息即可返回；
- **削峰**：转码是重任务，用队列控制消费速度；
- **异步**：缓存回写、文件删除等非关键路径异步做。

### 1.2 延迟消息的两种实现

| 方案 | 原理 | 优点 | 缺点 |
| --- | --- | --- | --- |
| 插件 `x-delayed-message` | 交换机保存消息，到点再投递 | 简单、任意延迟 | 需要安装 `rabbitmq_delayed_message_exchange` 插件 |
| **DLX + TTL（本项目）** | 普通队列设置 `x-message-ttl`，过期后死信转发到死信交换机/队列 | 原生支持、无需插件 | 只能固定 TTL；队头阻塞（先入队的先过期） |

### 1.3 DLX + TTL 的结构

```text
生产者 ──publish──▶ [exchange] ──binding──▶ [queue]  (x-message-ttl=30000,
                                                 x-dead-letter-exchange=dlx_exchange,
                                                 x-dead-letter-routing-key=dlx_key)
                                                    │ TTL 到期
                                                    ▼
                                          [dlx_exchange] ──▶ [dlx_queue]
                                                                  │
消费者 ◀───────────────────── consume(dlx_queue) ─────────────────┘
```

---

## 2. 本项目怎么做的

### 2.1 四类队列

| Exchange | 类型 | 队列 | TTL | 用途 | 生产/消费 |
| --- | --- | --- | --- | --- | --- |
| `hls_transcode_exchange` | direct | `hls_transcode_queue` | — | 转码任务 | video 发布 / transcode 消费 |
| `delete_file_exchange` | direct | `delete_file_queue` | — | 删除 FastDFS 文件 | video 发布 / file 消费 |
| `delete_cache_exchange` | delayed | `delete_cache_queue` | 3s | 延迟删缓存 | video/user 发布 / 同服务消费 |
| `cache_sync_to_db_exchange` | delayed | `cache_sync_to_db_queue` | 30s | 计数回写 MySQL | video 发布 / video 消费 |

配置在 `deploy/conf/global.conf` 的 `-*_queue_info`（JSON），运行时由 `declare_settings` 解析。

### 2.2 封装的关键逻辑

`JianLiuMq::MQClient::declare(settings)`：

```cpp
if (settings.exchange_type == DELAYED) {
    _declared(settings, args, true);              // 先声明 dlx_ 交换机/队列/绑定
    args[DEAD_LETTER_EXCHANGE]  = settings.dlx_exchange();
    args[DEAD_LETTER_BINDING_KEY] = settings.dlx_binding_key();
    args[MESSAGE_TTL] = settings.delayed_ttl;     // 毫秒
}
_declared(settings, args, false);                 // 再声明业务交换机/队列
```

- `Publisher::publish()` **永远发到业务交换机**（不区分是否延迟）；
- `Subscriber::consume()` 对延迟队列**订阅 `dlx_` 队列**，普通队列订阅自身；
- 声明是同步等待的：`condition_variable` + 5s 超时（防止 RabbitMQ 异常时永久挂起）。

### 2.3 消息体

用 protobuf 定义（`server/proto/message.proto`）：

| 消息 | 字段 | 用途 |
| --- | --- | --- |
| `HLSTranscodeMsg` | video_id, m3u8_name, ts_name… | 转码任务 |
| `DeleteCacheMsg` | repeated key | 删除缓存 |
| `CacheToDBMsg` | repeated video_id | 计数回写 |
| `DeleteFileMsg` | repeated file_id | 删除文件 |

### 2.4 可靠性现状（要能说出不足）

- 发布：`channel.publish()`，**未开启 publisher confirm**，极端情况可能丢；
- 队列：默认非持久化声明（课程代码），重启 RabbitMQ 会丢队列；
- 消费：手动 `ack`，处理完才确认；异常会记录日志，但没有重试/死信兜底；
- 幂等：转码按 video_id 覆盖产物、计数回写是「覆盖写」，重复消费影响可控。

改进方向：confirm + 持久化 + 失败重试队列 + 幂等表。

---

## 3. 面试问答

**Q：为什么不用延时线程/定时任务？**
定时任务轮询 DB 有延迟和空转；延迟队列由 MQ 精确投递，且天然支持削峰与重试。

**Q：TTL 队列的队头阻塞问题？**
RabbitMQ 只检查队头消息是否过期，若队头 TTL 很长会阻塞后面的短 TTL 消息。
解决：不同延迟拆不同队列，或用 `x-delayed-message` 插件。

**Q：消息重复消费怎么处理？**
业务幂等：转码按视频 id 覆盖、计数用当前值覆盖写、删除文件重复删无害；
更严格用「消息 id + 去重表」或 Redis SETNX。

**Q：消息丢失怎么排查？**
确认三处：生产端是否到达交换机（confirm/return）、队列是否持久化、消费端是否 ack 前崩溃。
管理台看队列的 publish/deliver/ack 计数。

**Q：为什么消费者订阅的是 dlx 队列？**
延迟消息的「到点」表现为：消息在业务队列过期 → 被投递到死信交换机 → 死信队列。
所以消费者要监听死信队列，而不是业务队列。

---

## 4. 动手实验

1. 把 `cache_del_queue_info` 的 `delayed_ttl` 从 3 改到 10，重启 video，触发一次更新，观察日志时间差；
2. 停掉 transcode，上传并创建视频，消息会积压在 `hls_transcode_queue`；看 video 仍返回成功（异步解耦）；
3. 重启 transcode，观察积压消息被消费、HLS 生成；
4. 在 `mq.cc` 的 `_declared` 里临时改错交换机类型，观察 5s 超时保护是否生效（会 abort 并打日志）。
