cmake_minimum_required(VERSION 3.16)
project(vod-system VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# 视频点播系统（VOD）—— 顶层构建入口
#
#   源码组织：
#     scaffold/  bite_scaffold 基础库（中间件客户端封装）
#     server/    微服务集群（gateway/user/video/file/transcode）
#     client/    Qt6 桌面客户端（独立工程，见 client/README.md）
#     deploy/    部署配置（docker-compose / 配置文件 / SQL）
#
#   构建：
#     cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
#     cmake --build build -j$(nproc)
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "构建类型" FORCE)
endif()

# 默认安装到 build/bin（与 deploy/Dockerfile 的 COPY ./build/bin/ 保持一致）
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "安装前缀" FORCE)
endif()

add_subdirectory(scaffold)
add_subdirectory(server)
