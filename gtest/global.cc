/*
    1.b包含头文件 <gtest/gtest.h>，它包含了 Google Test 的所有功能。
    2.定义全局测试环境类
        2.1定义测试前的准备工作 SetUp()，测试后的清理工作 TearDown()。
        2.2重写 SetUp() 和 TearDown() 方法。
    3.在 main() 函数中注册全局测试环境类，并调用 RUN_ALL_TESTS() 运行所有测试用例。
*/

#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

std::unordered_map<std::string, std::string> dict;
class GlobalTestEnvironment : public ::testing::Environment
{
public:
    void SetUp() override
    {
        // 在这里进行全局测试环境的初始化工作
        std::cout << "GlobalTestEnvironment SetUp()" << std::endl;
        std::cout << "测试前:提前准备数据!!\n";
        dict.insert(std::make_pair("Hello", "你好"));
        dict.insert(std::make_pair("hello", "你好"));
        dict.insert(std::make_pair("雷吼", "你好"));
    }
    void TearDown() override
    {
        std::cout << "GlobalTestEnvironment TearDown()" << std::endl;
        // 在这里进行全局测试环境的清理工作
        std::cout << "测试后:清理数据!!\n";
        dict.clear();
    }
};
TEST(GlobalTest, insert)
{

    EXPECT_EQ(dict.size(), 3);
}
TEST(GlobalTest, find)
{
    auto it = dict.find("Hello");
    EXPECT_NE(it, dict.end());
    EXPECT_EQ(it->second, "你好");
}
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // 注册全局测试环境
    ::testing::AddGlobalTestEnvironment(new GlobalTestEnvironment());
    return RUN_ALL_TESTS();
}
