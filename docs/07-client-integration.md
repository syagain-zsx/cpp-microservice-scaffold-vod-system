# 客户端联调指南（M4）

> 目标：在 Windows 上把 BitPlayer 编译起来，按顺序打通「登录 → 首页 → 播放 → 上传 → 个人中心 → 后台」。
> 服务端已通过 `scripts/e2e-test.py` 验证（18/18）。
>
> **验证进展（2026-10-18）**：客户端已在 Linux（Qt 6.2.4 + libmpv 0.34）下**编译通过**，
> 并以 `QT_QPA_PLATFORM=offscreen` 启动，成功 `tempLogin`、从服务端拉取 7 个视频；
> Windows 图形界面与播放链路待实机验证。

---

## 1. 接口一致性（静态检查已通过）

```bash
python3 scripts/check-client-api.py
# 服务端 rpc: 38，客户端调用: 38
# [PASS] 客户端调用的接口全部存在
# [PASS] 客户端 JSON 字段均能在 proto 中找到
```

客户端 `netclient` 的 38 个调用与服务端 `HttpService` 的 38 个 rpc **一一对应**，
JSON 字段名也与 proto 一致（`pageIndex` 等驼峰命名由 brpc 自动映射）。

---

## 2. 页面 ↔ 接口映射

| 页面/文件 | 调用的 async 方法 | 服务端接口 |
| --- | --- | --- |
| `login.cpp` | `loginTempUserAsync` / `getAuthcodeAsync` / `loginWithEmailAsync` / `loginWithPasswordAsync` | `tempLogin` / `getCode` / `vcodeLogin` / `passwdLogin` |
| `homepagewidget.cpp` | `getAllVideoListAsync` / `getAllVideoListInKindAsync` / `getAllVideoListInTagAsync` / `getAllVideoListSearchTextAsync` | `allVideoList` / `typeVideoList` / `tagVideoList` / `keyVideoList` |
| `playerpage.cpp` | `downloadVideoAsync` / `getVideoBarrageAsync` / `loadupBarrageAsync` / `setPlayNumberAsync` / `getIsLikeVdieoAsync` / `setLikeNumberAsync` | `downloadVideo` / `getBarrage` / `newBarrage` / `setPlay` / `judgeLike` / `setLike` |
| `uploadvideopage.cpp` | `uploadPhotoAsync` / `uploadVideoAsync` / `uploadVideoDescAsync` | `uploadPhoto` / `uploadVideo` / `newVideo` |
| `myselfwidget.cpp` | `getMyselfInfoAsync` / `getOtherUserInfoAsync` / `setAvatarAsync` / `downloadPhotoAsync` / `newAttentionAsync` / `delAttentionAsync` / `getUserVideoListAsync` / `deleteVideoAsync` / `logoutAsync` | `getUserInfo` / `setAvatar` / `downloadPhoto` / `newAttention` / `delAttention` / `userVideoList` / `removeVideo` / `logout` |
| `checktable.cpp` | `getStatusVideoListAsync` / `getUserVideoListAsync` / `checkVideoAsync` | `statusVideoList` / `userVideoList` / `checkVideo` |
| `roletable.cpp` | `getAdminInfoByEmailAsync` / `getAdminInfoByStatusAsync` / `newAdminAsync` / `editAdminAsync` / `delAdminAsync` / `setAdminStatusAsync` | `getAdminByEmail` / `getAdminListByStatus` / `newAdministrator` / `setAdministrator` / `delAdministrator` / `setStatus` |
| `adminwidget.cpp`（按钮） | `putawayVideoAsync` / `discardVideoAsync` | `saleVideo` / `haltVideo` |

---

## 3. 启动与配置

```ini
; config.ini —— 放在「程序当前工作目录」
[server]
serverIp=127.0.0.1
serverPort=10000
```

> 客户端用 `QDir::currentPath()` 找 `config.ini`，不是 exe 同目录。
> Qt Creator 运行时默认工作目录是构建目录，把 `config.ini` 放过去，或改运行配置的工作目录。

libmpv：`client/BitPlayer/mpv/dll/libmpv-2.dll`（约 100MB，需自行下载，见 [client/README.md](../client/README.md)）。

---

## 4. 构建（Windows）

```powershell
# 前提：Qt 6.7.x MinGW 64-bit 已安装，MinGW 的 bin 在 PATH
cd client/BitPlayer
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/Qt/6.7.3/mingw_64"
cmake --build build -j
# 或用脚本：
./build-windows.ps1 -QtPath "D:/Qt/6.7.3/mingw_64"
```

构建后建议用 `windeployqt build/BitPlayer.exe` 收集 Qt DLL，再把 `libmpv-2.dll` 放到 exe 同目录。

---

## 5. 建议的联调顺序（每步都能独立验证）

| 步骤 | 操作 | 预期 | 失败时先查 |
| --- | --- | --- | --- |
| 1 | 启动客户端 | 出现启动页 → 登录页 | `config.ini` 路径/端口 |
| 2 | 游客进入 | 首页显示视频列表 | 网关 `allVideoList` 是否 200 |
| 3 | 密码登录 | 用测试账号登录成功 | 服务端是否有该用户（可用 SQL 造数） |
| 4 | 点开视频 | 播放页出现封面、时长、点赞数 | `downloadVideo` 返回的是否 m3u8 |
| 5 | 播放 | mpv 正常播放 HLS | `hls_base_url` 是否指向网关、ts 是否可下载 |
| 6 | 发弹幕 | 弹幕出现并入库 | `newBarrage` 响应 |
| 7 | 上传视频 | 上传进度完成、提示成功 | 网关 10000 是否能直连（大文件走 HTTP） |
| 8 | 后台审核 | 视频出现在审核列表 → 通过 → 首页可见 | 账号是否有管理员身份 |

服务端可用命令快速核对：

```bash
curl -s -X POST http://127.0.0.1:10000/HttpService/allVideoList \
  -H 'Content-Type: application/json' \
  -d '{"requestId":"c1","sessionId":"<sid>","pageIndex":1,"pageCount":10}'
```

---

## 6. 已知差异与注意点

1. **分页从 1 开始**：客户端 `VideoList` 初始 `pageIndex=1`，服务端会 `pageIndex-1` 后作为偏移，传 0 会得到空列表；
2. **上传是原始字节流**：客户端 `Content-Type` 写的是 `multipart/form-data`，但实际发的是裸字节，
   服务端按 `request_attachment()` 读取，不要改成真正的 multipart；
3. **列表有 3 秒缓存**：审核通过后首页最多 3 秒才可见，客户端刷新即可；
4. **搜索依赖 ES**：当前环境 ES 未启动，`keyVideoList` 返回空列表（不是报错），属预期降级；
5. **删除视频**：服务端会删 FastDFS 文件 + 元信息，客户端需确认后才调用；
6. **`hello` / `ping`** 是课程遗留的测试接口，UI 未使用（可后续清理）。

---

## 7. 完成后要补的东西

- [ ] 客户端截图（登录/首页/播放/后台）放进 README；
- [ ] 上传进度条与断点续传（可选）；
- [ ] 弹幕渲染在真实播放时间轴上的校准；
- [ ] 客户端异常提示统一（toast 复用）。
