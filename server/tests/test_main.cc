#include <gtest/gtest.h>
#include <JianLiuScaffold/log.h>

int main(int argc, char **argv) {
    JianLiuLog::log_settings settings;
    settings.async = false;
    settings.level = 6;          // off：单测不输出日志
    settings.path = "stdout";
    JianLiuLog::init(settings);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
