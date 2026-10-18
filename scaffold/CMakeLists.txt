cmake_minimum_required(VERSION 3.16)
project(bite_scaffold VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# bite_scaffold —— 微服务脚手架基础库
#   把项目中反复出现的中间件客户端封装为可复用组件：
#   日志 / JSON / RPC / 注册中心 / 消息队列 / 缓存 / ORM /
#   搜索引擎 / 分布式文件系统 / HLS 转码 / 邮件验证码
#
#   对外只暴露一个 target：bite_scaffold::bite_scaffold
#   （头文件按 <bite_scaffold/xxx.h> 方式引入）
# ============================================================

option(BITE_ENABLE_HLS "启用基于 FFmpeg 的 HLS 转码模块" ON)
option(BITE_ENABLE_FDFS "启用 FastDFS 文件存储模块" ON)

set(BITE_SCAFFOLD_SOURCES
    bite_scaffold/log.cc
    bite_scaffold/util.cc
    bite_scaffold/rpc.cc
    bite_scaffold/etcd.cc
    bite_scaffold/mq.cc
    bite_scaffold/redis.cc
    bite_scaffold/odb.cc
    bite_scaffold/elastic.cc
    bite_scaffold/mail.cc
)

if(BITE_ENABLE_FDFS)
    list(APPEND BITE_SCAFFOLD_SOURCES bite_scaffold/fdfs.cc)
endif()

if(BITE_ENABLE_HLS)
    list(APPEND BITE_SCAFFOLD_SOURCES bite_scaffold/hls.cc)
endif()

# ------------------------------------------------------------
# 第三方依赖定位：全部是系统包（apt / 源码安装），
# 用 find_library 直接找，避免引入 pkg-config 依赖。
# ------------------------------------------------------------
function(bite_find_library var name)
    find_library(${var} NAMES ${name} ${ARGN})
    if(NOT ${var})
        message(FATAL_ERROR "未找到依赖库: ${name} (${ARGN})，请先安装后重试")
    endif()
endfunction()

find_path(JSONCPP_INCLUDE_DIR jsoncpp/json/json.h)
find_path(FDFS_INCLUDE_DIR fastdfs/fdfs_client.h)

bite_find_library(SPDLOG_LIBRARY     spdlog)
bite_find_library(FMT_LIBRARY        fmt)
bite_find_library(JSONCPP_LIBRARY    jsoncpp)
bite_find_library(BRPC_LIBRARY       brpc)
bite_find_library(PROTOBUF_LIBRARY   protobuf)
bite_find_library(LEVELDB_LIBRARY    leveldb)
bite_find_library(GFLAGS_LIBRARY     gflags)
bite_find_library(SSL_LIBRARY        ssl)
bite_find_library(CRYPTO_LIBRARY     crypto)
bite_find_library(Z_LIBRARY          z)
bite_find_library(ETCD_LIBRARY       etcd-cpp-api)
bite_find_library(CPPREST_LIBRARY    cpprest)
bite_find_library(AMQPCPP_LIBRARY    amqpcpp)
bite_find_library(EV_LIBRARY         ev)
bite_find_library(REDISPP_LIBRARY    redis++)
bite_find_library(HIREDIS_LIBRARY    hiredis)
bite_find_library(ODB_LIBRARY        odb)
bite_find_library(ODB_BOOST_LIBRARY  odb-boost)
bite_find_library(ODB_MYSQL_LIBRARY  odb-mysql)
bite_find_library(MYSQLCLIENT_LIBRARY mysqlclient)
bite_find_library(ELASTICLIENT_LIBRARY elasticlient)
bite_find_library(CPR_LIBRARY        cpr)
bite_find_library(CURL_LIBRARY       curl)

if(BITE_ENABLE_FDFS)
    bite_find_library(FDFS_LIBRARY     fdfsclient)
    bite_find_library(FASTCOMMON_LIBRARY fastcommon)
endif()

# ------------------------------------------------------------
# FFmpeg：允许使用独立前缀（系统未安装 dev 包时）
#   例：cmake -DBITE_FFMPEG_ROOT=/path/to/ffmpeg
# ------------------------------------------------------------
if(BITE_ENABLE_HLS)
    if(BITE_FFMPEG_ROOT)
        set(_ffmpeg_include_hints ${BITE_FFMPEG_ROOT}/include
                                  ${BITE_FFMPEG_ROOT}/include/x86_64-linux-gnu)
        set(_ffmpeg_lib_hints     ${BITE_FFMPEG_ROOT}/lib
                                  ${BITE_FFMPEG_ROOT}/lib/x86_64-linux-gnu)
    endif()

    find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h HINTS ${_ffmpeg_include_hints})
    find_library(AVFORMAT_LIBRARY NAMES avformat HINTS ${_ffmpeg_lib_hints})
    find_library(AVCODEC_LIBRARY  NAMES avcodec  HINTS ${_ffmpeg_lib_hints})
    find_library(AVUTIL_LIBRARY   NAMES avutil   HINTS ${_ffmpeg_lib_hints})

    if(NOT AVFORMAT_INCLUDE_DIR OR NOT AVFORMAT_LIBRARY OR NOT AVCODEC_LIBRARY OR NOT AVUTIL_LIBRARY)
        message(WARNING "未找到 FFmpeg 开发包，HLS 转码模块将被禁用。"
                        "安装 libavformat-dev/libavcodec-dev/libavutil-dev 或指定 BITE_FFMPEG_ROOT 后重新配置。")
        set(BITE_ENABLE_HLS OFF)
        list(REMOVE_ITEM BITE_SCAFFOLD_SOURCES bite_scaffold/hls.cc)
    endif()
endif()

# ------------------------------------------------------------
# 构建静态库
# ------------------------------------------------------------
add_library(bite_scaffold STATIC ${BITE_SCAFFOLD_SOURCES})
add_library(bite_scaffold::bite_scaffold ALIAS bite_scaffold)

target_compile_features(bite_scaffold PUBLIC cxx_std_17)

target_include_directories(bite_scaffold PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JSONCPP_INCLUDE_DIR}
)

if(BITE_ENABLE_FDFS)
    target_include_directories(bite_scaffold PUBLIC ${FDFS_INCLUDE_DIR})
endif()

if(BITE_ENABLE_HLS AND AVFORMAT_INCLUDE_DIR)
    target_include_directories(bite_scaffold PUBLIC ${AVFORMAT_INCLUDE_DIR})
endif()

find_package(Threads REQUIRED)

target_link_libraries(bite_scaffold PUBLIC
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

if(BITE_ENABLE_FDFS)
    target_link_libraries(bite_scaffold PUBLIC ${FDFS_LIBRARY} ${FASTCOMMON_LIBRARY})
endif()

if(BITE_ENABLE_HLS)
    target_link_libraries(bite_scaffold PUBLIC ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY})
endif()

message(STATUS "bite_scaffold: HLS=${BITE_ENABLE_HLS} FDFS=${BITE_ENABLE_FDFS}")
