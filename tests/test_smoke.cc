#include <unigui/unigui.h>

#include <gtest/gtest.h>

TEST(BuildSystemWorks, VersionMajorIsThree) {
    EXPECT_EQ(UNIGUI_VERSION_MAJOR, 3);
}

TEST(BuildSystemWorks, VersionAtLeastReleaseBaseline) {
    // Track the released baseline rather than pinning an exact minor (which went
    // stale on the 3.5 → 3.6 bump). version_test.cc covers component consistency.
    EXPECT_TRUE(UNIGUI_VERSION_AT_LEAST(3, 6, 0));
}

TEST(BuildSystemWorks, VersionPatchIsZero) {
    EXPECT_EQ(UNIGUI_VERSION_PATCH, 0);
}
