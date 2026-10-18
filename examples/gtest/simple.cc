/*
    gtest简单使用:
        1. 包含头文件 <gtest/gtest.h>
        2. 定义测试用例
        3. 初始测试环境
        4. 执行测试用例
*/

#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

TEST(unordered_map_test, insert) {
    std::unordered_map<int, int> map;
    map.insert(std::make_pair(1, 1));
    map.insert(std::make_pair(2, 2));
    map.insert(std::make_pair(3, 3));
    ASSERT_EQ(map.size(), 2);
}

TEST(unordered_map_test, find) {
    std::unordered_map<int, int> map;
    map.insert(std::make_pair(1, 0));
    map.insert(std::make_pair(2, 2));
    ASSERT_NE(map.find(1), map.end());
    ASSERT_EQ(map.find(1)->second, 1);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}