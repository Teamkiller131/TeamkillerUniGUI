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
    EXPECT_EQ(p.host, "localhost"); // port is split off the host
    EXPECT_EQ(p.port, 8080);
    EXPECT_EQ(p.path, "/api/v1");
}

TEST(UrlParseTest, NoPort_DefaultsToZero) {
    auto p = SplitUrl("http://example.com/x");
    EXPECT_EQ(p.host, "example.com");
    EXPECT_EQ(p.port, 0); // 0 == "use the scheme default"
}

// fix: a malformed / out-of-range port must NOT throw (httplib's single-arg ctor would
// std::stoi it and crash). It is rejected, host stays intact, port falls back to 0.
TEST(UrlParseTest, OverflowPort_DoesNotThrow_FallsBack) {
    ParsedUrl p;
    EXPECT_NO_THROW({ p = SplitUrl("http://host:99999999999999/x"); });
    EXPECT_EQ(p.host, "host:99999999999999"); // not a valid port → left on the host verbatim
    EXPECT_EQ(p.port, 0);
    EXPECT_EQ(p.path, "/x");
}

TEST(UrlParseTest, PortOutOfRange_Rejected) {
    auto p = SplitUrl("http://host:70000/"); // > 65535
    EXPECT_EQ(p.host, "host:70000");
    EXPECT_EQ(p.port, 0);
}

TEST(UrlParseTest, Ipv6Literal_NotMistakenForPort) {
    auto p = SplitUrl("http://[::1]:8080/x");
    EXPECT_EQ(p.port, 8080);
    EXPECT_EQ(p.path, "/x");
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
