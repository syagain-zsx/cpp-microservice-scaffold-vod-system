cmake_minimum_required(VERSION 3.16)

# ============================================================
# 单元测试（gtest）
#   只测纯逻辑：JSON 工具、M3U8 解析
#   依赖数据库/中间件的集成测试见 scripts/smoke-test.sh
# ============================================================

find_package(GTest REQUIRED)

add_executable(vod_unit_tests
    test_main.cc
    test_json.cc
    test_m3u8.cc
)

target_link_libraries(vod_unit_tests PRIVATE
    JianLiuScaffold::JianLiuScaffold
    GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(vod_unit_tests)
