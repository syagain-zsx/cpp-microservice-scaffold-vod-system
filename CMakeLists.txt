cmake_minimum_required(VERSION 3.16)
project(JianLiu VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# 视频点播系统（VOD）—— 顶层构建入口
#
#   源码组织：
#     scaffold/  JianLiuScaffold 基础库（中间件客户端封装）
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

# FFmpeg 使用独立前缀时，把库目录写进 RPATH，避免每次都要设置 LD_LIBRARY_PATH
if(JIANLIU_FFMPEG_ROOT)
    set(CMAKE_BUILD_RPATH "${JIANLIU_FFMPEG_ROOT}/lib;${JIANLIU_FFMPEG_ROOT}/lib/x86_64-linux-gnu")
    set(CMAKE_INSTALL_RPATH "${JIANLIU_FFMPEG_ROOT}/lib;${JIANLIU_FFMPEG_ROOT}/lib/x86_64-linux-gnu")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # 使用旧式 DT_RPATH（可被传递依赖继承），否则 libavformat 的间接依赖
    # （libgme/libvpx 等）在运行时找不到
    add_link_options(-Wl,--disable-new-dtags)
endif()

add_subdirectory(scaffold)
add_subdirectory(server)

# ------------------------------------------------------------
# 单元测试（可关闭：-DBUILD_TESTS=OFF）
# ------------------------------------------------------------
option(BUILD_TESTS "构建单元测试" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(server/tests)
endif()
