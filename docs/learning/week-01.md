# 第 1 周 · 跑起来 + 一次请求的全链路

> 术语看不懂？先查 [glossary.md](glossary.md)（租约、watch、TTL、DLX、remux 等都有大白话解释）。

> 时长：3 次 × 2 小时（约 6–8 小时）
> 产出：3 份接口 JSON、1 张登录链路图、1 段 5 分钟讲解录音、1 条耗时日志 diff

## 本周目标（可验收）

- [ ] 不看文档，能独立把 5 个服务启动并跑通冒烟测试；
- [ ] 能说清 `tempLogin` 从客户端到 MySQL 再返回的每一步；
- [ ] 会用日志定位一次“服务找不到”的失败；
- [ ] 能解释启动页为什么必须等登录成功才关闭。

## 前置

- 会基本的 Linux 命令（cd/ls/tail/kill/curl）；
- 中间件已启动（见 [cheatsheet.md](cheatsheet.md#4-学习前的环境自检)）。

## 时间安排

| 次 | 内容 |
| --- | --- |
| 第 1 次（读） | 45min 读 README + 代码导读；30min 画链路；45min 跑冒烟 |
| 第 2 次（改） | 30min 设计“加耗时日志”；60min 编码 + 重启；30min 验收 |
| 第 3 次（讲） | 30min 自测面试题；45min 录音讲解；45min 补漏 + 周记 |

## 阅读清单

| 文件 | 看什么 |
| --- | --- |
| `README.md` | 架构总览、快速开始 |
| `docs/03-deploy.md` §3 | 本地直接运行步骤 |
| `docs/06-code-walkthrough.md` §1–2 | 先跑起来、代码地图 |
| `scripts/run-dev.sh` | 每个服务用哪些 gflags 启动 |
| `server/svc_gateway/source/svc_rpc.cc` L17 | `tempLogin`：最小 HTTP → RPC 转发 |
| 同一文件 L1349 | `allVideoList`：鉴权 + 路由 |
| 同一文件 L855 | `downloadVideo`：下载（免会话） |
| `server/proto/http.proto` | 对外接口定义 |
| `scaffold/JianLiuScaffold/rpc.cc` | 怎么拿到服务节点 |

## 动手任务

### 任务 1：启动并跑通冒烟（必做）

```bash
cd /home/dev/workspace
./scripts/run-dev.sh start
./scripts/run-dev.sh status        # 期望 5 行“运行中”
./scripts/smoke-test.sh            # 期望 通过 4 / 失败 0
```

**验收**：5 个服务都是“运行中”，冒烟 4/4；任意看一个 `logs/video.log` 前 10 行，说得出它在启动时做了哪几件事。

### 任务 2：用 curl 走通 3 个接口（必做）

```bash
# 1) 临时登录
curl -s -X POST http://127.0.0.1:10000/HttpService/tempLogin \
  -H 'Content-Type: application/json' -d '{"requestId":"W1-1"}' | tee /tmp/login.json

# 2) 取列表（把上一步的 sessionId 填进去）
curl -s -X POST http://127.0.0.1:10000/HttpService/allVideoList \
  -H 'Content-Type: application/json' \
  -d '{"requestId":"W1-2","sessionId":"<sessionId>","pageIndex":1,"pageCount":10}' | tee /tmp/list.json

# 3) 下载封面（fileId 取列表里的 photoFileId）
curl -s -o /tmp/cover.png \
  'http://127.0.0.1:10000/HttpService/downloadPhoto?requestId=W1-3&sessionId=<sessionId>&fileId=<photoFileId>'
file /tmp/cover.png
```

**验收**：3 个文件都生成；`/tmp/cover.png` 是 PNG；能在 `/tmp/list.json` 里指出 `videoFileId`（m3u8）和 `photoFileId`（封面）的区别。

### 任务 3：制造一次“服务找不到”（必做）

```bash
# 停掉 user 服务（只停它，观察网关反应）
kill $(cat logs/user.pid)
sleep 2
# 再发一次 tempLogin
curl -s -X POST http://127.0.0.1:10000/HttpService/tempLogin \
  -H 'Content-Type: application/json' -d '{"requestId":"W1-4"}'
# 看网关日志最后几行，找到失败原因
tail -5 logs/gateway.log
# 恢复
./scripts/run-dev.sh start user
```

**验收**：能指出 `SERVICE_NOT_FOUND` 出现在哪一行日志、为什么会出现；重启 user 后请求恢复。

### 任务 4（加分）：给 tempLogin 加一条耗时日志

在 `server/svc_gateway/source/svc_rpc.cc` 的 `tempLogin`（L17 起）里，用 `std::chrono` 记录入口到转发之间的耗时，用 `DBG` 打印；重新 `./scripts/build.sh` 并 `./scripts/run-dev.sh start gateway`。

**验收**：diff 能编译；重启后发一次请求，日志里能看到耗时。

## 常见坑

1. 只重启一个服务时，etcd 会短暂下线再上线，期间请求可能失败——等 2–5 秒。
2. `config.ini` 只影响客户端，服务端用命令行 gflags。
3. 网关的 `downloadVideo` **不需要 sessionId**，但 `downloadPhoto` 走鉴权（可能是游客权限）。
4. 停服务用 `kill $(cat logs/xxx.pid)`，不要 `pkill -f _server` 把全部杀了。

## 面试题（本周 5 个）

1. 为什么要有一个独立的网关？它会不会成为瓶颈？
2. 客户端发的是 HTTP，内部为什么用 brpc/RPC？
3. `brpc::ClosureGuard done_guard(done)` 是干什么的？为什么 done 只能调用一次？
4. 启动页为什么要等登录成功才关闭？如果一直登录失败会发生什么？
5. sessionId 存在哪里？网关怎么用它做鉴权？

## 复盘模板（周记）

```text
本周我跑通了：……
我画出的登录链路：client → ______ → ______ → ______ → ______
我还没讲清的点：……
下周要先补的：……
```

## 检查点预告

第 2 周末是 **CP1**：需要 start/status/stop、curl 三个接口、etcd 查询、lease 改动 diff、5 分钟白板讲解。
