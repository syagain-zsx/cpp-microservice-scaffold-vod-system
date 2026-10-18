#!/usr/bin/env bash
# ============================================================
# 冒烟测试：网关 + 会话 + 视频列表（不依赖登录用户）
#
#   ./scripts/smoke-test.sh
#   GATEWAY=http://192.168.131.128:10000 ./scripts/smoke-test.sh
# ============================================================
set -euo pipefail

GW="${GATEWAY:-http://127.0.0.1:10000}"
PASS=0
FAIL=0

check() {
    local name="$1" actual="$2" expect="$3"
    if [[ "$actual" == *"$expect"* ]]; then
        echo "  [PASS] $name"
        PASS=$((PASS + 1))
    else
        echo "  [FAIL] $name -> $actual"
        FAIL=$((FAIL + 1))
    fi
}

post() {
    curl -s --max-time 10 -X POST "$GW/HttpService/$1" \
         -H 'Content-Type: application/json' -d "$2"
}

echo "== 笺流服务冒烟测试 ($GW) =="

status_code=$(curl -s -o /dev/null -w '%{http_code}' --max-time 5 "$GW/status" || true)
check "网关状态页" "$status_code" "200"

resp=$(post tempLogin '{"requestId":"smoke-1"}')
check "tempLogin" "$resp" '"errorCode":0'

SID=$(printf '%s' "$resp" | python3 -c 'import sys,json;print(json.load(sys.stdin).get("result",{}).get("sessionId",""))' 2>/dev/null || true)
if [[ -n "$SID" ]]; then
    echo "  [INFO] sessionId=$SID"
    check "sessionLogin" "$(post sessionLogin "{\"requestId\":\"smoke-2\",\"sessionId\":\"$SID\"}")" '"errorCode":0'
    check "allVideoList" "$(post allVideoList "{\"requestId\":\"smoke-3\",\"sessionId\":\"$SID\",\"pageIndex\":1,\"pageCount\":10}")" '"errorCode":0'
else
    echo "  [FAIL] 未能获取 sessionId"
    FAIL=$((FAIL + 1))
fi

echo "== 通过 $PASS / 失败 $FAIL =="
[[ $FAIL -eq 0 ]]
