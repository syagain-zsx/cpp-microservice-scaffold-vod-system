#include <gtest/gtest.h>
#include <JianLiuScaffold/util.h>

#include <filesystem>

// JSON 序列化 / 反序列化
TEST(Json, SerializeAndUnserialize) {
    Json::Value root;
    root["name"] = "jianliu";
    root["count"] = 3;

    auto text = JianLiuUtil::JSON::serialize(root);
    ASSERT_TRUE(text.has_value());

    auto back = JianLiuUtil::JSON::unserialize(*text);
    ASSERT_TRUE(back.has_value());
    EXPECT_EQ((*back)["name"].asString(), "jianliu");
    EXPECT_EQ((*back)["count"].asInt(), 3);
}

TEST(Json, InvalidInputReturnsNullopt) {
    EXPECT_FALSE(JianLiuUtil::JSON::unserialize("{not json").has_value());
}

// 字符串切分（连续分隔符会被跳过）
TEST(Str, Split) {
    std::vector<std::string> out;
    size_t n = JianLiuUtil::STR::split("a,b,,c", ",", out);
    EXPECT_EQ(n, 3u);
    EXPECT_EQ(out[0], "a");
    EXPECT_EQ(out[1], "b");
    EXPECT_EQ(out[2], "c");
}

// 随机串：len > 6 时会追加 4 位自增编号
TEST(Random, CodeLength) {
    auto s = JianLiuUtil::Random::code(16);
    EXPECT_EQ(s.size(), 20u);
    auto n = JianLiuUtil::Random::number(5, 10);
    EXPECT_GE(n, 5u);
    EXPECT_LE(n, 10u);
}

// 文件读写
TEST(File, ReadWrite) {
    auto path = std::filesystem::temp_directory_path() / "jianliu_futil_test.txt";
    ASSERT_TRUE(JianLiuUtil::FUTIL::write(path.string(), "hello 笺流"));

    std::string body;
    ASSERT_TRUE(JianLiuUtil::FUTIL::read(path.string(), body));
    EXPECT_EQ(body, "hello 笺流");

    std::filesystem::remove(path);
}
