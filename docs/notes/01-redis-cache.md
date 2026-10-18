# Redis 缓存设计（本项目实现）

> 代码：`server/data/*.cc`（缓存读写）、`server/svc_video/source/svc_sync.cc`（延迟删除/回写）、
> `scaffold/JianLiuScaffold/redis.cc`（连接池）

---

## 1. 原理速览

### 1.1 缓存模式

| 模式 | 读 | 写 | 特点 |
| --- | --- | --- | --- |
| Cache-Aside（旁路缓存） | 先查缓存，未命中查 DB 并回填 | 更新 DB，再删缓存 | 最常用，本项目用这个 |
| Read/Write-Through | 缓存层代理读写 | 同步写缓存+DB | 一致性好，实现复杂 |
| Write-Behind（回写） | 同上 | 只写缓存，异步刷 DB | 性能最好，可能丢数据；本项目计数用它 |

### 1.2 三个经典问题

- **穿透**：查不存在的数据 → 缓存空值（短 TTL）或布隆过滤器；
- **击穿**：热点 key 失效瞬间大量请求打 DB → 互斥重建 / 逻辑过期；
- **雪崩**：大量 key 同时失效 → TTL 加随机、Redis 高可用、限流。

### 1.3 为什么「先更库、后删缓存」

如果先删缓存再更库，并发读可能在「删缓存」和「更新库」之间把旧值重新加载进缓存。
先更库后删缓存，最坏情况是短暂读到旧值，但最终一致；延迟双删（删→更库→延迟再删）进一步降低脏读窗口。

---

## 2. 本项目怎么做的

### 2.1 缓存键

| 前缀 | 内容 | 代码 |
| --- | --- | --- |
| `vp_session_<sid>` | 会话（含 user_id） | `data/session.cc` |
| `cahce_video_<vid>` | 视频详情（后缀字段 JSON） | `data/video.cc` |
| `video_count_<vid>` | 播放数/点赞数 | `data/video_count.cc` |
| `user_count_<uid>` | 用户统计 | `data/user_count.cc` |
| `permission_role_<url>` | URL→角色 | `data/permission_role.cc` |
| `user_identify_role_<uid>` | 用户身份角色 | `data/user_identify_role.cc` |
| `cache_main_page_<page>_<count>` | 首页列表快照（protobuf，TTL=3s） | `svc_video/source/svc_data.cc` |

> 注意 `cahce_video_` 是历史拼写错误（cache 写成 cahce），重命名需要迁移缓存，暂未改。

### 2.2 读路径（以查视频为例）

```text
VideoData::get(vid)
  ├─ getVideoFromCache(vid)          Redis HGETALL cahce_video_<vid>
  │    └─ 命中：反序列化返回
  └─ 未命中：getVideoFromDB(vid) → insertVideoToCache(vid) → 返回
```

### 2.3 写路径：更新 + 延迟删除

`VideoData::update()` 更新 MySQL 后，调用 `vp_sync::CacheSync::sync(key)`：

```text
CacheDelete::sync(key)                 // svc_video/source/svc_sync.cc
  1. 立即 Redis DEL key               // 保证当前读不到旧值
  2. 发布 DeleteCacheMsg 到 delete_cache_exchange
     （delayed 类型，TTL=3s，走 DLX 到 delete_cache_queue）
  3. 3 秒后消费者再 DEL 一次 key       // 兜住「删缓存后、事务提交前」被回填的旧值
```

这就是课程里说的「延迟双删」。

### 2.4 计数：先写缓存，延迟回写 DB

播放/点赞这类高频写：

```text
setPlay → VideoCountCache::incrPlayCount(vid)     // Redis INCR，不碰 MySQL
        → CacheToDB::sync(vid)                    // 发布到 cache_sync_to_db_exchange（TTL=30s）
30s 后消费者 CacheToDB::callback：
  1. WATCH video_count_<vid>                     // 乐观锁，期间被改则重试
  2. MULTI
  3. 读缓存计数 → 更新 MySQL（play_count）
  4. DEL 缓存计数
  5. 再发一条延迟删除消息（二次清理）
  6. EXEC（WatchError 则回到步骤 1 重试）
```

关键点：**用 Redis 事务的 WATCH/MULTI 解决并发回写覆盖问题**，失败自动重试。

### 2.5 首页列表快照缓存（性能优化）

`SvcData::getMainPageList` 把组装好的 `VideoInfo` 列表序列化（protobuf）后写入
`cache_main_page_<page>_<count>`，TTL 3 秒；命中时直接返回，省掉下游的 id 列表查询、
逐条详情回源和用户信息查询。实测 2 核 Debug 下 QPS 290 → 804（2.8x），代价是上下架最多延迟 3 秒可见。

### 2.6 连接池

`JianLiuRedis::RedisFactory::create()` 用 `sw::redis::ConnectionPoolOptions{size}`，
所有 DAO 共享一个 `shared_ptr<sw::redis::Redis>`，线程安全。

---

## 3. 面试问答

**Q：为什么用延迟双删，3 秒够吗？**
延迟是为了覆盖「删缓存 → 事务提交」窗口内的并发回填。3 秒是经验值，长事务场景需要更大或改用 binlog 订阅（Canal）。
严格一致要靠「更新 DB + 删缓存」串行化或分布式锁，但成本高。

**Q：计数为什么不用 MySQL 直接更新？**
播放数/点赞数是典型高频写，直接 UPDATE 会造成行锁竞争和大量随机 IO；Redis INCR 是内存原子操作，
延迟合并回写把 N 次写压成 1 次。

**Q：回写时缓存又被改了怎么办？**
WATCH 监控 key，MULTI/EXEC 期间 key 变化会抛 `WatchError`，代码里 `while(true)` 重试，
保证「读到的值」和「写库的值」是同一时刻的。

**Q：Redis 挂了会怎样？**
读会穿透到 MySQL，写计数会失败返回错误码；生产需要主从/集群 + 降级开关。

**Q：如何避免大 key？**
视频详情用 HASH 存字段，不要一个大 JSON；列表页只缓存 id 列表，详情按需取。

---

## 4. 动手实验

1. 手动改 Redis：`DEL cahce_video_<vid>`，再调 `allVideoList`，观察日志里回源 DB；
2. 把 `run-dev.sh` 里 `cache_sync_queue_info` 的 `delayed_ttl` 从 30 改成 5，`setPlay` 后 5 秒查 DB 看播放量；
3. 用 `KEYS *` 观察缓存键的变化（生产禁用 KEYS，改用 SCAN）；
4. 制造 WatchError：回写期间高频 `setPlay`，观察日志重试。
