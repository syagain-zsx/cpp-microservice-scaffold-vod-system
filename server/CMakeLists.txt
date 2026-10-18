cmake_minimum_required(VERSION 3.16)
project(videoplayer VERSION 1.0 LANGUAGES CXX)

# ============================================================
# 服务端：公共代码 + 5 个子服务
#
#   common/  错误码、缓存同步抽象
#   data/    ODB 数据层（实体 + DAO）
#   proto/   protobuf 接口定义
#   svc_*/   各子服务的 main / rpc / data 装配代码
#
#   生成代码（protoc / odb）由自定义命令产出，只有 .
#   公共代码 + 生成代码打包为 libvideoplayer.a。
# ============================================================

set(DATA_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(PROTO_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(GEN_DIR    ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${GEN_DIR})

find_program(PROTOC_EXECUTABLE protoc)
find_program(ODB_EXECUTABLE odb)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "未找到 protoc，请安装 protobuf-compiler")
endif()
if(NOT ODB_EXECUTABLE)
    message(FATAL_ERROR "未找到 odb 编译器，请安装 ODB")
endif()

# ------------------------------------------------------------
# 代码生成（只在输入变化时重新生成）
# ------------------------------------------------------------
file(GLOB PROTO_FILES  ${PROTO_DIR}/*.proto)
file(GLOB DATA_HEADERS ${DATA_DIR}/*.h)

set(GEN_PROTO_SOURCES)
set(GEN_PROTO_HEADERS)
foreach(proto ${PROTO_FILES})
    get_filename_component(_name ${proto} NAME_WE)
    list(APPEND GEN_PROTO_SOURCES ${GEN_DIR}/${_name}.pb.cc)
    list(APPEND GEN_PROTO_HEADERS ${GEN_DIR}/${_name}.pb.h)
endforeach()

set(GEN_ODB_SOURCES ${GEN_DIR}/data-odb.cxx)
set(GEN_ODB_HEADERS ${GEN_DIR}/data-odb.hxx ${GEN_DIR}/data-odb.ixx)

add_custom_command(
    OUTPUT ${GEN_PROTO_SOURCES} ${GEN_PROTO_HEADERS}
    COMMAND ${PROTOC_EXECUTABLE} --experimental_allow_proto3_optional
            -I ${PROTO_DIR} --cpp_out=${GEN_DIR} ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "protoc: 生成 protobuf 代码"
    VERBATIM)

add_custom_command(
    OUTPUT ${GEN_ODB_SOURCES} ${GEN_ODB_HEADERS}
    COMMAND ${ODB_EXECUTABLE} -d mysql --std c++17 --generate-query --generate-schema
            --profile boost/date-time -I ${DATA_DIR} -o ${GEN_DIR} ${DATA_DIR}/data.h
    DEPENDS ${DATA_DIR}/data.h ${DATA_HEADERS}
    COMMENT "odb: 生成 ODB 代码"
    VERBATIM)

add_custom_target(vod_codegen DEPENDS
    ${GEN_PROTO_SOURCES} ${GEN_PROTO_HEADERS} ${GEN_ODB_SOURCES} ${GEN_ODB_HEADERS})

# ------------------------------------------------------------
# 公共静态库 libvideoplayer.a
# ------------------------------------------------------------
file(GLOB DATA_SOURCES   ${DATA_DIR}/*.cc)
file(GLOB COMMON_SOURCES ${COMMON_DIR}/*.cc)

add_library(videoplayer STATIC
    ${DATA_SOURCES}
    ${COMMON_SOURCES}
    ${GEN_PROTO_SOURCES}
    ${GEN_ODB_SOURCES})
add_dependencies(videoplayer vod_codegen)

target_include_directories(videoplayer PUBLIC
    ${COMMON_DIR}
    ${DATA_DIR}
    ${GEN_DIR}
)
target_link_libraries(videoplayer PUBLIC JianLiuScaffold::JianLiuScaffold)

# ------------------------------------------------------------
# 子服务
#   端口约定：gateway 10000 / file 10001 / user 10002 / video 10003
# ------------------------------------------------------------
set(VOD_SERVICES gateway user video file)
if(JIANLIU_ENABLE_HLS)
    list(APPEND VOD_SERVICES transcode)
else()
    message(FATAL_ERROR "svc_file 依赖 JianLiuScaffold/hls.h，必须启用 FFmpeg（-DJIANLIU_FFMPEG_ROOT=...）")
endif()

foreach(svc ${VOD_SERVICES})
    file(GLOB svc_sources ${CMAKE_CURRENT_SOURCE_DIR}/svc_${svc}/source/*.cc)
    add_executable(${svc}_server ${svc_sources})
    target_include_directories(${svc}_server PRIVATE
        ${COMMON_DIR}
        ${DATA_DIR}
        ${GEN_DIR}
    )
    target_link_libraries(${svc}_server PRIVATE videoplayer JianLiuScaffold::JianLiuScaffold)
    install(TARGETS ${svc}_server DESTINATION bin)
endforeach()

# ------------------------------------------------------------
# 命令行工具（文件服务联调客户端）
# ------------------------------------------------------------
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../tools/file-client/test.cc)
    add_executable(file_client ${CMAKE_CURRENT_SOURCE_DIR}/../tools/file-client/test.cc)
    target_include_directories(file_client PRIVATE ${COMMON_DIR} ${DATA_DIR} ${GEN_DIR})
    target_link_libraries(file_client PRIVATE videoplayer JianLiuScaffold::JianLiuScaffold)
endif()
