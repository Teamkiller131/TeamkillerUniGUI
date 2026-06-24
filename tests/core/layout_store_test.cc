#include <unigui/core/layout_store.h>

#include <gtest/gtest.h>

#include <filesystem>

using namespace unigui;

TEST(LayoutStoreTest, SetGetHasRemove) {
    LayoutStore s;
    EXPECT_FALSE(s.Has("a"));
    EXPECT_EQ(s.Get("a"), "");
    s.Set("a", "0.3,0.7");
    s.Set("theme", "Dark");
    EXPECT_TRUE(s.Has("a"));
    EXPECT_EQ(s.Get("a"), "0.3,0.7");
    EXPECT_EQ(s.Size(), 2u);
    s.Remove("a");
    EXPECT_FALSE(s.Has("a"));
    EXPECT_EQ(s.Size(), 1u);
}

TEST(LayoutStoreTest, StripsNewlinesInValue) {
    LayoutStore s;
    s.Set("k", "line1\nline2\r\n");
    EXPECT_EQ(s.Get("k").find('\n'), std::string::npos);
    EXPECT_EQ(s.Get("k").find('\r'), std::string::npos);
}

TEST(LayoutStoreTest, SaveLoadRoundTrip) {
    const auto path =
        (std::filesystem::temp_directory_path() / "unigui_layout_test.ini").string();
    {
        LayoutStore s;
        s.Set("main_split", "0.2800,0.4400,0.2800");
        s.Set("locale", "zh_CN");
        s.Set("content_scale", "1.50");
        ASSERT_TRUE(s.Save(path));
    }
    LayoutStore s2;
    ASSERT_TRUE(s2.Load(path));
    EXPECT_EQ(s2.Size(), 3u);
    EXPECT_EQ(s2.Get("main_split"), "0.2800,0.4400,0.2800");
    EXPECT_EQ(s2.Get("locale"), "zh_CN");
    EXPECT_EQ(s2.Get("content_scale"), "1.50");
    std::filesystem::remove(path);
}

TEST(LayoutStoreTest, Load_MissingFileIsBenign) {
    LayoutStore s;
    EXPECT_FALSE(s.Load("D:/no/such/unigui_layout_xyz.ini"));
}

TEST(LayoutStoreTest, Load_SkipsMalformedLines) {
    const auto path =
        (std::filesystem::temp_directory_path() / "unigui_layout_bad.ini").string();
    {
        std::ofstream f(path, std::ios::binary);
        f << "good=1\n";
        f << "no-equals-here\n"; // skipped
        f << "=novalkey\n";      // empty key → skipped
        f << "k2=a=b=c\n";       // value may contain '='
    }
    LayoutStore s;
    ASSERT_TRUE(s.Load(path));
    EXPECT_EQ(s.Size(), 2u);
    EXPECT_EQ(s.Get("good"), "1");
    EXPECT_EQ(s.Get("k2"), "a=b=c");
    std::filesystem::remove(path);
}
