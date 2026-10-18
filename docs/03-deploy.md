# 笺流（JianLiu）· 部署手册

> 覆盖两种部署方式：
> **A. 本地直接运行（本项目实测通过）**、**B. Docker Compose（仓库提供编排，未在当前容器验证）**。

---

## 1. 拓扑与端口

| 组件 | 端口 | 说明 |
| --- | --- | --- |
| gateway | 10000 | HTTP 入口（客户端只连这个） |
| file | 10001 | 文件服务 |
| user | 10002 | 用户服务 |
| video | 10003 | 视频服务 |
| transcode | 无 | 只消费 MQ |
| MySQL | 3306 | 库名 `vptest` |
| Redis | 6379 | 密码 `123456` |
| etcd | 2379 | 注册中心 |
| RabbitMQ | 5672 | 账号 `admin/123456` |
| FastDFS tracker | 22122 | |
| Elasticsearch | 9200 | 可选，缺失时搜索降级 |

---

## 2. 中间件连接参数

默认值在 [`deploy/conf/global.conf`](../deploy/conf/global.conf)（`${XXX}` 由 `envsubst` 注入），
本地联调时由 [`scripts/run-dev.sh`](../scripts/run-dev.sh) 通过 gflags 传入：

| 变量 | 默认（Docker 网络） | 说明 |
| --- | --- | --- |
| `ETCD_ADDR` | `http://dev-etcd:2379` | 注册中心 |
| `MYSQL_HOST` | `dev-mysql` | MySQL |
| `REDIS_HOST` | `dev-redis` | Redis |
| `MQ_URL` | `amqp://admin:123456@dev-rabbitmq:5672/` | RabbitMQ |
| `FDFS_TRACKER` | `dev-tracker:22122` | FastDFS |
| `ES_URL` | `http://elastic:123456@dev-icsearch:9200/` | ES（可缺省） |

---

## 3. 方式 A：本地直接运行（实测）

### 3.1 系统依赖（Ubuntu 22.04）

```bash
sudo apt install -y build-essential cmake protobuf-compiler \
  libspdlog-dev libfmt-dev libjsoncpp-dev libgflags-dev libleveldb-dev \
  libprotobuf-dev libssl-dev libcurl4-openssl-dev libmysqlclient-dev \
  libev-dev libhiredis-dev libboost-all-dev libgtest-dev \
  libfastcommon-dev libfdfsclient
# 其余源码安装：brpc、etcd-cpp-apiv3、cpr、elasticlient、amqpcpp、redis++、ODB
```

### 3.2 FFmpeg（无 root 场景）

```bash
./scripts/fetch-ffmpeg.sh            # 解包到 ~/local/ffmpeg
```

脚本原理：`apt-get download` 拉 `.deb`（不需要 root）+ `dpkg -x` 解到私有前缀，
CMake 通过 `-DJIANLIU_FFMPEG_ROOT=~/local/ffmpeg/usr` 引用；链接时把该目录写入 RPATH，
运行时不再需要 `LD_LIBRARY_PATH`。

### 3.3 构建

```bash
./scripts/build.sh
# 等价于：
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
      -DJIANLIU_FFMPEG_ROOT=$HOME/local/ffmpeg/usr
cmake --build build -j$(nproc)
cmake --install build      # 产物到 build/bin
```

产物：`build/bin/{gateway,user,video,file,transcode}_server`。

### 3.4 初始化数据库

```bash
mysql -h <mysql_host> -uroot -p < deploy/sql/data.sql
# 或使用构建时生成的 schema：build/server/generated/data.sql
```

包含 13 张表 + 角色/权限字典数据。

### 3.5 启动 / 停止

```bash
./scripts/run-dev.sh start     # 全部（会先停旧的）
./scripts/run-dev.sh start user
./scripts/run-dev.sh status
./scripts/run-dev.sh stop
```

日志：`logs/<service>.log`；PID：`logs/<service>.pid`。

### 3.6 验证

```bash
./scripts/smoke-test.sh        # 网关/会话/列表 4 项
ctest --test-dir build         # 8 个单元测试
python3 scripts/e2e-test.py --session <sid> --admin   # 端到端 18 项
python3 scripts/check-client-api.py                   # 客户端接口一致性
./build/bin/file_client --session_id=<sid> --file=tools/file-client/jj.jpg --mime=image/jpeg
curl -s http://127.0.0.1:10000/status | head
```

etcd 注册检查（HTTP API，无需 etcdctl）：

```bash
curl -s -X POST http://<etcd>:2379/v3/kv/range \
  -d '{"key":"Lw==","range_end":"MA=="}' | python3 -m json.tool
```

### 3.7 客户端连接

客户端在 Windows 上运行，网关在远端时可用 SSH 端口转发：

```bash
ssh -L 10000:127.0.0.1:10000 dev@<host> -p <port>
# 然后 client/JianLiuPlayer/config.ini：
# [server]
# serverIp=127.0.0.1
# serverPort=10000
```

---

## 4. 方式 B：Docker Compose（未在当前环境验证）

```bash
./scripts/build.sh
./scripts/package.sh        # 收集 .so 到 deploy/depend/
docker compose -f deploy/docker-compose.yml up -d --build
```

- 使用外部网络 `dev-environment_dev-network`（中间件所在网络）；
- 每个容器通过 `CONF_FILE` / `EXEC_FILE` / `LISTEN_PORT` 等环境变量启动，
  `entrypoint.sh` 用 `envsubst` 渲染配置后执行 `--flagfile`；
- 镜像基于 `ubuntu:22.04`，运行库放在 `/usr/local/lib/jianliu`（ldconfig 注册），
  避免覆盖系统库。

> 注意：当前开发容器内没有 Docker，Compose 文件按课程原版整理，未做端到端验证。

---

## 5. 常见问题

| 现象 | 原因 / 处理 |
| --- | --- |
| `libavformat.so.58: cannot open shared object file` | 二进制没有 FFmpeg 的 RPATH，或用了旧产物；重新 `cmake --install build`，或 `export LD_LIBRARY_PATH=~/local/ffmpeg/usr/lib/x86_64-linux-gnu` |
| `libgme.so.0: cannot open...` | 同上；新版已用 `DT_RPATH`（可被 libavformat 的间接依赖继承） |
| video 服务启动日志 `Elasticsearch 连接失败...搜索功能降级` | ES 未启动，属预期；搜索接口返回空，其余功能正常 |
| `Fail to listen 0.0.0.0:1000x` | 端口被占用（常见于残留进程）：`./scripts/run-dev.sh stop` 或 `pkill -f _server` |
| 命令行传了服务不认识的参数 | gflags 对 **命令行** 未知参数会报错；`--flagfile` 中的未知项会被忽略（Docker 共用一份配置就依赖这个行为） |
| 客户端一直连不上 | 确认 `config.ini` 的 IP/端口、网关是否监听 `0.0.0.0`、防火墙/端口转发 |
| 上传成功但列表看不到 | 视频状态为「待审核(1)」，需管理员 `checkVideo` 或 `saleVideo` 上架 |

---

## 6. 配置项（global.conf）速查

| 配置 | 含义 |
| --- | --- |
| `-listen_port` | 服务监听端口（gateway 10000 / file 10001 / user 10002 / video 10003） |
| `-registry_center_addr` / `-service_name` / `-service_addr` | etcd 注册信息 |
| `-mysql_*` / `-redis_*` | 数据库与缓存连接 |
| `-mq_url` | RabbitMQ 连接串 |
| `-*_queue_info` | 四类交换机/队列声明（JSON） |
| `-fdfs_tracker_host` | FastDFS tracker 地址 |
| `-hls_time` / `-hls_play_type` / `-hls_base_url` | HLS 切片参数 |
| `-temp_dir` | 转码临时目录 |
