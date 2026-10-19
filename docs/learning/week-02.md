# 第 2 周 · 服务注册与发现（etcd + brpc）

> 术语看不懂？先查 [glossary.md](glossary.md)（租约、watch、resync、Channel 等都有大白话解释）。

> 时长：3 次 × 2 小时（约 6–8 小时）
> 产出：etcd 节点观察记录、lease TTL 改动 diff、上下线时序图、5 分钟讲解录音

## 本周目标（可验收）

- [ ] 能说清服务“上线/下线”时 etcd 里发生了什么；
- [ ] 能解释为什么 watch 要先于 list、为什么还要 5s 周期 resync；
- [ ] 能改 lease TTL 并观察下线时间变化；
- [ ] 完成 **CP1**（见 [checkpoints.md](checkpoints.md)）。

## 阅读清单

| 文件 | 看什么 |
| --- | --- |
| `scaffold/JianLiuScaffold/etcd.cc` L36–L55 | 10s 租约、put 注册、KeepAlive、watch、5s resync |
| `scaffold/JianLiuScaffold/rpc.cc` | 节点管理、`getNode`/`getNodeAddr`、轮询 |
| `server/svc_user/source/svc_server.cc` | Builder：依赖装配 + 注册 |
| `server/svc_gateway/source/svc_server.cc` | 网关启动、watch 服务节点 |
| `docs/notes/03-brpc-etcd.md` | 原理 + 本项目实现 + 面试问答 |

## 时间安排

| 次 | 内容 |
| --- | --- |
| 第 1 次（读） | 45min 读 etcd.cc/rpc.cc；30min 画时序；45min etcd 查询 |
| 第 2 次（改） | 30min 设计 TTL 改动；60min 编译/重启/观察；30min 记录 |
| 第 3 次（讲） | 30min 自测；45min 白板 + 录音（CP1）；45min 补漏 |

## 动手任务

### 任务 1：观察 etcd 里的节点（必做）

```bash
curl -s -X POST http://dev-etcd:2379/v3/kv/range \
  -d '{"key":"Lw==","range_end":"MA=="}' | python3 -c '
import sys,json,base64
d=json.load(sys.stdin)
for k in d.get("kvs",[]):
    print(base64.b64decode(k["key"]).decode(), "=", base64.b64decode(k["value"]).decode())
'
```

**验收**：看到 `/file/...`、`/user/...`、`/video/...` 三个 key 和对应地址；能解释 key 里那串随机 id 是什么（实例唯一标识）。

### 任务 2：kill 一个服务，测下线延迟（必做）

```bash
kill $(cat logs/video.pid)
start=$(date +%s)
while curl -s -X POST http://dev-etcd:2379/v3/kv/range \
  -d '{"key":"L3ZpZGVvLw==","range_end":"L3ZpZGVvMA=="}' | grep -q '"kvs"'; do sleep 1; done
echo "video 节点消失耗时: $(( $(date +%s) - start ))s"
./scripts/run-dev.sh start video
```

> key 前缀 `/video/` 的 base64 是 `L3ZpZGVvLw==`；range_end 用 `L3ZpZGVvMA==`。

**验收**：记录实际耗时（应在 10s 左右，受 KeepAlive 与租约影响）；重启后节点回来。

### 任务 3：把 lease 从 10s 改成 5s（必做）

- `etcd.cc` L37：`client.leasegrant(10)` → `leasegrant(5)`；
- `etcd.cc` L55：`KeepAlive(..., 10, lease_id)` → `5`；
- `./scripts/build.sh && ./scripts/run-dev.sh start video`；
- 重复任务 2，记录新的下线耗时。

**验收**：diff 可编译；两次测量有对比表；能解释“为什么不能设成 1s”。

### 任务 4：画“上线/下线”时序并讲 5 分钟（CP1）

在纸上/白板上画：

```text
服务启动 → 建 lease(10s) → put /svc/instance=addr → KeepAlive 续约
网关启动 → watch(前缀) → list 全量 → 收到事件更新本地节点表
服务挂掉 → KeepAlive 停止 → lease 到期 → key 删除 → watch 事件 → 网关移除节点
每 5s：网关全量 resync，兜底漏掉的事件
```

**验收（CP1 三件套）**：
1. 跑：start/status/stop + 3 个 curl + etcd 查询全成功；
2. 改：lease TTL 的 diff 可编译、可运行；
3. 讲：5 分钟白板覆盖“注册、发现、失效”三点。

## 常见坑

1. `kill -9` 后租约不会立即释放，要等过期；这正是不用“立刻删除”的原因。
2. watch 与全量 list 有竞争：先 watch 再 list 会漏/重，代码里用实例键去重。
3. 同一个服务起两个实例，节点表里会有两个地址；网关轮询选择。
4. 改 TTL 后忘记重启服务，观察到的还是旧行为。

## 面试题（本周 5 个）

1. etcd 的 lease 和 watch 分别解决什么问题？
2. 为什么不用 DNS 或 Nginx 做服务发现？
3. watch 先于 list 的好处是什么？为什么还需要周期全量同步？
4. 服务发现挂掉时，网关会怎样？如何降级？
5. 两个实例注册了同一个服务名，网关怎么选？地址如何去重？

## 复盘模板

```text
我观察到的下线延迟：____ s（10s 租约）/ ____ s（5s 租约）
watch 先于 list 的原因：……
5s resync 兜底的是什么场景：……
我还讲不清的：……
```
