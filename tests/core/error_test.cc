#include <unigui/unigui.h>
#include <unigui/core/error.h>
#include <gtest/gtest.h>

TEST(ErrorCode, ToMessageReturnsNonEmpty) {
    EXPECT_FALSE(unigui::ErrorMessage(unigui::ErrorCode::None).empty());
    EXPECT_FALSE(unigui::ErrorMessage(unigui::ErrorCode::BackendInitFailed).empty());
    EXPECT_FALSE(unigui::ErrorMessage(unigui::ErrorCode::InvalidArgument).empty());
}

TEST(Result, SuccessHoldsValue) {
    unigui::Result<int> r(42);
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(r.value(), 42);
}

TEST(Result, ErrorHoldsErrorCode) {
    unigui::Result<int> r(unigui::ErrorCode::BackendInitFailed);
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), unigui::ErrorCode::BackendInitFailed);
}

TEST(Result, SuccessMoveSemantics) {
    unigui::Result<std::string> r(std::string("hello"));
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(r.value(), "hello");
}
