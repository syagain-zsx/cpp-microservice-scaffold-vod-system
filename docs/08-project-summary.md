# 笺流（JianLiu）· 项目总览（一页版）

> 用于作品集/简历/面试时的快速介绍。详细内容见 [ROADMAP.md](ROADMAP.md)、[01-architecture.md](01-architecture.md)、[04-interview.md](04-interview.md)。

---

## 1. 一句话

**一个用 C++17 写的微服务化视频点播系统**：Qt6 客户端 + 网关 + 用户/视频/文件/转码 4 个子服务，
覆盖 RPC、服务发现、消息队列、缓存、ORM、搜索引擎、分布式文件存储、视频转码等中间件实践。

## 2. 技术栈

| 层次 | 技术 |
| --- | --- |
| 语言/构建 | C++17、CMake（服务端与客户端统一） |
| RPC / HTTP | brpc + protobuf（网关对外 HTTP + 内部 RPC） |
| 服务发现 | etcd（租约 + watch + 周期全量自愈） |
| 消息队列 | RabbitMQ（DLX + TTL 延迟队列） |
| 缓存 | Redis（redis++，缓存旁路 + 计数异步回写） |
| 数据库 | MySQL 8 + ODB 2.5（代码生成 ORM） |
| 搜索 | Elasticsearch（当前环境不可用，自动降级） |
| 文件存储 | FastDFS（Tracker/Storage） |
| 转码 | FFmpeg（libavformat，MP4 → HLS） |
| 客户端 | Qt 6 + libmpv（Windows 构建，Linux 已验证） |
| 基础设施 | Docker Compose / Shell 脚本 / gtest / Xvfb（验证） |

## 3. 架构

```text
Qt 客户端 ──HTTP/JSON──▶ gateway ──brpc──▶ user / video / file
                                    │
                                    └──HTTP──▶ file（大文件）
transcode ◀──MQ── video            file ──▶ FastDFS
user/video ──▶ MySQL + Redis + ES（可选）
所有服务 ──▶ etcd（注册/发现）
```

## 4. 功能与验证方式

| 功能 | 验证方式 | 结果 |
| --- | --- | --- |
| 会话/密码登录 | `e2e-test.py`、客户端 GUI | ✅ |
| 视频列表/分类/标签 | HTTP 接口、客户端首页 | ✅ |
| 上传图片/视频 | `file_client`、客户端上传页 | ✅ |
| 视频转码 → HLS | `e2e-test.py` 等待 m3u8、客户端播放 | ✅ |
| 播放（m3u8 + ts） | 客户端播放页（libmpv） | ✅ |
| 点赞/播放计数/弹幕 | `e2e-test.py` + 客户端 UI | ✅ |
| 关注/取关 | 接口验证 | ✅ |
| 管理员审核/上下架 | 客户端「系统 → 审核管理」 | ✅ |
| 搜索 | ES 不可用时降级为空列表 | ⚠️ 降级 |
| 删除视频/文件 | 代码路径存在，MQ 异步删除 | ✅（接口级） |

## 5. 质量证据

```bash
# 单元测试（8 项：JSON/字符串/随机串/文件/M3U8/密码哈希）
ctest --test-dir build                 # 8/8 passed

# 冒烟（网关/会话/列表）
./scripts/smoke-test.sh                # 4/4 passed

# 端到端（上传→转码→HLS→审核→互动）
python3 scripts/e2e-test.py --session <sid> --admin   # 18/18 passed

# 客户端 ↔ 服务端接口一致性
python3 scripts/check-client-api.py    # 38/38

# 文档链接
python3 scripts/check-doc-links.py     # 0 broken
```

## 6. 性能

| 场景 | QPS | p99 |
| --- | --- | --- |
| 基线（Debug，2 核） | ~290 | 346 ms |
| 首页列表 3s 短缓存 | **~800**（2.8x） | **94 ms** |
| Release 构建 | ~800（无显著提升） | 96 ms |

> 说明：读路径瓶颈在 Redis/网络往返与 JSON 序列化，优化应针对瓶颈而非编译器。

## 7. 工程亮点

- **单仓库、统一构建**：`scripts/build.sh` 一条命令产出 5 个服务端可执行文件；客户端 CMake 跨平台
- **脚手架库 `JianLiuScaffold`**：11 个中间件封装（日志/JSON/RPC/etcd/MQ/Redis/ODB/ES/FastDFS/HLS/邮件）
- **服务发现可靠性**：租约 10s、watch 先于全量、实例键跟踪、5s 周期同步，解决重启丢节点问题
- **中间件降级**：ES 不可用搜索降级；MQ 异常不杀死服务，只记录并降级
- **可复现验证**：gtest + 冒烟 + 端到端 + 接口一致性 + 文档链接检查
- **文档齐全**：架构、接口、部署、代码导读、5 篇学习笔记、面试材料、性能报告

## 8. 已知限制

1. **Windows 实机未验证**（Linux + Xvfb 已完整验证，包括播放）；
2. **Docker Compose 未实机验证**（当前容器无 Docker）；
3. **Elasticsearch 不可用**，搜索为降级状态；
4. MQ 未开启 publisher confirm，极端情况下可能丢消息；
5. 历史明文密码仍兼容（新密码已加盐 SHA-256）。

## 9. 学习收获（对应笔记）

- [Redis 缓存设计](notes/01-redis-cache.md)：cache-aside、延迟双删、WATCH/MULTI 计数回写
- [RabbitMQ 延迟队列](notes/02-rabbitmq-delay.md)：DLX + TTL、消息可靠性与幂等
- [brpc + etcd](notes/03-brpc-etcd.md)：租约/watch、轮询 LB、异步 Closure
- [ODB](notes/04-odb.md)：实体/视图/事务/连接池/代码生成
- [HLS + FFmpeg](notes/05-hls-ffmpeg.md)：remux、m3u8/ts、播放链路

## 10. 简历描述（可直接用）

> **笺流（JianLiu）· C++ 微服务视频点播系统**（个人项目）
> - 将课程项目重构为单仓库微服务架构：网关 + 用户/视频/文件/转码 4 服务 + Qt6 客户端；
> - 封装 brpc/etcd/RabbitMQ/Redis/ODB/FastDFS/FFmpeg 等 11 类中间件客户端为 `JianLiuScaffold`；
> - 实现「上传 → MQ 异步转码 → HLS 切片 → 播放」链路，以及基于延迟队列的缓存计数异步回写；
> - 设计网关 URL→角色鉴权与视频审核状态机；解决服务发现抖动、MQ 异常退出等稳定性问题；
> - 建立统一构建、8 项单测、18 项端到端与接口一致性检查；首页列表缓存优化使 QPS 提升 2.8 倍。
