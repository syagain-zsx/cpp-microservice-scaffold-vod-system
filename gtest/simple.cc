// gtest 最简单的用法演示
//   编译：make
//   运行：./simple          （全部用例）
//        ./simple --gtest_filter=AddTest.*   （只跑某些用例）

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

// ==================== 1. 被测代码 ====================

int Add(int a, int b)
{
    return a + b;
}

double Divide(double a, double b)
{
    if (b == 0) {
        throw std::runtime_error("divide by zero");
    }
    return a / b;
}

std::string Greet(const std::string& name)
{
    return "Hello, " + name;
}

// ==================== 2. 最简单的测试 ====================
// 语法：TEST(测试套件名, 用例名)。两个名字必须是合法的 C++ 标识符（英文/数字/下划线）。

TEST(AddTest, Basic)          // 套件 AddTest，用例 Basic
{
    EXPECT_EQ(Add(1, 2), 3);  // 相等才通过；失败会继续往下跑
    EXPECT_EQ(Add(-1, 1), 0);
    ASSERT_EQ(Add(100, 200), 300);  // 致命断言；失败就直接结束本用例
}

TEST(AddTest, Compare)        // 同一个套件可以有多个用例
{
    EXPECT_GT(Add(10, 20), 20);  // 大于
    EXPECT_LT(Add(1, 1), 5);     // 小于
    EXPECT_NE(Add(1, 1), 3);     // 不等于
    EXPECT_TRUE(Add(0, 0) == 0); // 结果为真
}

// ==================== 3. 浮点、字符串、异常 ====================

TEST(DivideTest, Normal)
{
    EXPECT_DOUBLE_EQ(Divide(10.0, 4.0), 2.5);
    EXPECT_NEAR(Divide(1.0, 3.0), 0.3333, 1e-4);  // 允许误差 1e-4
}

TEST(DivideTest, Throw)
{
    EXPECT_THROW(Divide(1.0, 0.0), std::runtime_error);  // 必须抛这个异常
    EXPECT_NO_THROW(Divide(1.0, 1.0));                   // 必须不抛异常
}

TEST(GreetTest, String)
{
    EXPECT_STREQ(Greet("world").c_str(), "Hello, world");  // C 风格字符串比较
    EXPECT_EQ(Greet("bob"), "Hello, bob");                 // std::string 可以直接 ==
}

// ==================== 4. 测试夹具 TEST_F ====================
// 多个用例要共用同一套准备/清理代码时用夹具。
// 类必须继承 ::testing::Test，且类名就是 TEST_F 的第一个参数。

class VectorTest : public ::testing::Test {
protected:
    void SetUp() override          // 每个用例开始前都会自动调用
    {
        v_.push_back(1);
        v_.push_back(2);
    }

    void TearDown() override       // 每个用例结束后都会自动调用
    {
        v_.clear();
    }

    std::vector<int> v_;
};
//TEST_F没次都会创建一个新的 VectorTest 对象，所以每个用例都是独立的，互不影响。
TEST_F(VectorTest, SizeIsTwo)
{
    EXPECT_EQ(v_.size(), 2u);
}

TEST_F(VectorTest, FirstIsOne)
{
    ASSERT_FALSE(v_.empty());
    EXPECT_EQ(v_.front(), 1);
}

// 注意：不需要自己写 main()。链接 gtest_main 库即可（见 Makefile）。