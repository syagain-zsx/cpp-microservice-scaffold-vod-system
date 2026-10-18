cmake_minimum_required(VERSION 3.16)
project(JianLiuScaffold VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# JianLiuScaffold —— 微服务脚手架基础库
#   把项目中反复出现的中间件客户端封装为可复用组件：
#   日志 / JSON / RPC / 注册中心 / 消息队列 / 缓存 / ORM /
#   搜索引擎 / 分布式文件系统 / HLS 转码 / 邮件验证码
#
#   对外只暴露一个 target：JianLiuScaffold::JianLiuScaffold
#   （头文件按 <JianLiuScaffold/xxx.h> 方式引入）
# ============================================================

option(JIANLIU_ENABLE_HLS "启用基于 FFmpeg 的 HLS 转码模块" ON)
option(JIANLIU_ENABLE_FDFS "启用 FastDFS 文件存储模块" ON)

set(JIANLIU_SCAFFOLD_SOURCES
    JianLiuScaffold/log.cc
    JianLiuScaffold/util.cc
    JianLiuScaffold/rpc.cc
    JianLiuScaffold/etcd.cc
    JianLiuScaffold/mq.cc
    JianLiuScaffold/redis.cc
    JianLiuScaffold/odb.cc
    JianLiuScaffold/elastic.cc
    JianLiuScaffold/mail.cc
)

if(JIANLIU_ENABLE_FDFS)
    list(APPEND JIANLIU_SCAFFOLD_SOURCES JianLiuScaffold/fdfs.cc)
endif()

if(JIANLIU_ENABLE_HLS)
    list(APPEND JIANLIU_SCAFFOLD_SOURCES JianLiuScaffold/hls.cc)
endif()

# ------------------------------------------------------------
# 第三方依赖定位：全部是系统包（apt / 源码安装），
# 用 find_library 直接找，避免引入 pkg-config 依赖。
# ------------------------------------------------------------
function(JianLiu_find_library var name)
    find_library(${var} NAMES ${name} ${ARGN})
    if(NOT ${var})
        message(FATAL_ERROR "未找到依赖库: ${name} (${ARGN})，请先安装后重试")
    endif()
endfunction()

find_path(JSONCPP_INCLUDE_DIR jsoncpp/json/json.h)
find_path(FDFS_INCLUDE_DIR fastdfs/fdfs_client.h)

JianLiu_find_library(SPDLOG_LIBRARY     spdlog)
JianLiu_find_library(FMT_LIBRARY        fmt)
JianLiu_find_library(JSONCPP_LIBRARY    jsoncpp)
JianLiu_find_library(BRPC_LIBRARY       brpc)
JianLiu_find_library(PROTOBUF_LIBRARY   protobuf)
JianLiu_find_library(LEVELDB_LIBRARY    leveldb)
JianLiu_find_library(GFLAGS_LIBRARY     gflags)
JianLiu_find_library(SSL_LIBRARY        ssl)
JianLiu_find_library(CRYPTO_LIBRARY     crypto)
JianLiu_find_library(Z_LIBRARY          z)
JianLiu_find_library(ETCD_LIBRARY       etcd-cpp-api)
JianLiu_find_library(CPPREST_LIBRARY    cpprest)
JianLiu_find_library(AMQPCPP_LIBRARY    amqpcpp)
JianLiu_find_library(EV_LIBRARY         ev)
JianLiu_find_library(REDISPP_LIBRARY    redis++)
JianLiu_find_library(HIREDIS_LIBRARY    hiredis)
JianLiu_find_library(ODB_LIBRARY        odb)
JianLiu_find_library(ODB_BOOST_LIBRARY  odb-boost)
JianLiu_find_library(ODB_MYSQL_LIBRARY  odb-mysql)
JianLiu_find_library(MYSQLCLIENT_LIBRARY mysqlclient)
JianLiu_find_library(ELASTICLIENT_LIBRARY elasticlient)
JianLiu_find_library(CPR_LIBRARY        cpr)
JianLiu_find_library(CURL_LIBRARY       curl)

if(JIANLIU_ENABLE_FDFS)
    JianLiu_find_library(FDFS_LIBRARY     fdfsclient)
    JianLiu_find_library(FASTCOMMON_LIBRARY fastcommon)
endif()

# ------------------------------------------------------------
# FFmpeg：允许使用独立前缀（系统未安装 dev 包时）
#   例：cmake -DJIANLIU_FFMPEG_ROOT=/path/to/ffmpeg
# ------------------------------------------------------------
if(JIANLIU_ENABLE_HLS)
    if(JIANLIU_FFMPEG_ROOT)
        set(_ffmpeg_include_hints ${JIANLIU_FFMPEG_ROOT}/include
                                  ${JIANLIU_FFMPEG_ROOT}/include/x86_64-linux-gnu)
        set(_ffmpeg_lib_hints     ${JIANLIU_FFMPEG_ROOT}/lib
                                  ${JIANLIU_FFMPEG_ROOT}/lib/x86_64-linux-gnu)
    endif()

    find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h HINTS ${_ffmpeg_include_hints})
    find_library(AVFORMAT_LIBRARY NAMES avformat HINTS ${_ffmpeg_lib_hints})
    find_library(AVCODEC_LIBRARY  NAMES avcodec  HINTS ${_ffmpeg_lib_hints})
    find_library(AVUTIL_LIBRARY   NAMES avutil   HINTS ${_ffmpeg_lib_hints})

    if(NOT AVFORMAT_INCLUDE_DIR OR NOT AVFORMAT_LIBRARY OR NOT AVCODEC_LIBRARY OR NOT AVUTIL_LIBRARY)
        message(WARNING "未找到 FFmpeg 开发包，HLS 转码模块将被禁用。"
                        "安装 libavformat-dev/libavcodec-dev/libavutil-dev 或指定 JIANLIU_FFMPEG_ROOT 后重新配置。")
        set(JIANLIU_ENABLE_HLS OFF)
        list(REMOVE_ITEM JIANLIU_SCAFFOLD_SOURCES JianLiuScaffold/hls.cc)
    endif()
endif()

# ------------------------------------------------------------
# 构建静态库
# ------------------------------------------------------------
add_library(JianLiuScaffold STATIC ${JIANLIU_SCAFFOLD_SOURCES})
add_library(JianLiuScaffold::JianLiuScaffold ALIAS JianLiuScaffold)

target_compile_features(JianLiuScaffold PUBLIC cxx_std_17)

target_include_directories(JianLiuScaffold PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JSONCPP_INCLUDE_DIR}
)

if(JIANLIU_ENABLE_FDFS)
    target_include_directories(JianLiuScaffold PUBLIC ${FDFS_INCLUDE_DIR})
endif()

if(JIANLIU_ENABLE_HLS AND AVFORMAT_INCLUDE_DIR)
    target_include_directories(JianLiuScaffold PUBLIC ${AVFORMAT_INCLUDE_DIR})
endif()

find_package(Threads REQUIRED)

target_link_libraries(JianLiuScaffold PUBLIC
    ${SPDLOG_LIBRARY}
    ${FMT_LIBRARY}
    ${JSONCPP_LIBRARY}
    ${BRPC_LIBRARY}
    ${PROTOBUF_LIBRARY}
    ${LEVELDB_LIBRARY}
    ${GFLAGS_LIBRARY}
    ${SSL_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${Z_LIBRARY}
    ${ETCD_LIBRARY}
    ${CPPREST_LIBRARY}
    ${AMQPCPP_LIBRARY}
    ${EV_LIBRARY}
    ${REDISPP_LIBRARY}
    ${HIREDIS_LIBRARY}
    ${ODB_LIBRARY}
    ${ODB_BOOST_LIBRARY}
    ${ODB_MYSQL_LIBRARY}
    ${MYSQLCLIENT_LIBRARY}
    ${ELASTICLIENT_LIBRARY}
    ${CPR_LIBRARY}
    ${CURL_LIBRARY}
    Threads::Threads
    dl
    rt
)

if(JIANLIU_ENABLE_FDFS)
    target_link_libraries(JianLiuScaffold PUBLIC ${FDFS_LIBRARY} ${FASTCOMMON_LIBRARY})
endif()

if(JIANLIU_ENABLE_HLS)
    target_link_libraries(JianLiuScaffold PUBLIC ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY})
endif()

message(STATUS "JianLiuScaffold: HLS=${JIANLIU_ENABLE_HLS} FDFS=${JIANLIU_ENABLE_FDFS}")
