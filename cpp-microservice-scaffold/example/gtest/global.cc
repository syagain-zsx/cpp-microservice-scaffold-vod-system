/*
    全局测试套件使用：
        1. 包含头文件 <gtest/gtest.h>
        2. 定义全局测试环境类
            1. 定义测试环境初始接口：会在所有用例执行前执行
            2. 定义测试环境清理接口：会在所有用例执行后执行
        3. 定义测试用例
        4. 初始化测试环境
        5. 执行测试用例
*/
#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

std::unordered_map<int, int> map;

class GlobalTestEnvironment : public testing::Environment {
    public:
        virtual void SetUp() {
            std::cout << "执行于所有用例之前" << std::endl;
            map.insert(std::make_pair(1, 1));
            map.insert(std::make_pair(2, 2));
            map.insert(std::make_pair(3, 3));
        }
        virtual void TearDown() {
            std::cout << "执行于所有用例之后" << std::endl;
            map.clear();
        }
};

TEST(GlobalTestEnvironment, insert) {
    ASSERT_EQ(map.size(), 3);
    map.erase(1);
}

TEST(GlobalTestEnvironment, find) {
    ASSERT_NE(map.find(1), map.end());
    ASSERT_EQ(map.find(1)->second, 1);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new GlobalTestEnvironment);
    return RUN_ALL_TESTS();
}