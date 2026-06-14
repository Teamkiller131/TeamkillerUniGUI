#include <unigui/trading/ohlc_series.h>

#include <gtest/gtest.h>

using namespace unigui::trading;

TEST(OhlcSeries, AggregatesTicksIntoBars) {
    OhlcSeries s(60.0);
    s.AddTick(0, 10, 1);
    s.AddTick(10, 12, 2); // high
    s.AddTick(30, 9, 1);  // low
    s.AddTick(59, 11, 1); // close, still bar [0,60)
    ASSERT_EQ(s.Size(), 1u);
    const Bar& b = s.At(0);
    EXPECT_EQ(b.time, 0.0);
    EXPECT_EQ(b.open, 10.0);
    EXPECT_EQ(b.high, 12.0);
    EXPECT_EQ(b.low, 9.0);
    EXPECT_EQ(b.close, 11.0);
    EXPECT_EQ(b.volume, 5);
    EXPECT_TRUE(b.Bullish());
}

TEST(OhlcSeries, OpensNewBarOnIntervalCross) {
    OhlcSeries s(60.0);
    s.AddTick(10, 10);
    s.AddTick(65, 11); // bucket [60,120)
    s.AddTick(80, 13);
    ASSERT_EQ(s.Size(), 2u);
    EXPECT_EQ(s.At(1).time, 60.0);
    EXPECT_EQ(s.At(1).open, 11.0);
    EXPECT_EQ(s.At(1).high, 13.0);
}

TEST(OhlcSeries, IgnoresLateTicks) {
    OhlcSeries s(60.0);
    s.AddTick(10, 10);
    s.AddTick(70, 11);
    s.AddTick(5, 999); // older than current bucket → ignored
    EXPECT_EQ(s.Size(), 2u);
    EXPECT_EQ(s.At(1).high, 11.0);
}

TEST(OhlcSeries, RollingWindowTrims) {
    OhlcSeries s(60.0, 2);
    s.AddTick(0, 1);
    s.AddTick(70, 2);
    s.AddTick(130, 3);
    ASSERT_EQ(s.Size(), 2u);
    EXPECT_EQ(s.At(0).open, 2.0); // oldest dropped
    EXPECT_EQ(s.Back().open, 3.0);
}

TEST(OhlcSeries, ColumnExtractors) {
    OhlcSeries s(60.0);
    s.AddTick(0, 10);
    s.AddTick(70, 20);
    auto times = s.Times();
    auto opens = s.Opens();
    auto closes = s.Closes();
    ASSERT_EQ(opens.size(), 2u);
    EXPECT_EQ(times[1], 60.0);
    EXPECT_EQ(opens[0], 10.0);
    EXPECT_EQ(closes[1], 20.0);
}

TEST(OhlcSeries, AddBarBackfill) {
    OhlcSeries s(60.0);
    s.AddBar({0.0, 1, 2, 0.5, 1.5, 100});
    ASSERT_EQ(s.Size(), 1u);
    EXPECT_EQ(s.Back().high, 2.0);
    EXPECT_EQ(s.Back().volume, 100);
}
