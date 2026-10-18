#!/usr/bin/env python3
"""客户端 ↔ 服务端接口一致性检查（静态分析，不需要编译客户端）。

检查两件事：
  1. 客户端调用的 /HttpService/<方法> 是否都在 server/proto/http.proto 中定义；
  2. 客户端使用的 JSON 字段名是否都能在 proto 字段中找到（过滤本地配置键）。

用法:
    python3 scripts/check-client-api.py
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# 客户端本地 JSON（config.ini 持久化 / 测试接口），不是 proto 字段
LOCAL_KEYS = {"identityType", "loginSessionId", "hello", "ping", "data", "result", "errorCode"}


def main():
    proto = open(os.path.join(ROOT, "server/proto/http.proto"), encoding="utf-8").read()
    server_rpcs = set(re.findall(r"rpc\s+(\w+)\s*\(", proto))

    client_paths = set()
    for f in glob.glob(os.path.join(ROOT, "client/JianLiuPlayer/netclient/*.cpp")):
        for m in re.findall(r"/HttpService/(\w+)", open(f, encoding="utf-8").read()):
            client_paths.add(m)

    fields = set()
    for f in glob.glob(os.path.join(ROOT, "server/proto/*.proto")):
        text = open(f, encoding="utf-8").read()
        fields.update(re.findall(
            r"(?:optional\s+|repeated\s+|required\s+)?[\w\.]+\s+(\w+)\s*=\s*\d+\s*;", text))

    client_keys = set()
    for pattern in ("client/JianLiuPlayer/netclient/*.cpp", "client/JianLiuPlayer/model/*.cpp"):
        for f in glob.glob(os.path.join(ROOT, pattern)):
            client_keys.update(re.findall(r'\["(\w+)"\]', open(f, encoding="utf-8").read()))

    missing_api = sorted(client_paths - server_rpcs)
    unused_api = sorted(server_rpcs - client_paths)
    unknown_keys = sorted(client_keys - fields - LOCAL_KEYS)

    print(f"服务端 rpc: {len(server_rpcs)}，客户端调用: {len(client_paths)}")
    print(f"proto 字段: {len(fields)}，客户端 JSON 键: {len(client_keys)}")

    ok = True
    if missing_api:
        ok = False
        print(f"[FAIL] 客户端调用了服务端不存在的方法: {missing_api}")
    else:
        print("[PASS] 客户端调用的接口全部存在")

    if unknown_keys:
        ok = False
        print(f"[FAIL] 客户端使用了 proto 中不存在的字段: {unknown_keys}")
    else:
        print("[PASS] 客户端 JSON 字段均能在 proto 中找到")

    if unused_api:
        print(f"[INFO] 服务端有 {len(unused_api)} 个接口客户端未调用: {unused_api}")

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
