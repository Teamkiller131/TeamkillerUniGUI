#include <unigui/unigui.h>
#include <gtest/gtest.h>

TEST(BuildSystemWorks, VersionMajorIsZero) {
    EXPECT_EQ(UNIGUI_VERSION_MAJOR, 0);
}

TEST(BuildSystemWorks, VersionMinorIsOne) {
    EXPECT_EQ(UNIGUI_VERSION_MINOR, 1);
}

TEST(BuildSystemWorks, VersionPatchIsZero) {
    EXPECT_EQ(UNIGUI_VERSION_PATCH, 0);
}
