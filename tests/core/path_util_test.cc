#include <unigui/core/path_util.h>

#include <filesystem>
#include <gtest/gtest.h>
#include <string>

using unigui::PathFromUtf8;

namespace {
// Round-trip a path's UTF-8 view back to a std::string for comparison.
std::string ToUtf8(const std::filesystem::path& p) {
    const auto u8 = p.u8string();
    return std::string(u8.begin(), u8.end());
}
} // namespace

TEST(PathFromUtf8, AsciiPassthrough) {
    EXPECT_EQ(ToUtf8(PathFromUtf8("plain/dir/app.ini")), "plain/dir/app.ini");
}

TEST(PathFromUtf8, Utf8BytesPreserved) {
    // "配置" in UTF-8 = E9 85 8D E7 BD AE. On Windows a plain path(std::string)
    // would decode these bytes via the ANSI code page and mangle them; PathFromUtf8
    // must preserve the exact UTF-8 byte sequence (decoding to the right Unicode).
    const std::string utf8 = "\xE9\x85\x8D\xE7\xBD\xAE";
    EXPECT_EQ(ToUtf8(PathFromUtf8(utf8)), utf8);
}

TEST(PathFromUtf8, NonAsciiFilenameComponentIntact) {
    const auto p = PathFromUtf8("dir/\xE9\x85\x8D\xE7\xBD\xAE.ini");
    EXPECT_EQ(ToUtf8(p.filename()), "\xE9\x85\x8D\xE7\xBD\xAE.ini");
}

TEST(PathFromUtf8, EmptyIsEmpty) {
    EXPECT_TRUE(PathFromUtf8("").empty());
}
