# brpc + etcd：RPC 与服务发现（本项目实现）

> 代码：`scaffold/JianLiuScaffold/rpc.cc`、`etcd.cc`、`server/svc_gateway/source/svc_rpc.cc`（异步转发）

---

## 1. 原理速览

### 1.1 为什么需要服务发现

服务实例的地址是动态的（容器 IP 变化、扩缩容），调用方不能写死地址。
注册中心保存「服务名 → 实例地址列表」，调用方订阅变化，本地维护可用节点。

### 1.2 etcd 的两个关键能力

- **租约（Lease）**：key 绑定 TTL，客户端定时续租；进程崩溃后租约到期，key 自动删除（服务自动下线）；
- **Watch**：监听前缀变化，PUT 事件 = 上线，DELETE = 下线，服务端推送、无需轮询。

### 1.3 brpc 信道

`brpc::Channel` 是一个到服务端的连接池 + 协议栈；一个 Channel 对应一个地址。
多节点时调用方自己维护 `vector<Channel>` 做负载均衡（本项目轮询）。
brpc 也支持命名服务（`bns://`、`consul://`）做客户端 LB，本项目手写更直观。

---

## 2. 本项目怎么做的

### 2.1 注册（SvcProvider）

```text
1. 实例 ID = Random::code()（20 字符，前 16 位随机 + 4 位自增序号）
2. key = /<service_name>/<instance_id>，value = <host>:<port>
3. leasegrant(10) 创建 10 秒租约，put(key, value, lease)
4. etcd::KeepAlive 后台自动续租；续租失败回调里重新 registry()
```

服务启动时调用，实例对象由 `*Server` 持有（生命周期内保活）。

### 2.2 发现（SvcWatcher）

```text
1. ls("/") 拉全量已有节点（服务启动时可能已有其他服务在线）
2. Watcher("/", recursive=true) 持续监听
3. PUT    → parse_key("/user/xxx") → "user" → online 回调 → SvcChannels::addNode
4. DELETE → 用 prev_kv 取服务名、地址 → offline 回调 → delNode
```

网关启动时对 `user`/`file`/`video` 三个服务 `setWatch`，各自一个 `Channels` 集合。

### 2.3 调用（Channels）

```cpp
ChannelPtr Channels::select() {
    lock;
    if (空) return nullptr;
    idx = (_idx++) % size;          // 轮询
    return _channels[idx].second;
}
```

**节点表按 etcd 实例键跟踪**：`_instances[服务名][/svc/instance_id] = 地址`。
重启后新旧实例可能共用同一地址，旧 key 到期删除时，只有「该地址没有其它实例」才摘除信道，
避免把新实例的通道误删（这是本项目踩过的一个真实 bug，见 04-interview 排错一节）。

brpc 参数：`protocol = baidu_std`、`timeout_ms = 30000`。
大文件上传不走 RPC，改用 `PROTOCOL_HTTP` 直接 POST 到 file 节点（见 `svc_gateway` 的 uploadVideo）。

### 2.4 异步回调（ClosureFactory）

brpc 的异步接口需要一个 `google::protobuf::Closure*`。
`ClosureFactory::create(cb)` 把一个 `std::function` 包装成 Closure，回调触发时执行 `cb()`：

```cpp
auto closure = JianLiuRpc::ClosureFactory::create([=](){
    brpc::ClosureGuard done_guard(done);   // 保证 HTTP 响应最终返回
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    ...
});
stub.uploadPhoto(cntl, req, rsp, closure);
```

网关所有转发都是这个模式：**不阻塞 bthread，RPC 返回后回调里写 HTTP 响应**。

### 2.5 一次请求的完整链路

```text
HTTP POST /HttpService/newVideo
 → gateway HttpServiceImpl::newVideo
   → judgePermission(ssid, url)            // 查权限表 + 会话
   → _channels->getNode("video")           // etcd 维护的节点集合，轮询
   → brpc stub.newVideo()
     → video VideoServiceImpl::newVideo
       → SvcData::newVideo() → data 层写库 → MQ
   → 回调里把 protobuf 响应转 JSON 写回 HTTP
```

---

## 3. 面试问答

**Q：租约为什么是 10 秒？**
最初用 3 秒：下线更快，但高负载下续租压力大、频繁过期，导致服务在注册中心反复上下线（网关偶发 `SERVICE_NOT_FOUND`）。后来改为 10 秒，并配合「watch 先于全量 + 按实例键跟踪 + 5s 周期 resync」兜底。生产常用 10–30s，配合健康检查（K8s readiness）避免流量打到未就绪实例。

**Q：续租失败会怎样？**
`KeepAlive` 的回调会重新 `registry()`；如果 etcd 整体不可用，已注册 key 会在 TTL 内消失，
调用方 watch 到 DELETE 后摘除节点。恢复后重新注册。

**Q：网关怎么处理「没有可用节点」？**
`getNode` 返回空 → 抛 `SERVICE_NOT_FOUND` → 客户端收到错误码 28。
生产中会加本地缓存 + 重试 + 熔断。

**Q：轮询负载均衡有什么问题？**
不看节点负载/延迟，长尾请求会拖慢整体；可换加权轮询、最少连接、一致性哈希（同一视频命中同一节点缓存）。

**Q：RPC 超时怎么设置？**
本项目 30s（大文件上传 300s）。要分层设置：网关→子服务短超时（如 1s），避免级联阻塞；
需要幂等才能安全重试。

**Q：为什么不用服务端负载均衡（如 Nginx）？**
客户端直连 + etcd 少一跳、延迟低；代价是每个调用方要自己实现发现与 LB（由 scaffold 封装）。

**Q：如何保证调用不丢？**
brpc 支持重试（`ChannelOptions.max_retry`），但要求接口幂等；本项目暂未开启。

---

## 4. 动手实验

1. 停掉 user 服务，`curl sessionLogin` → 立即观察网关日志「user 服务下线节点」「移除节点」；
2. 重启 user，观察「上线节点」「添加节点」，请求恢复；
3. 给 user 起第二个实例（改端口 10012 + service_addr），观察 etcd 出现两个 key、网关轮询分发
   （可看两边日志交替出现请求）；
4. 把 `Channels::select` 改成随机，压测观察分布。
