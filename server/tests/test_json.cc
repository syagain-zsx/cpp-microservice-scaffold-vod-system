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

// 密码哈希：加盐 SHA-256（64 位十六进制），兼容历史明文
TEST(Password, HashAndVerify) {
    auto h = JianLiuUtil::Password::hash("secret123");
    EXPECT_EQ(h.size(), 64u);
    EXPECT_TRUE(JianLiuUtil::Password::verify(h, "secret123"));
    EXPECT_FALSE(JianLiuUtil::Password::verify(h, "secret124"));
    // 两次 hash 盐不同，结果不同
    EXPECT_NE(JianLiuUtil::Password::hash("secret123"), h);
    // 历史明文密码兼容
    EXPECT_TRUE(JianLiuUtil::Password::verify("plain123", "plain123"));
    EXPECT_FALSE(JianLiuUtil::Password::verify("plain123", "plain124"));
    // 非十六进制的 64 位字符串按明文比较
    std::string not_hex(64, 'z');
    EXPECT_TRUE(JianLiuUtil::Password::verify(not_hex, not_hex));
}
