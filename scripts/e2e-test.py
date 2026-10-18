#!/usr/bin/env python3
"""笺流（JianLiu）端到端联调脚本。

覆盖：会话 → 上传图片/视频 → 新建视频 → MQ 转码 → HLS(m3u8/ts) → 列表 → 点赞/播放/弹幕
可选：管理员审核上架（--admin）

用法:
    python3 scripts/e2e-test.py --session <已登录会话ID> [--admin]
    python3 scripts/e2e-test.py --gateway http://127.0.0.1:10000 --session ...

说明:
    - 上传/建视频需要「普通用户(role=3)」会话；游客会话会跳过这些步骤。
    - --admin 需要该用户有管理员身份（identify_type=2），否则审核步骤标记 SKIP。
"""
import argparse
import hashlib
import json
import os
import sys
import time
import urllib.parse
import urllib.request

PASS = 0
FAIL = 0
SKIP = 0


def check(name, cond, detail=""):
    global PASS, FAIL
    if cond:
        print(f"  [PASS] {name}")
        PASS += 1
    else:
        print(f"  [FAIL] {name} {detail}")
        FAIL += 1
    return cond


def skip(name, reason=""):
    global SKIP
    print(f"  [SKIP] {name} {reason}")
    SKIP += 1


class Api:
    def __init__(self, gateway):
        self.gateway = gateway.rstrip("/")

    def post(self, method, body, timeout=15):
        req = urllib.request.Request(
            f"{self.gateway}/HttpService/{method}",
            data=json.dumps(body, ensure_ascii=False).encode("utf-8"),
            headers={"Content-Type": "application/json"},
        )
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return json.loads(resp.read().decode("utf-8"))

    def upload(self, method, data, mime, sid, rid, timeout=120):
        url = f"{self.gateway}/HttpService/{method}?" + urllib.parse.urlencode(
            {"sessionId": sid, "requestId": rid})
        req = urllib.request.Request(url, data=data, headers={"Content-Type": mime})
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return json.loads(resp.read().decode("utf-8"))

    def raw_get(self, method, params, timeout=60):
        url = f"{self.gateway}/HttpService/{method}?" + urllib.parse.urlencode(params)
        with urllib.request.urlopen(url, timeout=timeout) as resp:
            return resp.read()


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PHOTO = os.path.join(ROOT, "client/JianLiuMockServer/images/touxiang.png")
VIDEO = os.path.join(ROOT, "tools/file-client/movie.mp4")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gateway", default=os.environ.get("GATEWAY", "http://127.0.0.1:10000"))
    ap.add_argument("--session", default=os.environ.get("E2E_SESSION", ""))
    ap.add_argument("--admin", action="store_true", help="尝试执行管理员审核上架")
    args = ap.parse_args()

    api = Api(args.gateway)
    print(f"== 笺流端到端测试 ({args.gateway}) ==")

    # 1. 会话
    if args.session:
        sid = args.session
    else:
        r = api.post("tempLogin", {"requestId": "e2e-login"})
        sid = r["result"]["sessionId"]
    r = api.post("sessionLogin", {"requestId": "e2e-session", "sessionId": sid})
    check("sessionLogin", r.get("errorCode") == 0, r)
    is_guest = r.get("result", {}).get("isGuest", True)
    print(f"  [INFO] sessionId={sid} isGuest={is_guest}")

    # 2. 公共读接口
    r = api.post("allVideoList", {"requestId": "e2e-list", "sessionId": sid,
                                  "pageIndex": 1, "pageCount": 10})
    check("allVideoList", r.get("errorCode") == 0, r)

    if is_guest:
        skip("上传 / 建视频 / 互动", "游客会话，需要普通用户会话")
        print(f"== 通过 {PASS} / 失败 {FAIL} / 跳过 {SKIP} ==")
        return 0 if FAIL == 0 else 1

    # 3. 当前用户
    r = api.post("getUserInfo", {"requestId": "e2e-me", "sessionId": sid})
    user_id = r.get("result", {}).get("userInfo", {}).get("userId", "")
    check("getUserInfo", bool(user_id), r)
    if not user_id:
        print(f"== 通过 {PASS} / 失败 {FAIL} / 跳过 {SKIP} ==")
        return 1

    # 4. 上传图片并回读
    photo = open(PHOTO, "rb").read()
    r = api.upload("uploadPhoto", photo, "image/png", sid, "e2e-photo")
    photo_fid = r.get("result", {}).get("fileId", "")
    check("uploadPhoto", r.get("errorCode") == 0 and photo_fid, r)

    if photo_fid:
        body = api.raw_get("downloadPhoto", {"sessionId": sid,
                                             "requestId": "e2e-photo-get",
                                             "fileId": photo_fid})
        check("downloadPhoto 字节一致",
              hashlib.md5(body).hexdigest() == hashlib.md5(photo).hexdigest(),
              f"{len(body)} bytes")

    # 5. 上传视频
    video = open(VIDEO, "rb").read()
    r = api.upload("uploadVideo", video, "video/mp4", sid, "e2e-video", timeout=180)
    video_fid = r.get("result", {}).get("fileId", "")
    check("uploadVideo", r.get("errorCode") == 0 and video_fid, r)

    # 6. 新建视频
    title = f"E2E-{int(time.time())}"
    r = api.post("newVideo", {
        "requestId": "e2e-new", "sessionId": sid,
        "videoInfo": {"videoFileId": video_fid, "photoFileId": photo_fid,
                      "videoTitle": title, "videoDesc": "e2e test",
                      "duration": 10, "videoType": 1, "videoTag": [1]},
    })
    check("newVideo", r.get("errorCode") == 0, r)

    # 7. 找到视频 ID（待审核列表）
    vid = ""
    deadline = time.time() + 20
    while time.time() < deadline and not vid:
        r = api.post("userVideoList", {"requestId": "e2e-uv", "sessionId": sid,
                                       "userId": user_id, "videoStatus": 1,
                                       "pageIndex": 1, "pageCount": 50})
        for v in r.get("result", {}).get("videoList", []):
            if v.get("videoTitle") == title:
                vid = v.get("videoId", "")
                break
        time.sleep(1)
    check("新视频出现在待审核列表", bool(vid), f"title={title}")
    if not vid:
        print(f"== 通过 {PASS} / 失败 {FAIL} / 跳过 {SKIP} ==")
        return 1

    # 8. 等待转码完成：downloadVideo 返回 m3u8 文本
    m3u8 = ""
    deadline = time.time() + 60
    while time.time() < deadline:
        body = api.raw_get("downloadVideo", {"sessionId": sid,
                                             "requestId": "e2e-hls",
                                             "fileId": video_fid})
        text = body.decode("utf-8", errors="ignore")
        if text.startswith("#EXTM3U"):
            m3u8 = text
            break
        time.sleep(2)
    check("转码完成（拿到 m3u8）", bool(m3u8))
    if m3u8:
        check("m3u8 含分片", "#EXTINF" in m3u8 and "downloadVideo" in m3u8)
        seg = ""
        for line in m3u8.splitlines():
            line = line.strip()
            if "fileId=" in line:
                seg = line.split("fileId=")[-1]
                break
        if seg:
            ts = api.raw_get("downloadVideo", {"sessionId": sid,
                                               "requestId": "e2e-ts", "fileId": seg})
            check("分片可下载", len(ts) > 0, f"{len(ts)} bytes")

    # 9. 管理员审核上架
    if args.admin:
        r = api.post("checkVideo", {"requestId": "e2e-check", "sessionId": sid,
                                    "videoId": vid, "checkResult": True})
        if r.get("errorCode") == 0:
            check("checkVideo 审核通过", True)
            # 首页列表有 3s 短缓存，轮询等待上架可见
            found = False
            deadline = time.time() + 10
            while time.time() < deadline and not found:
                r = api.post("allVideoList", {"requestId": "e2e-list2", "sessionId": sid,
                                              "pageIndex": 1, "pageCount": 50})
                ids = [v.get("videoId") for v in r.get("result", {}).get("videoList", [])]
                found = vid in ids
                if not found:
                    time.sleep(1)
            check("审核后出现在首页", found)
        elif r.get("errorCode") == 16:  # PERMISSION_DENIED
            skip("checkVideo", "当前用户不是管理员")
        else:
            check("checkVideo", False, r)
    else:
        skip("checkVideo", "未加 --admin")

    # 10. 互动
    r = api.post("setPlay", {"requestId": "e2e-play", "sessionId": sid, "videoId": vid})
    check("setPlay", r.get("errorCode") == 0, r)

    r = api.post("setLike", {"requestId": "e2e-like", "sessionId": sid, "videoId": vid})
    check("setLike", r.get("errorCode") == 0, r)
    r = api.post("judgeLike", {"requestId": "e2e-judge", "sessionId": sid, "videoId": vid})
    check("judgeLike isLike=true", r.get("result", {}).get("isLike") is True, r)

    content = f"e2e 弹幕 {int(time.time())}"
    r = api.post("newBarrage", {"requestId": "e2e-barrage", "sessionId": sid, "videoId": vid,
                                "barrageInfo": {"videoId": vid, "barrageContent": content,
                                                "barrageTime": 1000}})
    check("newBarrage", r.get("errorCode") == 0, r)
    r = api.post("getBarrage", {"requestId": "e2e-barrage-get", "sessionId": sid, "videoId": vid})
    texts = [b.get("barrageContent") for b in r.get("result", {}).get("barrageList", [])]
    check("getBarrage 含新弹幕", content in texts, texts)

    print(f"== 通过 {PASS} / 失败 {FAIL} / 跳过 {SKIP} ==")
    return 0 if FAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
