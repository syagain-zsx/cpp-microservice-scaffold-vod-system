#include <gtest/gtest.h>
#include <JianLiuScaffold/hls.h>
#include <JianLiuScaffold/util.h>

#include <filesystem>

static const char *kM3u8 =
    "#EXTM3U\n"
    "#EXT-X-VERSION:6\n"
    "#EXTINF:10.000000,\n"
    "seg0.ts\n"
    "#EXTINF:2.000000,\n"
    "seg1.ts\n"
    "#EXT-X-ENDLIST\n";

TEST(M3U8, Parse) {
    auto path = std::filesystem::temp_directory_path() / "jianliu_test.m3u8";
    ASSERT_TRUE(JianLiuUtil::FUTIL::write(path.string(), kM3u8));

    JianLiuHls::M3U8Info info(path.string());
    ASSERT_TRUE(info.parse());
    EXPECT_EQ(info.pieces().size(), 2u);
    EXPECT_EQ(info.pieces()[0].second, "seg0.ts");
    EXPECT_EQ(info.pieces()[1].second, "seg1.ts");
    EXPECT_GE(info.headers().size(), 2u);

    std::filesystem::remove(path);
}

TEST(M3U8, WriteRoundTrip) {
    auto path = std::filesystem::temp_directory_path() / "jianliu_test_write.m3u8";

    JianLiuHls::M3U8Info info(path.string());
    info.headers().push_back("#EXTM3U");
    info.headers().push_back("#EXT-X-VERSION:6");
    info.pieces().push_back({"#EXTINF:5.0,", "a.ts"});
    ASSERT_TRUE(info.write());

    std::string body;
    ASSERT_TRUE(JianLiuUtil::FUTIL::read(path.string(), body));
    EXPECT_NE(body.find("#EXT-X-ENDLIST"), std::string::npos);
    EXPECT_NE(body.find("a.ts"), std::string::npos);

    JianLiuHls::M3U8Info back(path.string());
    ASSERT_TRUE(back.parse());
    EXPECT_EQ(back.pieces().size(), 1u);

    std::filesystem::remove(path);
}
