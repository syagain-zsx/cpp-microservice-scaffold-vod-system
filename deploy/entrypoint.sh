#!/bin/bash
#备注：当前配置文件名称，以及程序名称也通过环境变量进行获取 CONF_FILE， EXEC_FILE
# 1. 替换配置文件中的变量
# 2. 启动服务
# export LISTEN_PORT=9001
# export REGISTRY_NAME=user
# export REGISTRY_ADDR=192.168.65.128:9001
# export USER_SVC_NAME=user-svc
# export FILE_SVC_NAME=file-svc
# export VIDEO_SVC_NAME=video-svc

envsubst < ${CONF_FILE} > ./server.conf
exec ${EXEC_FILE} --flagfile=./server.conf