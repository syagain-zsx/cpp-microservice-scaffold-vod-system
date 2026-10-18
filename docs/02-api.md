# 笺流（JianLiu）· 接口文档

> 网关地址：`http://127.0.0.1:10000`（本地联调）
> 协议：HTTP POST + JSON（服务端是 brpc 的 protobuf 服务，brpc 自动把 JSON 映射为 protobuf）
> 编码：UTF-8

---

## 1. 通用约定

### 1.1 请求

- 普通接口：`POST /HttpService/<方法名>`，`Content-Type: application/json`，body 为 JSON；
- 文件上传：`POST /HttpService/uploadPhoto|uploadVideo?requestId=...&sessionId=...`，
  文件二进制放在 **请求体（attachment）**，`Content-Type` 填文件 MIME；
- 文件下载：`GET /HttpService/downloadPhoto|downloadVideo?requestId=...&sessionId=...&fileId=...`；
- 会话 ID 一般放在 JSON 的 `sessionId` 字段；文件上传/下载放在 query 参数里。

### 1.2 响应

成功：

```json
{
  "requestId": "R7005",
  "errorCode": 0,
  "errorMsg": "处理成功",
  "result": { }
}
```

失败：`errorCode != 0`，`errorMsg` 为中文错误描述；HTTP 状态码可能仍为 200（业务错误）或 500（上传类接口）。

### 1.3 错误码（`server/common/error.h`）

| 值 | 枚举 | 含义 |
| --- | --- | --- |
| 0 | SUCCESS | 处理成功 |
| 1 | INVALID_SESSION | 会话无效 |
| 2 | UPLOAD_FILE_TO_FDFS_FAILED | 文件上传失败 |
| 3 | DATABASE_OP_FAILED | 数据操作失败 |
| 5 | DOWNLOAD_FILE_FROM_FDFS_FAILED | 文件下载失败 |
| 6 | REDIS_OP_FAILED | Redis 操作失败 |
| 7 | MQ_OP_FAILED | MQ 操作失败 |
| 8 | FILE_NOT_EXIST | 文件不存在 |
| 9 | USER_NOT_EXIST | 用户不存在 |
| 10 | SERVER_INERNAL_ERROR | 服务器内部错误 |
| 11 | SESSION_INVALID | 无效会话 |
| 12 | VERIFY_CODE_ERROR | 验证码错误 |
| 16 | PERMISSION_DENIED | 权限不足 |
| 17 | USER_COUNT_NOT_EXISTS | 用户统计数据不存在 |
| 19 | USER_IDENTIFY_ROLE_NOT_EXISTS | 用户身份角色不存在 |
| 20 | USER_ALREADY_FOLLOWING | 已关注 |
| 21 | USER_NOT_FOLLOWING | 未关注 |
| 22 | MAIL_CODE_ERROR | 邮箱验证码发送失败 |
| 26 | ELASTIC_OP_ERROR | 搜索引擎操作失败（ES 不可用时会降级） |
| 27 | VIDEO_NOT_EXISTS | 视频不存在 |
| 28 | TRANSCODE_FAILED | 转码失败 |
| 29 | SERVICE_NOT_FOUND | 服务未找到 |
| 30 | RPC_REQUEST_FAILED | RPC 请求失败 |
| 31 | CLIENT_REQUEST_PARAM_ERROR | 参数错误 |
| 32 | NICKNAME_IS_EXISTS | 昵称已存在 |

### 1.4 鉴权

`tbl_operator_role_meta` 维护 `URL → 允许的角色` 映射：

| role_type | 含义 |
| --- | --- |
| 0 | 无需登录 |
| 1 | 超级管理员 |
| 2 | 普通管理员 |
| 3 | 普通登录用户 |
| 4 | 访客（有会话但未登录） |

网关收到请求后按 URL 查角色，再取会话用户的身份角色做匹配；无权限返回 `PERMISSION_DENIED`。

---

## 2. 会话（user 服务）

| 接口 | 说明 | 关键字段 |
| --- | --- | --- |
| `tempLogin` | 创建临时会话（游客） | `requestId` → `result.sessionId` |
| `sessionLogin` | 校验会话，判断是否游客 | `sessionId` → `result.isGuest` |
| `getCode` | 发送邮箱验证码 | `sessionId`, `email` → `result.codeId` |
| `vcodeLogin` | 验证码注册/登录 | `sessionId`, `codeId`, `code`, `email` |
| `passwdLogin` | 用户名密码登录 | `sessionId`, `username`, `password` |
| `logout` | 退出登录 | `sessionId` |

```bash
# 1) 建会话
curl -s -X POST http://127.0.0.1:10000/HttpService/tempLogin \
     -H 'Content-Type: application/json' -d '{"requestId":"R1"}'
# {"requestId":"R1","errorCode":0,"result":{"sessionId":"IBVEecdlyMqny3fH0001"}}

# 2) 校验（游客）
curl -s -X POST http://127.0.0.1:10000/HttpService/sessionLogin \
     -H 'Content-Type: application/json' \
     -d '{"requestId":"R2","sessionId":"IBVEecdlyMqny3fH0001"}'
# {"errorCode":0,"result":{"isGuest":true}}
```

> 本地联调可跳过邮件：直接向 `tbl_user_meta` / `tbl_user_identify_role_meta` 插入用户，
> 再把 `tbl_session_meta.user_id` 指向该用户，删掉 Redis 里的 `vp_session_<sid>` 后重新 `sessionLogin`。

> 注意：`passwdLogin` 成功会移除该用户的其它会话（单端登录），旧会话会变回访客。
> 密码以加盐 SHA-256 存储；历史明文密码仍可登录（兼容逻辑见 `JianLiuUtil::Password`）。

---

## 3. 用户（user 服务）

| 接口 | 说明 |
| --- | --- |
| `getUserInfo` | 获取用户详情（含粉丝/关注/获赞统计） |
| `setAvatar` / `setNickname` / `setPassword` | 修改资料 |
| `setStatus` | 管理员启用/禁用用户 |
| `newAttention` / `delAttention` | 关注 / 取关 |
| `newAdministrator` / `delAdministrator` / `setAdministrator` / `getAdminByEmail` / `getAdminListByStatus` | 管理员管理 |

---

## 4. 文件（file 服务，经网关透传）

| 接口 | 说明 |
| --- | --- |
| `uploadPhoto` | 上传图片（头像/封面），返回 `fileId` |
| `downloadPhoto` | 按 `fileId` 下载图片 |
| `uploadVideo` | 上传视频（原始文件），返回 `fileId` |
| `downloadVideo` | 按 `fileId` 下载（转码完成后是 m3u8 文本） |

```bash
# 上传图片
curl -s -X POST "http://127.0.0.1:10000/HttpService/uploadPhoto?sessionId=$SID&requestId=R3" \
     -H 'Content-Type: image/png' --data-binary @avatar.png
# {"errorCode":0,"result":{"fileId":"DFA7toGbH6hiOhKQ0001"}}

# 下载图片（字节一致）
curl -s "http://127.0.0.1:10000/HttpService/downloadPhoto?sessionId=$SID&requestId=R4&fileId=DFA7toGbH6hiOhKQ0001" -o out.png

# 上传视频
curl -s -X POST "http://127.0.0.1:10000/HttpService/uploadVideo?sessionId=$SID&requestId=R5" \
     -H 'Content-Type: video/mp4' --data-binary @movie.mp4
# {"errorCode":0,"result":{"fileId":"N0z7Fz0KSXUzSnOy0002"}}
```

---

## 5. 视频（video 服务）

| 接口 | 说明 |
| --- | --- |
| `newVideo` | 新增视频（投递转码消息） |
| `removeVideo` | 删除视频（投递删除文件消息） |
| `checkVideo` / `saleVideo` / `haltVideo` | 管理员审核 / 上架 / 下架 |
| `judgeLike` / `setLike` | 是否点赞 / 点赞或取消 |
| `setPlay` | 播放计数 +1 |
| `newBarrage` / `getBarrage` | 发弹幕 / 拉弹幕 |
| `userVideoList` / `statusVideoList` / `allVideoList` / `typeVideoList` / `tagVideoList` / `keyVideoList` | 各类列表（**pageIndex 从 1 开始**） |

```bash
# 新增视频（videoType=分类，videoTag=标签 id 数组）
curl -s -X POST http://127.0.0.1:10000/HttpService/newVideo -H 'Content-Type: application/json' -d '{
  "requestId":"R6","sessionId":"'"$SID"'",
  "videoInfo":{"videoFileId":"...","photoFileId":"...","videoTitle":"测试视频",
               "videoDesc":"demo","duration":10,"videoType":1,"videoTag":[1,2]}
}'

# 首页列表（pageIndex 从 1 开始！传 0 会得到空列表）
curl -s -X POST http://127.0.0.1:10000/HttpService/allVideoList -H 'Content-Type: application/json' \
     -d '{"requestId":"R7","sessionId":"'"$SID"'","pageIndex":1,"pageCount":10}'

# 点赞 / 播放 / 弹幕
curl -s -X POST http://127.0.0.1:10000/HttpService/setLike    -d '{"requestId":"R8","sessionId":"'"$SID"'","videoId":"'"$VID"'"}'
curl -s -X POST http://127.0.0.1:10000/HttpService/setPlay    -d '{"requestId":"R9","sessionId":"'"$SID"'","videoId":"'"$VID"'"}'
curl -s -X POST http://127.0.0.1:10000/HttpService/newBarrage -d '{"requestId":"R10","sessionId":"'"$SID"',
     "videoId":"'"$VID"'","barrageInfo":{"videoId":"'"$VID"'","barrageContent":"第一条弹幕","barrageTime":1200}}'
```

### 转码与播放

1. `newVideo` 成功后，video 服务向 RabbitMQ 的 `hls_transcode_queue` 投递消息，视频状态为「待审核(1)」；
2. transcode 服务消费消息：从 FastDFS 下载原始视频 → FFmpeg 切片（`hls_time=10`）→ 上传 m3u8 + ts →
   用 m3u8 的 `file_id` 覆盖视频的 `video_fid`；
3. 管理员审核通过（`checkVideo`）后状态变「上架(2)」，出现在 `allVideoList`；
4. 客户端 `downloadVideo` 拿到 m3u8 文本，再按其中的 `/HttpService/downloadVideo/?fileId=xxx` 拉取 ts 分片。

示例 m3u8：

```text
#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:0
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-INDEPENDENT-SEGMENTS
#EXTINF:10.143000,
/HttpService/downloadVideo/?fileId=QYlPCHVl8Ytz7Kls0000
#EXTINF:2.602000,
/HttpService/downloadVideo/?fileId=HNRZPniGloBrWG610001
#EXT-X-ENDLIST
```

---

## 6. 与客户端（BitPlayer）的对应关系

- `client/BitPlayer/netclient/netclient.cpp` 里每个 `xxxAsync` 方法对应上表一个接口；
- 客户端从可执行文件同目录的 `config.ini` 读取网关地址：

```ini
[server]
serverIp=192.168.131.128
serverPort=10000
```

- 客户端用 JSON 传参（`QJsonObject`），文件上传统一走原始请求体，与网关的 attachment 读取方式一致。
