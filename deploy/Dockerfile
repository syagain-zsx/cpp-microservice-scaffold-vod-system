# 1. 拉取运行环境镜像
FROM crpi-50dgs1uofzodc9pn.cn-chengdu.personal.cr.aliyuncs.com/dev-envronment/ubuntu:22.04
# 2. 设置工作路径
WORKDIR /app
# 3. 拷贝必要文件到容器内：程序文件，启动脚本，运行依赖。
COPY ./build/bin/ /bin/
COPY ./depend/ /usr/lib/
COPY ./entrypoint.sh /bin/
# 4. 为要执行的程序和脚本赋予可执行权限。
RUN chmod +x /bin/*
# 5. 设置入口启动命令
ENTRYPOINT ["/bin/entrypoint.sh"]