# HLS 与 FFmpeg 转码（本项目实现）

> 代码：`scaffold/JianLiuScaffold/hls.cc`（remux + m3u8 解析）、`server/svc_transcode`（消费/上传/状态更新）

---

## 1. 原理速览

### 1.1 容器 vs 编码

- **容器**：MP4 / TS / FLV，装视频、音频、字幕等轨道；
- **编码**：H.264 / H.265 / AAC，决定压缩方式；
- **HLS**：把视频切成若干 `.ts` 分片，用 `.m3u8` 播放列表描述顺序与时长，客户端按 HTTP 拉取。

### 1.2 为什么点播用 HLS

- 基于 HTTP，CDN 友好、穿透防火墙；
- 分片可缓存、可自适应码率（多码率 playlist）；
- 延迟秒级（点播不敏感），直播可用 LL-HLS 降低延迟。

### 1.3 remux 与 transcode

| | 做什么 | 开销 | 本项目 |
| --- | --- | --- | --- |
| remux（转封装） | 只换容器，不重编码（`avcodec_parameters_copy`） | 很低 | ✅ 用的这个 |
| transcode（转码） | 解码再编码（改分辨率/码率） | 高（CPU/GPU） | 未做，可扩展 |

---

## 2. 本项目怎么做的

### 2.1 FFmpeg 调用流程（`HLSTranscoder::transcode`）

```text
1. avformat_open_input(&in, input)          打开 mp4
2. avformat_find_stream_info(in)            解析流信息（编码、时长）
3. avformat_alloc_output_context2(&out, "hls", output.m3u8)
4. for each stream:
     avformat_new_stream(out)
     avcodec_parameters_copy(out->codecpar, in->codecpar)   // 复制编码参数（不重编码）
5. av_dict_set: hls_time=10, hls_base_url=..., hls_playlist_type=vod,
                hls_flags=independent_segments
6. avformat_write_header(out, &dict)
7. while (av_read_frame(in, &pkt) >= 0):
     av_packet_rescale_ts(pkt, in->time_base, out->time_base)
     av_interleaved_write_frame(out, &pkt)
8. av_write_trailer(out)                     写出 #EXT-X-ENDLIST
```

### 2.2 生成的 m3u8

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

分片 URL = `hls_base_url` + 上传 FastDFS 后拿到的 `file_id`（由 transcode 服务改写）。

### 2.3 转码服务链路（`svc_transcode`）

```text
MQ(hls_transcode_queue) → TranscodeMQ::transcode(video_id)
  1. 查视频元信息（video → file_id）
  2. 查文件元信息（FDFS path）
  3. FDFSClient::download_to_file → temp_path/<video_id>
  4. HLSTranscoder::transcode → temp_path/<video_id>.m3u8 + .ts
  5. M3U8Info::parse 解析分片
  6. 逐个上传 ts（得到 file_id）→ 改写 m3u8 的分片 URL
  7. 上传 m3u8（覆盖视频的 video_fid）
  8. 更新视频状态（待审核）
  9. 清理临时文件
```

### 2.4 播放链路

客户端 `downloadVideo(file_id)` 拿到 m3u8 文本 → 逐行解析出 ts 的 `fileId` →
再 `downloadVideo` 拉取 ts 字节 → 交给 mpv 播放（mpv 也支持直接给 HLS URL）。

---

## 3. 面试问答

**Q：为什么切片时间 10 秒？**
分片越短，起播越快、延迟越低，但请求数多、开销大；点播常用 6–10s。`hls_time` 可配置。

**Q：为什么转码不阻塞上传接口？**
转码耗时与视频大小成正比，同步做会占满连接/线程；用 MQ 异步化，上传只做存储与元信息，
转码失败也不影响源文件（可重试）。

**Q：HLS 和 DASH 的区别？**
HLS 用 m3u8 + ts/fMP4，Apple 生态好；DASH 用 MPD + fMP4，更开放、码率切换更灵活。
点播场景两者都常见，HLS 更简单。

**Q：怎么支持多码率自适应？**
转码时输出多档分辨率/码率，生成 master playlist（`#EXT-X-STREAM-INF`），客户端按带宽选择。
本项目只做单码率 remux。

**Q：切片为什么要求关键帧对齐？**
每个 ts 必须从关键帧开始才能独立解码；`independent_segments` 标志表示分片可独立播放。
remux 不重编码时，如果源文件关键帧间隔大于切片时长，FFmpeg 会按关键帧切（分片可能比 hls_time 长）。

**Q：转码失败怎么处理？**
当前把视频状态置「转码失败」，源文件保留；改进：失败重试（有限次）+ 死信队列 + 告警。

---

## 4. 动手实验

1. 用 `ffprobe`（在 FFmpeg 前缀的 `bin/` 下）看 `tools/file-client/movie.mp4` 的编码与关键帧间隔；
2. 把 `run-dev.sh` 里 `--hls_time=10` 改成 4，重新上传视频，对比 ts 数量与 m3u8；
3. 手动调 `HLSTranscoder::transcode` 的 `hls_base_url`，观察 m3u8 里 URL 的变化；
4. 转码中 `ls temp/transcode`，观察临时文件生成与清理。
