#include <unigui/core/context.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>

TEST(CreateContext, ReturnsValidPointer) {
    ASSERT_TRUE(unigui::CreateContext());
    EXPECT_NE(unigui::GetContext(), nullptr);
    unigui::DestroyContext();
}

TEST(DoubleCreate, ReturnsSameContext) {
    ASSERT_TRUE(unigui::CreateContext());
    auto* ctx1 = unigui::GetContext();
    ASSERT_TRUE(unigui::CreateContext());
    auto* ctx2 = unigui::GetContext();
    EXPECT_EQ(ctx1, ctx2);
    unigui::DestroyContext();
}

TEST(DestroyThenCreate, ReturnsNewContext) {
    ASSERT_TRUE(unigui::CreateContext());
    unigui::DestroyContext();
    ASSERT_TRUE(unigui::CreateContext());
    EXPECT_NE(unigui::GetContext(), nullptr);
    unigui::DestroyContext();
}

TEST(GetContext, WithoutCreateReturnsNull) {
    EXPECT_EQ(unigui::GetContext(), nullptr);
}
