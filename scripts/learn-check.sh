#!/usr/bin/env bash
# ============================================================
# 学习环境自检（只读：不启动/不修改任何东西）
#
#   ./scripts/learn-check.sh
#
# 检查项：
#   1. 中间件 TCP 端口（MySQL/Redis/etcd/RabbitMQ/FastDFS tracker+storage）
#   2. 5 个服务是否在运行
#   3. 网关 /status
#   4. 冒烟测试（smoke-test.sh）
#   5. 单元测试（ctest）
#   6. 客户端接口一致性（check-client-api.py）
#   7. 文档链接（check-doc-links.py）
#
# 退出码：0 = 全通过；1 = 有失败项
# ============================================================
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT" || exit 1

PASS=0
FAIL=0
SKIP=0

ok()   { echo "  [OK]   $1"; PASS=$((PASS + 1)); }
bad()  { echo "  [FAIL] $1"; FAIL=$((FAIL + 1)); }
skip() { echo "  [SKIP] $1"; SKIP=$((SKIP + 1)); }

port_open() { timeout 2 bash -c "echo > /dev/tcp/$1/$2" 2>/dev/null; }

FDFS_TRACKER="${FDFS_TRACKER:-dev-tracker:22122}"
FDFS_STORAGE="${FDFS_STORAGE:-dev-storage:23000}"

echo "== 笺流 · 学习环境自检 =="

# ---------------- 1. 中间件 ----------------
echo "-- 1. 中间件"
MIDDLEWARE=(
  "${MYSQL_HOST:-dev-mysql}:3306"
  "${REDIS_HOST:-dev-redis}:6379"
  "${ETCD_HOST:-dev-etcd}:2379"
  "${RABBITMQ_HOST:-dev-rabbitmq}:5672"
  "${FDFS_TRACKER}"
  "${FDFS_STORAGE}"
)
for hp in "${MIDDLEWARE[@]}"; do
  h="${hp%:*}"
  p="${hp##*:}"
  if port_open "$h" "$p"; then
    ok "$hp"
  else
    bad "$hp 不可达"
  fi
done
if ! port_open "${FDFS_STORAGE%:*}" "${FDFS_STORAGE##*:}"; then
  echo "        提示：FastDFS storage 没起时上传会报 fdfs_recv_response fail result=2；"
  echo "              在宿主机执行 docker start dev-storage"
fi

# ---------------- 2. 服务状态 ----------------
echo "-- 2. 服务进程"
SVC_OUT="$("./scripts/run-dev.sh" status 2>/dev/null || true)"
for svc in gateway file user video transcode; do
  if echo "$SVC_OUT" | grep -q "  $svc: 运行中"; then
    ok "$svc 运行中"
  else
    bad "$svc 未运行（./scripts/run-dev.sh start $svc）"
  fi
done

# ---------------- 3. 网关 ----------------
echo "-- 3. 网关状态"
HTTP="$(curl -s -o /dev/null -w '%{http_code}' --max-time 5 http://127.0.0.1:10000/status 2>/dev/null || true)"
if [[ "$HTTP" == "200" ]]; then
  ok "GET /status -> 200"
else
  bad "GET /status -> ${HTTP:-无响应}"
fi

# ---------------- 4. 冒烟 ----------------
echo "-- 4. 冒烟测试"
if [[ "$HTTP" == "200" ]]; then
  if ./scripts/smoke-test.sh >/tmp/learn-smoke.log 2>&1; then
    ok "$(grep -o '通过 [0-9]* / 失败 [0-9]*' /tmp/learn-smoke.log | tail -1)"
  else
    bad "冒烟测试失败，详见 /tmp/learn-smoke.log"
  fi
else
  skip "网关不可用，跳过冒烟"
fi

# ---------------- 5. 单元测试 ----------------
echo "-- 5. 单元测试"
if [[ -d build ]] && command -v ctest >/dev/null 2>&1; then
  if ctest --test-dir build >/tmp/learn-ctest.log 2>&1; then
    ok "ctest 全部通过（$(grep -o 'out of [0-9]*' /tmp/learn-ctest.log | tail -1)）"
  else
    bad "ctest 失败，详见 /tmp/learn-ctest.log"
  fi
else
  bad "build 目录不存在（先 ./scripts/build.sh）"
fi

# ---------------- 6. 接口一致性 ----------------
echo "-- 6. 接口一致性"
if command -v python3 >/dev/null 2>&1; then
  if python3 scripts/check-client-api.py >/tmp/learn-api.log 2>&1; then
    ok "check-client-api 通过"
  else
    bad "check-client-api 失败，详见 /tmp/learn-api.log"
  fi
else
  skip "没有 python3"
fi

# ---------------- 7. 文档链接 ----------------
echo "-- 7. 文档链接"
if command -v python3 >/dev/null 2>&1; then
  if python3 scripts/check-doc-links.py >/tmp/learn-docs.log 2>&1; then
    ok "$(head -1 /tmp/learn-docs.log)"
  else
    bad "文档链接检查失败，详见 /tmp/learn-docs.log"
  fi
else
  skip "没有 python3"
fi

# ---------------- 汇总 ----------------
echo
echo "== 结果：通过 $PASS / 失败 $FAIL / 跳过 $SKIP =="
echo "检查点提示："
echo "  CP1（第 2 周）：服务全在 + 冒烟 + etcd 查询 + lease diff"
echo "  CP2（第 4 周）：能触发一次转码并 curl 到 m3u8 + 失败分支"
echo "  CP3（第 6 周）：以上全绿 + e2e 18/18 + check-api 38/38 + 模拟面试"
echo "  端到端：先拿会话再 python3 scripts/e2e-test.py --session <sid> --admin（见 cheatsheet 第 3 节）"

[[ "$FAIL" -eq 0 ]]
