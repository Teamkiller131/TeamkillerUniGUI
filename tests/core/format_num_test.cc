#include <unigui/core/format_num.h>

#include <gtest/gtest.h>

using namespace unigui::format;

TEST(FormatNum, Thousands) {
    EXPECT_EQ(Thousands(0), "0");
    EXPECT_EQ(Thousands(999), "999");
    EXPECT_EQ(Thousands(1000), "1,000");
    EXPECT_EQ(Thousands(1234567), "1,234,567");
    EXPECT_EQ(Thousands(-1234567), "-1,234,567");
    EXPECT_EQ(Thousands(1234567, '.'), "1.234.567");
}

TEST(FormatNum, ThousandsHandlesInt64Min) {
    // Must not overflow when negating the most-negative value.
    EXPECT_EQ(Thousands(-9223372036854775807LL - 1), "-9,223,372,036,854,775,808");
}

TEST(FormatNum, Fixed) {
    EXPECT_EQ(Fixed(1234567.891, 2), "1,234,567.89");
    EXPECT_EQ(Fixed(1234567.0, 0), "1,234,567");
    EXPECT_EQ(Fixed(-1234.5, 2), "-1,234.50");
    EXPECT_EQ(Fixed(0.0, 2), "0.00");
}

TEST(FormatNum, FixedNoNegativeZero) {
    EXPECT_EQ(Fixed(-0.001, 2), "0.00");
    EXPECT_EQ(Fixed(-0.0, 2), "0.00");
}

TEST(FormatNum, Currency) {
    EXPECT_EQ(Currency(1234.5, "$"), "$1,234.50");
    EXPECT_EQ(Currency(-1234.5, "$"), "-$1,234.50");
    EXPECT_EQ(Currency(50, "€", 0), "€50");
    EXPECT_EQ(Currency(0.0, "$"), "$0.00");
}

TEST(FormatNum, Percent) {
    EXPECT_EQ(Percent(0.0425), "4.25%");
    EXPECT_EQ(Percent(4.25, 2, false), "4.25%");
    EXPECT_EQ(Percent(-0.10, 1), "-10.0%");
}

TEST(FormatNum, SignedDelta) {
    EXPECT_EQ(SignedDelta(1.5), "+1.50");
    EXPECT_EQ(SignedDelta(-1.5), "-1.50");
    EXPECT_EQ(SignedDelta(0.0), "+0.00");
    EXPECT_EQ(SignedDelta(1.5, 2, false), "1.50");
}

TEST(FormatNum, SignClassification) {
    EXPECT_EQ(Sign(0.5), Direction::Up);
    EXPECT_EQ(Sign(-0.5), Direction::Down);
    EXPECT_EQ(Sign(0.0), Direction::Flat);
    // Dead-band keeps small noise flat.
    EXPECT_EQ(Sign(0.005, 0.01), Direction::Flat);
}

TEST(FormatNum, TickAlign) {
    EXPECT_NEAR(TickAlign(100.123, 0.05), 100.10, 1e-9);
    EXPECT_NEAR(TickAlign(100.13, 0.05), 100.15, 1e-9);
    EXPECT_NEAR(TickAlign(100.123, 0.0), 100.123, 1e-9); // no-op for invalid tick
}

TEST(FormatNum, Latency) {
    EXPECT_EQ(Latency(0.0), "0\xC2\xB5s");        // 0µs
    EXPECT_EQ(Latency(850.0), "850\xC2\xB5s");    // 850µs
    EXPECT_EQ(Latency(1500.0), "1.50ms");          // → ms
    EXPECT_EQ(Latency(12300.0), "12.30ms");
    EXPECT_EQ(Latency(2500000.0), "2.50s");        // → s
    EXPECT_EQ(Latency(-5.0), "0\xC2\xB5s");        // clamps negatives
}
