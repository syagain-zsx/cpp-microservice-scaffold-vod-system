#!/usr/bin/env python3
"""简单压测：对网关的只读接口做并发请求，输出 QPS 与延迟分位。

用法:
    python3 scripts/bench.py [--gateway URL] [--path /HttpService/allVideoList]
                             [--concurrency 20] [--total 500]

默认场景：先 tempLogin 拿会话，然后并发请求 allVideoList（读路径：Redis + MySQL + RPC）。
"""
import argparse
import json
import statistics
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor


def post(gateway: str, path: str, body: dict) -> dict:
    data = json.dumps(body).encode()
    req = urllib.request.Request(gateway + path, data=data,
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=10) as resp:
        return json.loads(resp.read().decode())


def call(gateway: str, path: str, body: dict) -> float:
    data = json.dumps(body).encode()
    req = urllib.request.Request(gateway + path, data=data,
                                 headers={"Content-Type": "application/json"})
    start = time.perf_counter()
    with urllib.request.urlopen(req, timeout=10) as resp:
        resp.read()
    return (time.perf_counter() - start) * 1000  # ms


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gateway", default="http://127.0.0.1:10000")
    ap.add_argument("--path", default="/HttpService/allVideoList")
    ap.add_argument("--concurrency", type=int, default=20)
    ap.add_argument("--total", type=int, default=500)
    args = ap.parse_args()

    login = post(args.gateway, "/HttpService/tempLogin", {"requestId": "bench-login"})
    sid = login["result"]["sessionId"]
    body = {"requestId": "bench", "sessionId": sid, "pageIndex": 1, "pageCount": 10}

    latencies = []
    start = time.perf_counter()

    def one(_):
        return call(args.gateway, args.path, body)

    with ThreadPoolExecutor(max_workers=args.concurrency) as pool:
        for ms in pool.map(one, range(args.total)):
            latencies.append(ms)

    elapsed = time.perf_counter() - start
    latencies.sort()

    def pct(p):
        return latencies[min(len(latencies) - 1, int(len(latencies) * p / 100))]

    print(f"gateway={args.gateway} path={args.path}")
    print(f"requests={args.total} concurrency={args.concurrency} elapsed={elapsed:.2f}s")
    print(f"QPS={args.total / elapsed:.1f}")
    print(f"latency(ms): avg={statistics.mean(latencies):.2f} "
          f"p50={pct(50):.2f} p90={pct(90):.2f} p99={pct(99):.2f} "
          f"max={latencies[-1]:.2f}")


if __name__ == "__main__":
    main()
