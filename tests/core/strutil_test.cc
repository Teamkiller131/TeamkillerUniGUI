#include <unigui/core/strutil.h>

#include <gtest/gtest.h>

TEST(StrUtilTest, ToIntOr_Valid) {
    EXPECT_EQ(unigui::ToIntOr("42"), 42);
    EXPECT_EQ(unigui::ToIntOr("-7"), -7);
    EXPECT_EQ(unigui::ToIntOr("0"), 0);
}

TEST(StrUtilTest, ToIntOr_Empty_ReturnsDefault) {
    EXPECT_EQ(unigui::ToIntOr("", 5), 5);
}

TEST(StrUtilTest, ToIntOr_Garbage_ReturnsDefault) {
    EXPECT_EQ(unigui::ToIntOr("abc", 3), 3);
}

TEST(StrUtilTest, ToIntOr_Whitespace_ReturnsDefault) {
    EXPECT_EQ(unigui::ToIntOr("   ", 9), 9);
}

TEST(StrUtilTest, ToIntOr_TrailingText_ParsesLeading) {
    EXPECT_EQ(unigui::ToIntOr("42abc"), 42);
}

TEST(StrUtilTest, ToIntOr_LargeNumber_Parses) {
    EXPECT_EQ(unigui::ToIntOr("999999"), 999999);
}

TEST(StrUtilTest, ToFloatOr_Valid) {
    EXPECT_FLOAT_EQ(unigui::ToFloatOr("3.14"), 3.14f);
    EXPECT_FLOAT_EQ(unigui::ToFloatOr("-1.5"), -1.5f);
}

TEST(StrUtilTest, ToFloatOr_Empty_ReturnsDefault) {
    EXPECT_FLOAT_EQ(unigui::ToFloatOr("", 2.5f), 2.5f);
}

TEST(StrUtilTest, ToFloatOr_Garbage_ReturnsDefault) {
    EXPECT_FLOAT_EQ(unigui::ToFloatOr("xyz", 1.0f), 1.0f);
}

TEST(StrUtilTest, ToDoubleOr_Valid) {
    EXPECT_DOUBLE_EQ(unigui::ToDoubleOr("3.14159"), 3.14159);
}

TEST(StrUtilTest, ToDoubleOr_Empty_ReturnsDefault) {
    EXPECT_DOUBLE_EQ(unigui::ToDoubleOr("", 2.71), 2.71);
}

TEST(StrUtilTest, ToDoubleOr_Garbage_ReturnsDefault) {
    EXPECT_DOUBLE_EQ(unigui::ToDoubleOr("abc", 1.0), 1.0);
}

TEST(StrUtilTest, TrimInPlace_LeadingTrailing) {
    std::string s = "  hello  ";
    unigui::TrimInPlace(s);
    EXPECT_EQ(s, "hello");
}

TEST(StrUtilTest, TrimInPlace_Tabs) {
    std::string s = "\tfoo\t";
    unigui::TrimInPlace(s);
    EXPECT_EQ(s, "foo");
}

TEST(StrUtilTest, TrimInPlace_Empty) {
    std::string s = "";
    unigui::TrimInPlace(s);
    EXPECT_EQ(s, "");
}

TEST(StrUtilTest, TrimInPlace_WhitespaceOnly) {
    std::string s = "   ";
    unigui::TrimInPlace(s);
    EXPECT_EQ(s, "");
}

TEST(StrUtilTest, Trim_ReturnsCopy) {
    std::string original = "  bar  ";
    std::string trimmed = unigui::Trim(original);
    EXPECT_EQ(trimmed, "bar");
    EXPECT_EQ(original, "  bar  "); // original unchanged
}
