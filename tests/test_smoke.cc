#include <unigui/unigui.h>
#include <gtest/gtest.h>

TEST(BuildSystemWorks, VersionMajorIsThree) {
    EXPECT_EQ(UNIGUI_VERSION_MAJOR, 3);
}

TEST(BuildSystemWorks, VersionMinorIsFive) {
    EXPECT_EQ(UNIGUI_VERSION_MINOR, 5);
}

TEST(BuildSystemWorks, VersionPatchIsZero) {
    EXPECT_EQ(UNIGUI_VERSION_PATCH, 0);
}
