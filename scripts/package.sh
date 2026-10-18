#!/usr/bin/env bash
# ============================================================
# 打运行时依赖包：把 5 个服务 ldd 出来的第三方 .so 收集到 deploy/depend/
# 供 deploy/Dockerfile COPY 进镜像。
#
#   ./scripts/package.sh
# ============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_DIR="${BIN_DIR:-$ROOT/build/bin}"
DEPEND_DIR="$ROOT/deploy/depend"

# FFmpeg 私有前缀（若使用）
FFMPEG_LIB="${FFMPEG_LIB:-$HOME/local/ffmpeg/usr/lib/x86_64-linux-gnu}"
[[ -d "$FFMPEG_LIB" ]] && export LD_LIBRARY_PATH="$FFMPEG_LIB:${LD_LIBRARY_PATH:-}"

# 这些是系统基础库，镜像里已经有，不重复打包（避免版本冲突）
SKIP_RE='^(ld-linux|libc\.|libm\.|libpthread|libdl\.|librt\.|libstdc\+\+|libgcc_s|libnss_|libresolv|libutil\.|libcrypt\.)'

rm -rf "$DEPEND_DIR"
mkdir -p "$DEPEND_DIR"

count=0
shopt -s nullglob
for bin in "$BIN_DIR"/*_server; do
    echo "[package] 收集 $(basename "$bin") 的依赖"
    while read -r lib; do
        [[ -f "$lib" ]] || continue
        base="$(basename "$lib")"
        [[ "$base" =~ $SKIP_RE ]] && continue
        [[ -f "$DEPEND_DIR/$base" ]] && continue
        cp -L "$lib" "$DEPEND_DIR/"
        count=$((count + 1))
    done < <(ldd "$bin" | awk '/=>/ {print $3}')
done

echo "[package] 共收集 $count 个库 -> $DEPEND_DIR"
echo "[package] 下一步：docker build -f deploy/Dockerfile -t jianliu/vod:latest ."
