#include <unigui/network/network.h>

#include <gtest/gtest.h>

using namespace unigui::network;

// SplitUrl is pure (no live HTTP), so it runs in headless CI regardless of network.

TEST(UrlParseTest, Https_WithPath) {
    auto p = SplitUrl("https://example.com/path/to");
    EXPECT_TRUE(p.https);
    EXPECT_EQ(p.host, "example.com");
    EXPECT_EQ(p.path, "/path/to");
}

TEST(UrlParseTest, Http_NoPath_DefaultsToSlash) {
    auto p = SplitUrl("http://example.com");
    EXPECT_FALSE(p.https);
    EXPECT_EQ(p.host, "example.com");
    EXPECT_EQ(p.path, "/");
}

TEST(UrlParseTest, BareHost_NoScheme) {
    auto p = SplitUrl("example.com/x");
    EXPECT_FALSE(p.https);
    EXPECT_EQ(p.host, "example.com");
    EXPECT_EQ(p.path, "/x");
}

TEST(UrlParseTest, HostWithPort) {
    auto p = SplitUrl("http://localhost:8080/api/v1");
    EXPECT_EQ(p.host, "localhost:8080");
    EXPECT_EQ(p.path, "/api/v1");
}

TEST(UrlParseTest, TrailingSlash) {
    auto p = SplitUrl("https://example.com/");
    EXPECT_TRUE(p.https);
    EXPECT_EQ(p.host, "example.com");
    EXPECT_EQ(p.path, "/");
}

TEST(UrlParseTest, Empty_IsSafe) {
    auto p = SplitUrl("");
    EXPECT_FALSE(p.https);
    EXPECT_EQ(p.host, "");
    EXPECT_EQ(p.path, "/");
}
