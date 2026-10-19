# 第 6 周 · 测试 / 性能 / 部署 + 讲出来

> 术语看不懂？先查 [glossary.md](glossary.md)（QPS、p99、幂等、状态机等都有大白话解释）。

> 时长：3 次 × 2 小时（约 6–8 小时）
> 产出：全套测试输出、1 页优化报告、15 分钟模拟面试录音、简历一页纸

## 本周目标（可验收）

- [ ] 能独立跑完整套验证（ctest / smoke / e2e / check-api）；
- [ ] 能做一次有数据支撑的优化对比；
- [ ] 能 15 分钟讲项目并接住 10 个追问；
- [ ] 完成 **CP3**（见 [checkpoints.md](checkpoints.md)）。

## 阅读清单

| 文件 | 看什么 |
| --- | --- |
| `server/tests/*.cc` | 8 个单测覆盖点（JSON、字符串、文件、M3U8、密码哈希） |
| `scripts/e2e-test.py` | 18 项端到端覆盖范围 |
| `scripts/bench.py` | 压测参数与指标 |
| `deploy/Dockerfile`、`deploy/docker-compose.yml`、`deploy/conf/global.conf` | 部署方式（未实机验证，面试可讲取舍） |
| `docs/04-interview.md` | 12 组面试问答 |
| `docs/05-performance.md` | 已有性能结论 |
| `docs/08-project-summary.md` | 一页纸总览 + 简历描述 |

## 时间安排

| 次 | 内容 |
| --- | --- |
| 第 1 次（跑） | 全新构建 + 全套测试，记录输出 |
| 第 2 次（改） | 一次优化对比（Release/TTL/连接池），写 1 页报告 |
| 第 3 次（讲） | 模拟面试 10 题 + 录音；整理简历 |

## 动手任务

### 任务 1：全新构建 + 全套验证（必做）

```bash
rm -rf build
./scripts/build.sh
ctest --test-dir build
./scripts/smoke-test.sh
# e2e 需要会话，见 cheatsheet 第 3 节
python3 scripts/e2e-test.py --session "$SID" --admin
python3 scripts/check-client-api.py
python3 scripts/check-doc-links.py
./scripts/learn-check.sh
```

**验收**：8/8、4/4、18/18、38/38、0 broken；把关键输出贴进周记。

### 任务 2：一次有数据的优化对比（必做，二选一）

- **A. Release vs Debug**：`BUILD_TYPE=Release ./scripts/build.sh`，重启服务后 `bench.py` 对比；
- **B. 连接池大小**：把 `--mysql_connection_pool_size` 从 5 改成 20，压测对比（注意并发不要超过机器能力）。

写 1 页报告：

```text
假设 → 改动 → 方法（并发/总量/环境）→ 结果表 → 结论 → 下一步
```

**验收**：报告里有数字、有结论、能解释“为什么提升/没提升”。

### 任务 3：模拟面试（必做）

1. 从 `docs/04-interview.md` 选 10 题（架构 3 + 缓存 2 + MQ 2 + 排错 2 + 优化 1）；
2. 用手机录音，15 分钟，不看稿；
3. 回听，记录“卡住/说错”的点，补进周记。

**验收（CP3）**：10 题至少 8 题答到点；能拿出一个可演示的改进（diff + 测试）。

## 常被问倒的 3 个坑（务必准备）

1. **MQ 声明超时拖死服务启动**：为什么不能 `abort`？降级后异步链路不可用，怎么监控、怎么恢复？30s×N 的启动时间怎么优化？
2. **0 时长视频崩溃**：除零 → `inf` → 赋给 `int` 未定义 → `setGeometry` 非法宽度；如何防御（判空、`std::isfinite`、夹取）。
3. **大文件上传整体进内存 + 网关 protobuf 拷贝**：内存放大、超时；分块/流式/直传（预签名 URL）方案怎么选。

附加：`downloadVideo` 不做鉴权（fileId 即凭证）的权衡；ES 降级；「我的视频只查 ONLINE」的可见性。

## 5 个深挖点（面试高频）

1. 网关：统一入口 + URL→角色鉴权 + etcd 路由；
2. 服务发现可靠性：lease/watch/resync/实例键（真实 bug 与修复）；
3. 缓存：3s 列表缓存、延迟双删、计数回写 + 290→800 QPS 数据；
4. 异步转码：DLX+TTL、HLS remux、幂等、失败处理；
5. 数据层：13 表建模、ODB 事务/连接池、视频状态机。

## 简历与讲述

- 5 分钟讲述稿：背景 → 架构 → 三个难点（异步转码 / 缓存一致性 / 服务发现容错）→ 数据 → 反思；
- 简历一行版见 `docs/08-project-summary.md` 第 10 节；
- 演示顺序：登录 → 首页 → 播放 → 上传 → 审核 → 上架 → 播放。

## 复盘模板

```text
全套测试结果：……
优化报告结论：……
模拟面试卡住的题：……
6 周后我还想补的：……
```
