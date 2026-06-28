#include <unigui/core/error.h>
#include <unigui/unigui.h>

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
    unigui::Result<int> r = unigui::Err(unigui::ErrorCode::BackendInitFailed);
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), unigui::ErrorCode::BackendInitFailed);
}

TEST(Result, SuccessMoveSemantics) {
    unigui::Result<std::string> r(std::string("hello"));
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(r.value(), "hello");
}

// std::expected migration (4.0): value() on the error path now throws rather than
// being undefined behaviour.
TEST(Result, ValueOnErrorThrows) {
    unigui::Result<int> r = unigui::Err(unigui::ErrorCode::InvalidArgument);
    EXPECT_THROW((void) r.value(), std::bad_expected_access<unigui::ErrorCode>);
}

// …and the monadic surface (transform / and_then / value_or) is now available.
TEST(Result, MonadicChaining) {
    unigui::Result<int> r = 2;

    const auto doubled = r.transform([](int v) { return v * 10; });
    ASSERT_TRUE(doubled.has_value());
    EXPECT_EQ(doubled.value(), 20);

    const auto chained = r.and_then([](int v) -> unigui::Result<int> {
        return v > 0 ? unigui::Result<int>(v + 1) : unigui::Err(unigui::ErrorCode::InvalidArgument);
    });
    EXPECT_EQ(chained.value(), 3);

    unigui::Result<int> bad = unigui::Err(unigui::ErrorCode::RenderFailed);
    EXPECT_EQ(bad.value_or(-1), -1);
}
