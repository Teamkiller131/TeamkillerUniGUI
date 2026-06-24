#include <unigui/unigui.h>

#include <gtest/gtest.h>

TEST(BuildSystemWorks, VersionMajorIsThree) {
    EXPECT_EQ(UNIGUI_VERSION_MAJOR, 3);
}

TEST(BuildSystemWorks, VersionAtLeastReleaseBaseline) {
    // Track the released baseline rather than pinning an exact minor/patch (those
    // went stale on the 3.5→3.6 minor and 3.8.0→3.8.1 patch bumps). version_test.cc
    // covers component consistency; this is just a smoke floor.
    EXPECT_TRUE(UNIGUI_VERSION_AT_LEAST(3, 8, 0));
    EXPECT_GE(UNIGUI_VERSION_PATCH, 0);
}
