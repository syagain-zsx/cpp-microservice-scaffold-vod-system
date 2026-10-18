#!/usr/bin/env bash
# ============================================================
# 本地联调：一键启动 / 停止 5 个服务
#
#   ./scripts/run-dev.sh start      # 启动全部（会先停掉旧的）
#   ./scripts/run-dev.sh start user # 只启动某个
#   ./scripts/run-dev.sh stop
#   ./scripts/run-dev.sh status
#
# 依赖的中间件默认使用 Docker 网络内的容器名：
#   dev-etcd / dev-mysql / dev-redis / dev-rabbitmq / dev-tracker
# 可用环境变量覆盖，例如：
#   MYSQL_HOST=127.0.0.1 REDIS_HOST=127.0.0.1 ./scripts/run-dev.sh start
# ============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_DIR="${BIN_DIR:-$ROOT/build/bin}"
LOG_DIR="${LOG_DIR:-$ROOT/logs}"
mkdir -p "$LOG_DIR" "$ROOT/temp/transcode"

# FFmpeg 私有前缀（无 root 场景，见 scripts/fetch-ffmpeg.sh）
FFMPEG_LIB="${FFMPEG_LIB:-$HOME/local/ffmpeg/usr/lib/x86_64-linux-gnu}"
[[ -d "$FFMPEG_LIB" ]] && export LD_LIBRARY_PATH="$FFMPEG_LIB:${LD_LIBRARY_PATH:-}"

ETCD_ADDR="${ETCD_ADDR:-http://dev-etcd:2379}"
MYSQL_HOST="${MYSQL_HOST:-dev-mysql}"
REDIS_HOST="${REDIS_HOST:-dev-redis}"
MQ_URL="${MQ_URL:-amqp://admin:123456@dev-rabbitmq:5672/}"
ES_URL="${ES_URL:-http://elastic:123456@dev-icsearch:9200/}"
FDFS_TRACKER="${FDFS_TRACKER:-dev-tracker:22122}"
HOST_ADDR="${HOST_ADDR:-127.0.0.1}"

LOG_FLAGS=(
    "--log_level=${LOG_LEVEL:-1}"
    "--log_async=false"
    "--log_format=[%H:%M:%S][%-7l]: %v"
    "--log_path=stdout"
)
MYSQL_FLAGS=(
    "--mysql_host=$MYSQL_HOST"
    "--mysql_port=3306"
    "--mysql_user=root"
    "--mysql_passwd=123456"
    "--mysql_db=vptest"
    "--mysql_connection_pool_size=5"
)
REDIS_FLAGS=(
    "--redis_host=$REDIS_HOST"
    "--redis_port=6379"
    "--redis_user=default"
    "--redis_passwd=123456"
    "--redis_connection_pool_size=5"
)
ETCD_FLAGS=("--registry_center_addr=$ETCD_ADDR")

DEL_FILE_Q='{"exchange": "delete_file_exchange","type": "direct","queue": "delete_file_queue","binding_key": "delete_file_queue"}'
DEL_CACHE_Q='{"exchange": "delete_cache_exchange","type": "delayed","queue": "delete_cache_queue","binding_key": "delete_cache_queue","delayed_ttl":3}'
SYNC_CACHE_Q='{"exchange": "cache_sync_to_db_exchange","type": "delayed","queue": "cache_sync_to_db_queue","binding_key": "cache_sync_to_db_queue","delayed_ttl":30}'
TRANSCODE_Q='{"exchange": "hls_transcode_exchange","type": "direct","queue": "hls_transcode_queue","binding_key": "hls_transcode_queue"}'

start_one() {
    local svc="$1"
    local bin="$BIN_DIR/${svc}_server"
    [[ -x "$bin" ]] || { echo "[run] 缺少可执行文件: $bin（先执行 scripts/build.sh）"; return 1; }

    local -a args=("${LOG_FLAGS[@]}")
    case "$svc" in
        gateway)
            args+=("${ETCD_FLAGS[@]}" "${MYSQL_FLAGS[@]}" "${REDIS_FLAGS[@]}"
                   "--listen_port=10000"
                   "--user_svc_name=user"
                   "--file_svc_name=file"
                   "--video_svc_name=video") ;;
        file)
            args+=("${ETCD_FLAGS[@]}" "${MYSQL_FLAGS[@]}" "${REDIS_FLAGS[@]}"
                   "--listen_port=10001"
                   "--service_name=file" "--service_addr=$HOST_ADDR:10001"
                   "--mq_url=$MQ_URL"
                   "--delete_file_queue_info=$DEL_FILE_Q"
                   "--cache_del_queue_info=$DEL_CACHE_Q"
                   "--fdfs_tracker_host=$FDFS_TRACKER") ;;
        user)
            args+=("${ETCD_FLAGS[@]}" "${MYSQL_FLAGS[@]}" "${REDIS_FLAGS[@]}"
                   "--listen_port=10002"
                   "--service_name=user" "--service_addr=$HOST_ADDR:10002"
                   "--mq_url=$MQ_URL"
                   "--delete_file_queue_info=$DEL_FILE_Q"
                   "--cache_del_queue_info=$DEL_CACHE_Q") ;;
        video)
            args+=("${ETCD_FLAGS[@]}" "${MYSQL_FLAGS[@]}" "${REDIS_FLAGS[@]}"
                   "--listen_port=10003"
                   "--service_name=video" "--service_addr=$HOST_ADDR:10003"
                   "--mq_url=$MQ_URL" "--es_url=$ES_URL"
                   "--delete_file_queue_info=$DEL_FILE_Q"
                   "--transcode_queue_info=$TRANSCODE_Q"
                   "--cache_sync_queue_info=$SYNC_CACHE_Q"
                   "--cache_del_queue_info=$DEL_CACHE_Q") ;;
        transcode)
            # 转码服务只依赖 MySQL + MQ + FastDFS
            args+=("${MYSQL_FLAGS[@]}"
                   "--mq_url=$MQ_URL"
                   "--transcode_queue_info=$TRANSCODE_Q"
                   "--fdfs_tracker_host=$FDFS_TRACKER"
                   "--temp_dir=$ROOT/temp/transcode"
                   "--hls_time=10" "--hls_play_type=vod"
                   "--hls_base_url=/HttpService/downloadVideo/?fileId=") ;;
        *) echo "[run] 未知服务: $svc"; return 1 ;;
    esac

    echo "[run] 启动 $svc ..."
    nohup "$bin" "${args[@]}" > "$LOG_DIR/$svc.log" 2>&1 &
    echo $! > "$LOG_DIR/$svc.pid"
}

stop_all() {
    for pidfile in "$LOG_DIR"/*.pid; do
        [[ -f "$pidfile" ]] || continue
        local pid; pid="$(cat "$pidfile")"
        if kill -0 "$pid" 2>/dev/null; then
            echo "[run] 停止 $(basename "$pidfile" .pid) (pid=$pid)"
            kill "$pid" 2>/dev/null || true
        fi
        rm -f "$pidfile"
    done
}

status_all() {
    for svc in gateway file user video transcode; do
        local pidfile="$LOG_DIR/$svc.pid"
        if [[ -f "$pidfile" ]] && kill -0 "$(cat "$pidfile")" 2>/dev/null; then
            echo "  $svc: 运行中 (pid=$(cat "$pidfile"))"
        else
            echo "  $svc: 已停止"
        fi
    done
}

case "${1:-start}" in
    start)
        if [[ -n "${2:-}" ]]; then
            start_one "$2"
        else
            stop_all
            sleep 1
            for s in file user video gateway transcode; do start_one "$s"; done
        fi
        sleep 2
        status_all
        echo "[run] 日志目录: $LOG_DIR"
        ;;
    stop)   stop_all ;;
    status) status_all ;;
    *) echo "用法: $0 {start [服务]|stop|status}"; exit 1 ;;
esac
