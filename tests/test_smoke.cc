#include <unigui/unigui.h>

#include <gtest/gtest.h>

TEST(BuildSystemWorks, VersionMajorIsPositive) {
    // Don't pin an exact major — that went stale on the 3.x→4.0 bump. version_test.cc
    // covers component consistency; this is just a smoke floor.
    EXPECT_GE(UNIGUI_VERSION_MAJOR, 1);
}

TEST(BuildSystemWorks, VersionAtLeastReleaseBaseline) {
    // Track the released baseline rather than pinning an exact minor/patch (those
    // went stale on the 3.5→3.6 minor and 3.8.0→3.8.1 patch bumps). version_test.cc
    // covers component consistency; this is just a smoke floor.
    EXPECT_TRUE(UNIGUI_VERSION_AT_LEAST(4, 0, 0));
    EXPECT_GE(UNIGUI_VERSION_PATCH, 0);
}
