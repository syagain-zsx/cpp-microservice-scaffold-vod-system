# 第 4 周 · 异步链路（MQ → transcode → HLS → 审核）

> 时长：3 次 × 2 小时（约 8–10 小时）
> 产出：转码时序图（标 4 个队列）、hls_time 改动 diff、失败分支 diff、10 分钟讲解录音

## 本周目标（可验收）

- [ ] 能完整讲清“上传 → newVideo(status=5) → MQ → 转码 → status=1 → 审核 → 2”的链路；
- [ ] 能说出 4 个队列的 exchange/queue/类型/TTL；
- [ ] 能手工触发一次转码并从日志、m3u8 验证结果；
- [ ] 能给“源文件不存在”加失败分支，且不影响正常链路；
- [ ] 完成 **CP2**（见 [checkpoints.md](checkpoints.md)）。

## 前置

`dev-storage:23000` 必须是通的（上传/转码依赖 FastDFS storage）。检查方法见 [cheatsheet.md](cheatsheet.md#4-学习前的环境自检)。

## 阅读清单

| 文件 | 看什么 |
| --- | --- |
| `scripts/run-dev.sh` | 四段队列 JSON（delete_file / delete_cache / cache_sync_to_db / hls_transcode） |
| `scaffold/JianLiuScaffold/mq.cc` | 声明、发布、订阅、延迟队列、超时降级 |
| `server/svc_transcode/source/svc_mq.cc` | 消费 → 下载 → 转码 → 上传 → 更新状态 |
| `scaffold/JianLiuScaffold/hls.cc` | libavformat remux、`hls_time`、m3u8 |
| `scaffold/JianLiuScaffold/fdfs.cc` | 上传/下载/删除 |
| `server/svc_video/source/svc_data.cc` `newVideo` / `checkVideo` | 状态机与 MQ 投递 |
| `server/svc_file/source/svc_sync.cc` | 删除缓存延迟消息 |
| `docs/notes/02-rabbitmq-delay.md`、`05-hls-ffmpeg.md` | 原理 + 实现 |
| `docs/diagrams/jianliu-vod-e2e.html` | 端到端流程图 |

## 时间安排

| 次 | 内容 |
| --- | --- |
| 第 1 次（读） | 45min 读 MQ/转码；30min 画时序；45min 触发一次转码 |
| 第 2 次（改） | 30min 设计 hls_time 与失败分支；60min 改 + 验证；30min 记录 |
| 第 3 次（讲） | 30min 自测；45min 讲链路（CP2）；45min 补漏 |

## 动手任务

### 任务 1：画时序并标出 4 个队列（必做）

从 `run-dev.sh` 的 `DEL_FILE_Q / DEL_CACHE_Q / SYNC_CACHE_Q / TRANSCODE_Q` 抄出四个队列，填表：

| 队列 | 类型 | TTL | 谁发布 | 谁消费 |
| --- | --- | --- | --- | --- |
| delete_file_queue | direct | 无 | | file |
| delete_cache_queue | delayed | 3s | | file / video |
| cache_sync_to_db_queue | delayed | 30s | | video |
| hls_transcode_queue | direct | 无 | video | transcode |

**验收**：能解释“为什么删除缓存也要走延迟队列”“为什么计数回写延迟 30s”。

### 任务 2：触发一次转码并验证（必做）

用 e2e（会自动上传一个视频并等 m3u8）：

```bash
SID=$(curl -s -X POST http://127.0.0.1:10000/HttpService/tempLogin \
  -H 'Content-Type: application/json' -d '{"requestId":"W4-1"}' \
  | python3 -c 'import sys,json;print(json.load(sys.stdin)["result"]["sessionId"])')
curl -s -X POST http://127.0.0.1:10000/HttpService/passwdLogin \
  -H 'Content-Type: application/json' \
  -d "{\"requestId\":\"W4-2\",\"sessionId\":\"$SID\",\"username\":\"tester@example.com\",\"password\":\"TestPass123\"}" >/dev/null
python3 scripts/e2e-test.py --session "$SID" --admin
```

然后：

```bash
tail -20 logs/transcode.log
# 从日志拿到 video_id / m3u8 fileId，用 curl 看分片
curl -s 'http://127.0.0.1:10000/HttpService/downloadVideo?fileId=<m3u8_fileId>'
```

**验收**：transcode 日志出现“转码成功”；m3u8 里有 `#EXTINF` 分片；能解释状态从 5 → 1。

### 任务 3：改 `hls_time` 观察分片（必做）

- 在 `scripts/run-dev.sh` 把 transcode 的 `--hls_time=10` 改成 `5`；
- `./scripts/run-dev.sh start transcode`；
- 再跑一次 e2e（约 10s 的测试视频），curl m3u8 数 `#EXTINF` 的个数；
- 改回 10 并重启。

**验收**：记录改动前后分片数差异；能解释 `hls_time` 与切片数的关系。

### 任务 4：给“源文件不存在”加失败分支（必做）

在 `svc_transcode/source/svc_mq.cc` 的消费逻辑里，下载/打开源文件失败时：

- 记录 `ERR`（带 video_id 与原因）；
- 不更新 `video_fid`，状态保持 5（转码中）；
- 不删除源文件、不生成空 m3u8。

**验收**：手工构造一次失败（把源文件 fileId 改成不存在，或临时停 storage），日志明确、状态不变；恢复后正常链路仍通过 e2e 18/18。

## 常见坑

1. MQ 声明超时（30s）会拖慢启动；本项目已改为“降级运行不退出”，但异步链路会暂时不可用。
2. 转码是 **remux 不转码**：源文件编码不支持时不会失败，但播放端可能解不了。
3. e2e 等待 m3u8 有轮询超时；storage 没起时会一直失败。
4. 重复消费：转码任务没有幂等键，重投递会重复生成文件（面试可讨论）。
5. 审核通过后最多 3s 才在首页可见（列表缓存）。

## 面试题（本周 5 个）

1. 为什么要用 MQ 把转码异步化？同步做会怎样？
2. 用 DLX + TTL 实现延迟队列的原理？消息积压怎么办？
3. 如何保证消息不丢？publisher confirm / 消费 ack / 重试分别解决什么？
4. 转码失败了怎么办？状态机怎么设计重试与补偿？
5. HLS 和 MP4/RTMP 的区别？为什么点播选 HLS？

## 复盘模板

```text
四个队列：……
hls_time 改动前后分片数：____ → ____
失败分支怎么验证的：……
我还讲不清的：……
```
