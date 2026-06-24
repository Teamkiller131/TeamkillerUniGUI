#include <unigui/core/decimate.h>

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace unigui;

TEST(DecimateTest, Lttb_KeepsAllWhenUnderThreshold) {
    std::vector<double> xs{0, 1, 2}, ys{0, 1, 0};
    auto idx = LttbIndices(xs.data(), ys.data(), 3, 100);
    ASSERT_EQ(idx.size(), 3u);
    EXPECT_EQ(idx[0], 0u);
    EXPECT_EQ(idx[2], 2u);
}

TEST(DecimateTest, Lttb_ReducesToThreshold) {
    std::vector<double> xs(1000), ys(1000);
    for (std::size_t i = 0; i < 1000; ++i) {
        xs[i] = static_cast<double>(i);
        ys[i] = std::sin(i * 0.05);
    }
    auto idx = LttbIndices(xs.data(), ys.data(), 1000, 100);
    EXPECT_EQ(idx.size(), 100u);
    // First and last always kept.
    EXPECT_EQ(idx.front(), 0u);
    EXPECT_EQ(idx.back(), 999u);
    // Strictly ascending.
    for (std::size_t i = 1; i < idx.size(); ++i)
        EXPECT_LT(idx[i - 1], idx[i]);
}

TEST(DecimateTest, Lttb_PreservesAPeak) {
    // Flat line with one tall spike — LTTB must keep a point at/near the spike.
    std::vector<double> xs(500), ys(500, 0.0);
    for (std::size_t i = 0; i < 500; ++i)
        xs[i] = static_cast<double>(i);
    ys[250] = 100.0; // spike
    auto idx = LttbIndices(xs.data(), ys.data(), 500, 20);
    bool keptSpike = false;
    for (std::size_t i : idx)
        if (ys[i] > 50.0)
            keptSpike = true;
    EXPECT_TRUE(keptSpike);
}

TEST(DecimateTest, Decimate_WritesShorterSeries) {
    std::vector<double> xs(2000), ys(2000);
    for (std::size_t i = 0; i < 2000; ++i) {
        xs[i] = static_cast<double>(i);
        ys[i] = static_cast<double>(i % 7);
    }
    std::vector<double> ox, oy;
    Decimate(xs, ys, 250, ox, oy);
    EXPECT_EQ(ox.size(), 250u);
    EXPECT_EQ(oy.size(), 250u);
    EXPECT_DOUBLE_EQ(ox.front(), 0.0);
    EXPECT_DOUBLE_EQ(ox.back(), 1999.0);
}

TEST(DecimateTest, MinMaxBuckets_PreservesExtremes) {
    std::vector<double> ys(1000, 0.0);
    ys[123] = -99.0; // trough
    ys[876] = 99.0;  // peak
    auto idx = MinMaxBuckets(ys.data(), 1000, 50);
    bool keptMin = false, keptMax = false;
    for (std::size_t i : idx) {
        if (ys[i] <= -99.0)
            keptMin = true;
        if (ys[i] >= 99.0)
            keptMax = true;
    }
    EXPECT_TRUE(keptMin);
    EXPECT_TRUE(keptMax);
    // Ascending order.
    for (std::size_t i = 1; i < idx.size(); ++i)
        EXPECT_LT(idx[i - 1], idx[i]);
}

TEST(DecimateTest, Empty_IsSafe) {
    EXPECT_TRUE(LttbIndices(nullptr, nullptr, 0, 10).empty());
    EXPECT_TRUE(MinMaxBuckets(nullptr, 0, 10).empty());
}
