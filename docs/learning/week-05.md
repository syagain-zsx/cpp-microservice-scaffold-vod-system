# 第 5 周 · 客户端与接口契约

> 时长：3 次 × 2 小时（约 6–8 小时）
> 产出：接口一致性检查记录、大文件上传改造 diff、0 时长问题复现/修复记录

## 本周目标（可验收）

- [ ] 能说出客户端每个页面对应哪些网关接口；
- [ ] 能独立在 Windows（或 Linux/Xvfb）构建并运行客户端；
- [ ] 能改一个真实的客户端问题（大文件上传 / 播放保护）并验证；
- [ ] 能解释“接口一致性检查”在防什么。

## 阅读清单

| 文件 | 看什么 |
| --- | --- |
| `client/JianLiuPlayer/netclient/netclient.cpp` | 每个 `xxxAsync` 对应一个接口；`sendHttpRequest` |
| 同一文件 L376 | `uploadVideoAsync`：整文件读入内存（L394 `loadFileToByteArray`） |
| `client/JianLiuPlayer/model/datacenter.cpp` | sessionId、用户信息、列表状态 |
| `client/JianLiuPlayer/playerpage.cpp` L210–L240 | 播放进度、时长、进度条 |
| `client/JianLiuPlayer/uploadvideopage.cpp` | 上传流程、封面截帧、提交 |
| `client/JianLiuPlayer/mpv/mpvplayer.cpp` L190–L230 | libmpv 截帧、事件 |
| `docs/07-client-integration.md` | 页面↔接口映射、Windows 构建、联调顺序 |
| `scripts/check-client-api.py` | 38 项接口/字段一致性检查 |

## 时间安排

| 次 | 内容 |
| --- | --- |
| 第 1 次（读） | 45min 读 netclient/playerpage；30min 画页面↔接口表；45min 跑 check-client-api |
| 第 2 次（改） | 30min 设计上传上限；60min 改 + 构建 + 实测；30min 记录 |
| 第 3 次（讲） | 30min 自测；45min 讲客户端链路；45min 补漏 |

## 动手任务

### 任务 1：接口一致性检查（必做）

```bash
python3 scripts/check-client-api.py
# 期望：服务端 rpc 38 / 客户端调用 38，[PASS] 两项
```

然后**故意破坏**一次：在 `netclient.cpp` 里注释掉一个 `xxxAsync` 对某个接口的调用（或改错路径），重跑脚本看它如何报错，再恢复。

**验收**：能解释脚本检查的两件事（接口存在性、JSON 字段能否在 proto 中找到）；恢复后仍 38/38。

### 任务 2：大文件上传保护（必做）

现状：`uploadVideoAsync` 把整个文件读进 `QByteArray` 再 POST，大文件会内存暴涨/超时，且失败时**没有明显提示**。

改法（二选一，先做简单的）：

- **方案 A（上限 + 提示）**：读取文件大小，超过 50MB 时用 `Toast::showMessage("视频过大，请压缩后再上传")`（或 `LOG` + 界面提示）并直接返回，不发请求；
- **方案 B（加分）**：改用 `QHttpMultiPart` 流式上传，或分片上传 + 断点续传（工程量大，可作为第 6 周的延伸）。

**验收**：选一个 >50MB 的视频，界面有明确提示、不崩溃、不静默失败；正常小视频上传成功（可在 Linux/Xvfb 或 Windows 实测）。

### 任务 3：复现并理解 0 时长崩溃（必做）

项目里有 0.85 秒的短视频（或自己造一个极短视频上传）。

1. 先看修复提交：`git show 92142e5 -- client/JianLiuPlayer/playerpage.cpp client/JianLiuPlayer/playslider.cpp`；
2. 本地把两处保护临时改回旧逻辑（或用 `git checkout 92142e5^ -- <files>` 后恢复），构建，点开该视频；
3. 观察崩溃/卡死；再恢复修复代码。

**验收**：能讲清 `duration=0 → playTime/duration = inf → 赋给 int 未定义 → setGeometry 非法宽度 → 崩溃` 这条链路；能解释为什么“播放结束自动重播”会让极短视频更糟。

## 常见坑

1. `config.ini` 必须在运行目录（Qt Creator 默认是构建目录）；
2. Qt 的 `QNetworkAccessManager` 是异步的，别在回调外读 `QNetworkReply`；
3. 跨线程信号槽默认 `AutoConnection`，注意对象所在线程；
4. libmpv 的事件在 mpv 线程，必须转到主线程再操作 UI；
5. 残留共享内存会让客户端提示“已经在运行”。

## Windows 构建（摘要）

1. Qt Creator 打开 `client/JianLiuPlayer/CMakeLists.txt`，选 Qt 6.x MinGW；
2. `mpv/dll/libmpv-2.dll` 必须存在（构建后自动复制到输出目录）；
3. 运行目录放 `config.ini`，上传封面需要 `ffmpeg/ffmpeg.exe`；
4. 详见 `docs/07-client-integration.md`。

Linux 下没有 Qt 环境时，可用 Xvfb + 已编译的客户端验证（见 `docs/07` 与 `docs/06`）。

## 面试题（本周 5 个）

1. 如何保证客户端和服务端接口一致？你做了什么工程化手段？
2. 大文件上传有哪些方案？各自的优缺点（内存、超时、断点续传）？
3. Qt 信号槽的连接方式有哪几种？跨线程要注意什么？
4. libmpv 在客户端里怎么用？事件循环为什么不能阻塞 UI 线程？
5. 遇到“服务端正常但客户端黑屏/卡启动页”，你怎么排查？

## 复盘模板

```text
页面 ↔ 接口表：……
大文件改造方案与验收：……
0 时长崩溃的因果链：……
还没讲清的：……
```
