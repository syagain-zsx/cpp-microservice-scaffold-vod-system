# 第 3 周 · Redis 缓存 + MySQL/ODB

> 术语看不懂？先查 [glossary.md](glossary.md)（cache-aside、TTL、ODB、事务等都有大白话解释）。

> 时长：3 次 × 2 小时（约 6–8 小时）
> 产出：压测对比表、缓存命中/未命中日志 diff、ODB 计数查询 diff、1 页结论

## 本周目标（可验收）

- [ ] 能画一次“列表请求”的缓存读/写流程（cache-aside）；
- [ ] 能用压测数据说明缓存带来的收益；
- [ ] 能解释延迟双删、计数只写 Redis 再回写的取舍；
- [ ] 能写一个 ODB 查询，并验证结果与接口一致。

## 阅读清单

| 文件 | 看什么 |
| --- | --- |
| `server/svc_video/source/svc_data.cc` L468 `getMainPageList` | 3s 缓存、回源、写缓存（L539 `setex`） |
| `server/data/video_idlist.cc` | 主页/用户页 ID 列表缓存（zset，1000 条） |
| `server/data/video.cc` | 视频 DAO：ODB 查询 + 缓存删除 |
| `server/data/session.cc` | 会话查询 |
| `server/data/data.h` | `VideoStatus`、`Video` 实体 |
| `scaffold/JianLiuScaffold/redis.cc` | 连接池与常用命令封装 |
| `scaffold/JianLiuScaffold/odb.cc` | 连接池、事务工厂 |
| `docs/notes/01-redis-cache.md`、`04-odb.md` | 原理 + 本项目 + 面试 |
| `docs/05-performance.md` | 已有压测结论 |

## 时间安排

| 次 | 内容 |
| --- | --- |
| 第 1 次（读） | 45min 读缓存与 DAO；30min 画 cache-aside；45min 跑基线压测 |
| 第 2 次（改） | 30min 设计 TTL/日志；60min 改代码 + 压测；30min 记录 |
| 第 3 次（讲） | 30min 自测；45min 讲数据与取舍；45min 补漏 |

## 动手任务

### 任务 1：缓存收益对比（必做）

```bash
# 基线（默认 TTL 3s）
python3 scripts/bench.py --concurrency 20 --total 500

# 把 svc_data.cc L539 的 setex(cache_key, 3, bytes) 改成 30
# 重新构建并重启 video 服务
./scripts/build.sh && ./scripts/run-dev.sh start video
python3 scripts/bench.py --concurrency 20 --total 500
```

**验收**：填一张表（QPS、p50、p99、错误数），写 3 行结论：为什么 TTL 变长后 QPS 变化不大/变大？瓶颈在哪？（可结合 `docs/05-performance.md`：读路径主要在 Redis/网络往返。）

### 任务 2：加缓存命中/未命中日志（必做）

在 `getMainPageList` 的缓存读取分支加两条 `DBG`：

```cpp
// 命中
DBG("main page cache hit: {}", cache_key);
// 未命中（准备回源）
DBG("main page cache miss: {}", cache_key);
```

重新构建、重启 video，压测 200 次，然后：

```bash
grep -c 'cache hit'  logs/video.log
grep -c 'cache miss' logs/video.log
```

**验收**：日志里能数出 hit/miss；能解释“第一次请求 miss、之后 hit（3s 内）”的现象。

### 任务 3：写一个 ODB 查询并验证（必做）

给 `SvcData` 加一个方法（`svc_data.h` 声明 + `svc_data.cc` 实现）：

```cpp
size_t countUserVideos(const std::string &uid, vp_data::VideoStatus status, odb::transaction &mtx) {
    typedef odb::query<vp_data::Video> Query;
    auto result = mtx.database().query<vp_data::Video>(
        Query::uploader_uid == uid && Query::status == (int)status);
    size_t n = 0;
    for (auto it = result.begin(); it != result.end(); ++it) ++n;
    return n;
}
```

在 `getUserPageList` 里调用并 `DBG` 打印；然后请求：

```bash
curl -s -X POST http://127.0.0.1:10000/HttpService/userVideoList \
  -H 'Content-Type: application/json' \
  -d '{"requestId":"W3-1","sessionId":"<sid>","pageIndex":1,"pageCount":10,"videoStatus":2}'
```

**验收**：日志里的计数 == 响应里的 `totalCount`；能解释 `VideoStatus` 的 1/2/3/4/5 含义。

## 常见坑

1. 改完代码只 `build` 没重启服务，行为不变；
2. 压测前没重启 video 服务，缓存里还有旧数据；
3. `getMainPageList` 的缓存是 **protobuf 序列化** 后写入的，不是 JSON；
4. ODB 查询的 `status` 是 tinyint，比较时注意类型转换；
5. 计数类数据只写 Redis、30s 后由延迟队列回写 MySQL，直接查库会“看起来少”。

## 面试题（本周 5 个）

1. cache-aside 的读写流程是什么？为什么是“先更库、后删缓存”？
2. 什么是延迟双删？为什么本项目用延迟队列做计数回写？
3. 缓存穿透/击穿/雪崩分别怎么防？本项目做了哪些、还缺哪些？
4. ODB 相比手写 SQL 的优缺点？事务和连接池怎么用？
5. 缓存和数据库不一致的窗口有多大？如何监控？

## 复盘模板

```text
基线 QPS/p99：______ / ______
TTL 30s 后：______ / ______
结论：……
我解释不清的一个点：……
```
