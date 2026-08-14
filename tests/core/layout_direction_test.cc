// Layout-direction state tests (the RTL mirroring switch).
#include <unigui/core/layout_direction.h>

#include <gtest/gtest.h>

TEST(LayoutDirection, DefaultsToLeftToRight) {
    unigui::SetLayoutDirection(unigui::LayoutDirection::LeftToRight);
    EXPECT_EQ(unigui::GetLayoutDirection(), unigui::LayoutDirection::LeftToRight);
    EXPECT_FALSE(unigui::IsRightToLeft());
}

TEST(LayoutDirection, SetRoundTripsAndPredicateFollows) {
    unigui::SetLayoutDirection(unigui::LayoutDirection::RightToLeft);
    EXPECT_EQ(unigui::GetLayoutDirection(), unigui::LayoutDirection::RightToLeft);
    EXPECT_TRUE(unigui::IsRightToLeft());

    unigui::SetLayoutDirection(unigui::LayoutDirection::LeftToRight);
    EXPECT_EQ(unigui::GetLayoutDirection(), unigui::LayoutDirection::LeftToRight);
    EXPECT_FALSE(unigui::IsRightToLeft());
}
