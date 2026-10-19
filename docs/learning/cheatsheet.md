# 笺流 · 学习速查表

> 术语看不懂？先查 [glossary.md](glossary.md)。

> 学习过程中 90% 的命令都在这一页。先能照抄跑通，再理解每一步在做什么。

## 1. 服务与端口

| 组件 | 地址 | 说明 |
| --- | --- | --- |
| gateway | `:10000` | 唯一对外入口（HTTP + JSON） |
| file 服务 | `:10001` | 图片/视频上传、下载 |
| user 服务 | `:10002` | 会话、用户、密码 |
| video 服务 | `:10003` | 视频列表、点赞、弹幕、审核 |
| transcode | 无端口 | 只消费 MQ，产出 HLS |
| MySQL | `dev-mysql:3306` | 库 `vptest`，root/123456 |
| Redis | `dev-redis:6379` | 密码 123456 |
| etcd | `dev-etcd:2379` | 服务注册中心 |
| RabbitMQ | `dev-rabbitmq:5672` | admin/123456 |
| FastDFS tracker | `dev-tracker:22122` | 文件上传入口 |
| FastDFS storage | `dev-storage:23000` | 真正存文件（**必须启动**） |
| Elasticsearch | `dev-icsearch:9200` | 可选，不可用时搜索降级 |

## 2. 一键命令

```bash
cd /home/dev/workspace

# 构建（Debug 默认；5 个服务端可执行文件 + 安装到 build/bin）
./scripts/build.sh
BUILD_TYPE=Release ./scripts/build.sh          # Release 构建

# 启停与状态
./scripts/run-dev.sh start                     # 启动全部 5 个服务（先停旧的）
./scripts/run-dev.sh status
./scripts/run-dev.sh stop
./scripts/run-dev.sh start user                # 只启动某一个（file/user/video/gateway/transcode）

# 验证
./scripts/smoke-test.sh                        # 冒烟 4 项
ctest --test-dir build                         # 单元测试 8 项
python3 scripts/check-client-api.py            # 客户端↔服务端接口一致性 38 项
python3 scripts/check-doc-links.py             # 文档相对链接（0 broken）
python3 scripts/bench.py --concurrency 20 --total 500   # 只读接口压测
```

日志：`logs/<service>.log`；PID：`logs/<service>.pid`。

## 3. 拿一个可用的会话（后续 curl / e2e 都要用）

```bash
SID=$(curl -s -X POST http://127.0.0.1:10000/HttpService/tempLogin \
  -H 'Content-Type: application/json' -d '{"requestId":"R1"}' \
  | python3 -c 'import sys,json;print(json.load(sys.stdin)["result"]["sessionId"])')

curl -s -X POST http://127.0.0.1:10000/HttpService/passwdLogin \
  -H 'Content-Type: application/json' \
  -d "{\"requestId\":\"R2\",\"sessionId\":\"$SID\",\"username\":\"tester@example.com\",\"password\":\"TestPass123\"}"

echo "SID=$SID"
python3 scripts/e2e-test.py --session "$SID" --admin
```

> `tester@example.com / TestPass123` 是普通用户 + 管理员身份；`superadmin / 123456` 是超管。

## 4. 学习前的环境自检

先跑 `./scripts/learn-check.sh`；它会检查中间件、服务、测试与文档链接。

手工检查中间件端口（bash 的 `/dev/tcp` 不需要额外工具）：

```bash
for hp in dev-mysql:3306 dev-redis:6379 dev-etcd:2379 dev-rabbitmq:5672 dev-tracker:22122 dev-storage:23000; do
  h=${hp%:*}; p=${hp#*:}
  timeout 2 bash -c "echo > /dev/tcp/$h/$p" 2>/dev/null && echo "$hp open" || echo "$hp CLOSED"
done
```

`dev-storage:23000` 关闭时上传会报 `fdfs_recv_response fail, result: 2`：
在**宿主机**上执行 `docker start dev-storage`（或重启整套中间件容器）。

## 5. etcd 查询（没有 etcdctl 也能看）

etcd v3 HTTP 的 key 是 base64：`/` = `Lw==`，前缀 `/` 的 range_end = `MA==`。

```bash
curl -s -X POST http://dev-etcd:2379/v3/kv/range \
  -d '{"key":"Lw==","range_end":"MA=="}' | python3 -m json.tool
```

返回里能看到 `/file/xxx`、`/user/xxx`、`/video/xxx` 三个节点的地址。

## 6. 查看 Redis（容器内没有 redis-cli，用 Python 发 RESP）

```bash
python3 - <<'PY'
import socket
s = socket.create_connection(("dev-redis", 6379), 2)
def cmd(*args):
    req = ("*%d\r\n" % len(args)).encode()
    for a in args:
        req += b"$%d\r\n%s\r\n" % (len(a.encode()), a.encode())
    s.sendall(req)
    return s.recv(8192)
print(cmd("AUTH", "123456"))
print(cmd("KEYS", "cache_main_page_*"))
print(cmd("GET", "cache_main_page_0_10"))
PY
```

## 7. MySQL 与日志

- 用 Navicat / DBeaver 连 `dev-mysql:3306`，用户 `root`、密码 `123456`、库 `vptest`；
- 或在宿主机：`docker exec -it dev-mysql mysql -uroot -p123456 vptest -e "select video_id,title,status from tbl_video_meta;"`；
- 常用表：`tbl_user_meta`、`tbl_session_meta`、`tbl_video_meta`、`tbl_file_meta`、`tbl_video_like_meta`、`tbl_barrage_meta`。

## 8. 常见故障对照表

| 现象 | 原因 / 处理 |
| --- | --- |
| `Fail to listen 0.0.0.0:1000x` | 端口被残留进程占用：`./scripts/run-dev.sh stop`，必要时 `kill $(cat logs/xxx.pid)` |
| 请求返回 `SERVICE_NOT_FOUND` | 目标服务没注册/已下线：看 `run-dev.sh status`、etcd 节点、`logs/*.log` |
| `上传图片失败` + `fdfs_recv_response fail result: 2` | FastDFS **storage** 没启动（tracker 在但 storage 不在）：宿主机 `docker start dev-storage` |
| `Elasticsearch 连接失败...搜索降级` | ES 不可用，属预期；搜索返回空，其余正常 |
| 客户端一直停在启动页 | `user` 服务没起来或 tempLogin 失败；看网关日志“临时用户登录失败” |
| 上传成功但首页看不到 | 视频 `status=1`（待审核），审核通过（2）才可见 |
| 客户端连不上 | `config.ini` 不在运行目录，或网关地址/端口错 |
| 播放黑屏、进度 00:00 | 视频本身极短或转码分片异常；先 `curl` m3u8 确认 |

## 9. 客户端（Windows）

1. Qt Creator 打开 `client/JianLiuPlayer/CMakeLists.txt`，选 `Qt 6.x MinGW 64-bit`；
2. `mpv/dll/libmpv-2.dll` 必须存在（构建后会自动复制到输出目录）；
3. 运行目录放 `config.ini`：

```ini
[server]
serverIp=127.0.0.1
serverPort=10000
```

4. 上传封面需要 `<运行目录>/ffmpeg/ffmpeg.exe`（没有则用默认封面兜底）。

Linux 下可用 Xvfb 验证客户端，步骤见 `docs/07-client-integration.md`。

## 10. Git 常用

```bash
git status --short
git log --oneline -10
git diff HEAD --stat
git switch feature/vod-system
git push origin feature/vod-system
```
