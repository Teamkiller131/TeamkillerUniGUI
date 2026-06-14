#include <unigui/core/api.h>
#include <unigui/core/version.h>

#include <gtest/gtest.h>
#include <string>

TEST(VersionTest, ComponentsArePositive) {
    EXPECT_GE(UNIGUI_VERSION_MAJOR, 0);
    EXPECT_GE(UNIGUI_VERSION_MINOR, 0);
    EXPECT_GE(UNIGUI_VERSION_PATCH, 0);
}

TEST(VersionTest, NumberEncodesComponents) {
    EXPECT_EQ(UNIGUI_VERSION_NUMBER, UNIGUI_MAKE_VERSION(UNIGUI_VERSION_MAJOR, UNIGUI_VERSION_MINOR,
                                                         UNIGUI_VERSION_PATCH));
}

TEST(VersionTest, MakeVersionOrdersCorrectly) {
    EXPECT_LT(UNIGUI_MAKE_VERSION(3, 4, 9), UNIGUI_MAKE_VERSION(3, 5, 0));
    EXPECT_LT(UNIGUI_MAKE_VERSION(2, 99, 99), UNIGUI_MAKE_VERSION(3, 0, 0));
    EXPECT_GT(UNIGUI_MAKE_VERSION(3, 5, 1), UNIGUI_MAKE_VERSION(3, 5, 0));
}

TEST(VersionTest, AtLeastMatchesCurrent) {
    EXPECT_TRUE(
        UNIGUI_VERSION_AT_LEAST(UNIGUI_VERSION_MAJOR, UNIGUI_VERSION_MINOR, UNIGUI_VERSION_PATCH));
    EXPECT_TRUE(UNIGUI_VERSION_AT_LEAST(0, 0, 0));
    EXPECT_FALSE(UNIGUI_VERSION_AT_LEAST(UNIGUI_VERSION_MAJOR + 1, 0, 0));
}

TEST(VersionTest, StringMatchesComponents) {
    const std::string expected = std::to_string(UNIGUI_VERSION_MAJOR) + "." +
                                 std::to_string(UNIGUI_VERSION_MINOR) + "." +
                                 std::to_string(UNIGUI_VERSION_PATCH);
    EXPECT_EQ(std::string(UNIGUI_VERSION_STRING), expected);
}

// The stability markers must expand to something that compiles at a declaration
// site. UNIGUI_EXPERIMENTAL / UNIGUI_INTERNAL are documentation no-ops;
// UNIGUI_DEPRECATED must accept a message and still allow the symbol to be used.
namespace {
UNIGUI_EXPERIMENTAL
int ExperimentalFn() {
    return 1;
}

UNIGUI_INTERNAL
int InternalFn() {
    return 2;
}
} // namespace

TEST(ApiMacrosTest, MarkersCompileAndCallable) {
    EXPECT_EQ(ExperimentalFn(), 1);
    EXPECT_EQ(InternalFn(), 2);
}
