#!/usr/bin/env bash
# ============================================================
# 在「无 root」环境下准备 FFmpeg 开发包（Ubuntu 22.04 / 24.04）
#
# 原理：apt-get download 只下载 .deb，不需要 root；
#       dpkg -x 把包解到私有前缀，CMake 用 -DBITE_FFMPEG_ROOT 指过去。
#
# 用法:
#   ./scripts/fetch-ffmpeg.sh [安装前缀]
#   默认前缀: ~/local/ffmpeg
#
# 完成后构建：
#   BITE_FFMPEG_ROOT=~/local/ffmpeg/usr ./scripts/build.sh
# 运行时：
#   export LD_LIBRARY_PATH=~/local/ffmpeg/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
# ============================================================
set -euo pipefail

PREFIX="${1:-$HOME/local/ffmpeg}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

echo "[ffmpeg] 下载 .deb 到 $WORK"
cd "$WORK"

# 开发包（头文件 + .so 链接 + pkg-config）
apt-get download \
    libavformat-dev libavcodec-dev libavutil-dev \
    libswresample-dev libswscale-dev

# 运行库（.so.x）
apt-get download \
    libavformat58 libavcodec58 libavutil56 libswresample3 libswscale5

# 运行库的依赖：让 apt 帮我们解析（仅下载，不安装）
mkdir -p rt
apt-get -o Dir::Cache::archives="$WORK/rt" -o Debug::NoLocking=1 \
        install --download-only -y --no-install-recommends ffmpeg || true

echo "[ffmpeg] 解包到 $PREFIX"
mkdir -p "$PREFIX"
for f in "$WORK"/*.deb "$WORK"/rt/*.deb; do
    [[ -f "$f" ]] && dpkg -x "$f" "$PREFIX"
done

echo
echo "[ffmpeg] 完成：$PREFIX"
echo "  构建： BITE_FFMPEG_ROOT=$PREFIX/usr ./scripts/build.sh"
echo "  运行： export LD_LIBRARY_PATH=$PREFIX/usr/lib/x86_64-linux-gnu:\$LD_LIBRARY_PATH"
