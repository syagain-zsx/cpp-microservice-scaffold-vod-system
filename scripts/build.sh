#!/usr/bin/env bash
# ============================================================
# 一键构建：scaffold + server（客户端请用 Qt Creator / client/CMakeLists.txt）
#
# 用法:
#   ./scripts/build.sh                 # Debug 构建
#   BUILD_TYPE=Release ./scripts/build.sh
#   BITE_FFMPEG_ROOT=/opt/ffmpeg/usr ./scripts/build.sh
# ============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
# 无 root 时脚本 fetch-ffmpeg.sh 默认解包到这里
FFMPEG_PREFIX="${BITE_FFMPEG_ROOT:-$HOME/local/ffmpeg/usr}"

CMAKE_ARGS=(-S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE")

if [[ -d "$FFMPEG_PREFIX" ]]; then
    CMAKE_ARGS+=(-DBITE_FFMPEG_ROOT="$FFMPEG_PREFIX")
    echo "[build] 使用 FFmpeg: $FFMPEG_PREFIX"
else
    echo "[build] 未找到 FFmpeg（$FFMPEG_PREFIX），HLS/transcode 将跳过"
fi

cmake "${CMAKE_ARGS[@]}"
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo
echo "[build] 完成，可执行文件："
find "$BUILD_DIR/server" -maxdepth 1 -type f -executable -printf '  %p\n' 2>/dev/null | sort || true
echo "[build] 安装到 build/bin："
cmake --install "$BUILD_DIR" >/dev/null 2>&1 || true
ls -1 "$BUILD_DIR/bin" 2>/dev/null | sed 's/^/  /' || true
