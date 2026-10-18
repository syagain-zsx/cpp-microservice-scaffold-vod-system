/*
    局部测试套件使用：
        1. 包含头文件 <gtest/gtest.h>
        2. 定义测试套件
            1. 定义测试套件初始接口：会在每个测试用例执行之前执行
            2. 定义测试套件清理接口：会在每个测试用例执行之后执行
        3. 定义测试用例
        4. 执行测试用例
*/
#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

class LocalTestSuite : public testing::Test {
    public:
        static void SetUpTestCase() {
            std::cout << "公共环境初始接口--会在所有用例执行之前执行\n";
        }
        static void TearDownTestCase() {
            std::cout << "公共环境清理接口--会在所有用例执行之后执行\n";
        }
        virtual void SetUp() {
            std::cout << "执行于每个用例之前" << std::endl;
            map.insert(std::make_pair(1, 1));
            map.insert(std::make_pair(2, 2));
            map.insert(std::make_pair(3, 3));
        }
        virtual void TearDown() {
            std::cout << "执行于每个用例之后" << std::endl;
            map.clear();
        }
    public:
        std::unordered_map<int, int> map;
};

TEST_F(LocalTestSuite, find) {
    ASSERT_EQ(map.size(), 3);
    ASSERT_EQ(map.find(1)->second, 1);
    map.erase(1);
    std::cout << "测试用例1" << std::endl;
}

TEST_F(LocalTestSuite, remove) {
    map.erase(1);
    ASSERT_EQ(map.size(), 2);
    std::cout << "测试用例2" << std::endl;
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}