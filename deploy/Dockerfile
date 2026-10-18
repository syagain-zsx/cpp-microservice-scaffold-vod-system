# ============================================================
# 笺流（JianLiu）运行镜像
#
# 构建前提（在仓库根目录）：
#   ./scripts/build.sh          # 产出 build/bin/*
#   ./scripts/package.sh        # 收集运行期 .so 到 deploy/depend/
#   docker build -f deploy/Dockerfile -t jianliu/vod:latest .
# ============================================================
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates gettext-base curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY build/bin/ /bin/
COPY deploy/entrypoint.sh /bin/

# 运行期依赖（FFmpeg 等，由 scripts/package.sh 收集）
COPY deploy/depend/ /usr/local/lib/jianliu/
RUN chmod +x /bin/* \
    && echo "/usr/local/lib/jianliu" > /etc/ld.so.conf.d/jianliu.conf \
    && ldconfig

ENTRYPOINT ["/bin/entrypoint.sh"]
