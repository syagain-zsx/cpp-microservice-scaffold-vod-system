# 笺流客户端（BitPlayer）

Qt 6 桌面客户端：登录、首页、搜索、播放（libmpv）、上传、弹幕、个人中心、后台管理。

> 客户端只支持 Windows 构建（依赖 libmpv 的 DLL）。服务端在 Linux 上运行。

---

## 1. 环境

| 依赖 | 版本 / 说明 |
| --- | --- |
| Qt | 6.7.x MinGW 64-bit（课程使用 6.7.3） |
| CMake | ≥ 3.16（Qt 自带即可） |
| 编译器 | MinGW 64-bit（随 Qt 安装） |
| libmpv | `libmpv-2.dll`，**需要自行下载** |

### 准备 libmpv

1. 从 <https://sourceforge.net/projects/mpv-player-windows/files/libmpv/> 下载
   `mpv-dev-x86_64-*.7z`（或 shinchiro 的构建）；
2. 解压后把 `libmpv-2.dll` 放到：

```text
client/BitPlayer/mpv/dll/libmpv-2.dll
```

> 该 DLL 约 100 MB，已加入 `.gitignore`，不进仓库。

---

## 2. 构建

### Qt Creator（推荐）

1. `文件 → 打开文件或项目`，选择 `client/BitPlayer/CMakeLists.txt`；
2. 选择 `Desktop Qt 6.7.x MinGW 64-bit` 套件，配置后构建。

### 命令行

```powershell
cd client/BitPlayer
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/Qt/6.7.3/mingw_64"
cmake --build build -j
```

产物：`build/BitPlayer.exe`（需要把 Qt 的 `windeployqt` 跑一遍，或将 Qt bin 加入 PATH）。

---

## 3. 运行配置

客户端启动时从 **当前工作目录** 读取 `config.ini`（`model/datacenter.cpp`）：

```ini
[server]
serverIp=192.168.131.128
serverPort=10000
```

仓库里提供了模板 [`config.ini.example`](BitPlayer/config.ini.example)，复制为 `config.ini` 后按需修改。

- `serverIp` 填运行网关的机器 IP（本地联调用 `127.0.0.1`）；
- `serverPort` 默认 `10000`；
- 若用 Qt Creator 直接运行，工作目录默认是构建目录，把 `config.ini` 放到那里，或把工作目录改为 exe 所在目录。

---

## 4. 代码结构

```text
BitPlayer/
├── main.cpp                启动页 + 主窗口
├── bitplayer.*             主框架（页面切换）
├── login.*                 登录/注册（邮箱验证码）
├── startuppage.*           启动页
├── homepagewidget.*        首页（推荐列表）
├── searchlineedit.*        搜索框
├── videobox.*              视频卡片
├── playerpage.*            播放页（进度、音量、倍速、弹幕）
├── uploadvideopage.*       上传页
├── myselfwidget.*          个人中心
├── adminwidget.*           后台管理（审核/用户管理）
├── paginator.*             分页组件
├── bulletscreenitem.*      弹幕动画
├── model/                  数据层（DataCenter、UserInfo、VideoInfo…）
├── netclient/              HTTP 客户端（每个接口一个 xxxAsync）
└── mpv/                    libmpv 封装（播放器渲染）
```

请求链路：`UI → model::DataCenter → netclient::NetClient → 网关 /HttpService/xxx`。
接口对照见 [../docs/02-api.md](../docs/02-api.md)。

---

## 5. 已知问题

1. `config.ini` 依赖「当前工作目录」，从 IDE 以外启动时要保证目录正确；
2. 上传视频使用 `Content-Type: multipart/form-data`，但实际是原始字节流（服务端按 attachment 读取）；
3. 播放 m3u8 时会按行解析分片 URL，依赖服务端 `hls_base_url` 配置；
4. libmpv 版本需与头文件（`mpv/client.h`）匹配，版本过低会缺 API。
