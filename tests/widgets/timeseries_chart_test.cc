#include <unigui/core/session_axis.h>
#include <unigui/widgets/timeseries_chart.h>

#include <gtest/gtest.h>
#include <vector>

using namespace unigui;

// These exercise only the data path (AddSeries / SetSeriesData / decimation),
// which touch no ImGui/ImPlot state — so no GL context or frame is needed.

TEST(TimeSeriesChartTest, SetSeriesData_StoresPoints) {
    TimeSeriesChart c("ts");
    c.SetSlidingWindow(0); // no trim
    const int id = c.AddSeries({});
    std::vector<double> xs, ys;
    for (int i = 0; i < 50; ++i) {
        xs.push_back(i);
        ys.push_back(i * 0.5);
    }
    c.SetSeriesData(id, xs, ys);
    EXPECT_EQ(c.GetSeriesPointCount(id), 50);
    EXPECT_EQ(c.GetSeriesPointCount(9999), -1); // unknown series
}

TEST(TimeSeriesChartTest, MaxRenderPoints_DecimatesLargeSeries) {
    TimeSeriesChart c("ts2");
    c.SetSlidingWindow(0);
    c.SetMaxRenderPoints(200);
    const int id = c.AddSeries({});
    std::vector<double> xs(10000), ys(10000);
    for (int i = 0; i < 10000; ++i) {
        xs[i] = i;
        ys[i] = (i % 50) - 25; // sawtooth
    }
    c.SetSeriesData(id, xs, ys);
    // Decimated down to ~the cap (LTTB returns exactly the threshold).
    EXPECT_LE(c.GetSeriesPointCount(id), 200);
    EXPECT_GT(c.GetSeriesPointCount(id), 0);
}

TEST(TimeSeriesChartTest, MaxRenderPoints_Zero_KeepsAll) {
    TimeSeriesChart c("ts3");
    c.SetSlidingWindow(0);
    c.SetMaxRenderPoints(0); // disabled
    const int id = c.AddSeries({});
    std::vector<double> xs(2000), ys(2000);
    for (int i = 0; i < 2000; ++i) {
        xs[i] = i;
        ys[i] = i;
    }
    c.SetSeriesData(id, xs, ys);
    EXPECT_EQ(c.GetSeriesPointCount(id), 2000);
}

TEST(TimeSeriesChartTest, UpsertPoint_UpdatesInPlaceElseAppends) {
    TimeSeriesChart c("ts_up");
    c.SetSlidingWindow(100); // ample window (0 would mean "keep none" on the append path)
    const int id = c.AddSeries({});

    // First upsert at t=100 appends.
    c.UpsertPoint(id, 1.0f, 100.0);
    EXPECT_EQ(c.GetSeriesPointCount(id), 1);

    // Re-upsert at the same timestamp updates in place — no growth.
    c.UpsertPoint(id, 2.0f, 100.0);
    c.UpsertPoint(id, 3.0f, 100.0);
    EXPECT_EQ(c.GetSeriesPointCount(id), 1);

    // A new timestamp appends.
    c.UpsertPoint(id, 5.0f, 101.0);
    EXPECT_EQ(c.GetSeriesPointCount(id), 2);

    // Unknown series id is a no-op (no crash).
    c.UpsertPoint(9999, 1.0f, 1.0);
}

TEST(TimeSeriesChartTest, UpsertPoint_RespectsSlidingWindowOnAppend) {
    TimeSeriesChart c("ts_up2");
    c.SetSlidingWindow(3);
    const int id = c.AddSeries({});
    for (int i = 0; i < 10; ++i)
        c.UpsertPoint(id, static_cast<float>(i), static_cast<double>(i));
    EXPECT_EQ(c.GetSeriesPointCount(id), 3); // trimmed to window
}

TEST(TimeSeriesChartTest, SetSessionAxis_DoesNotCrashAndAcceptsFormatter) {
    // The formatter is pure (delegates to SessionAxis::FormatAxis); installing it
    // must not require a GL context. We can at least confirm the wiring compiles
    // and runs, and that SessionAxis itself maps an axis position to "HH:MM".
    TimeSeriesChart c("ts_sa");
    const int id = c.AddSeries({});
    SessionAxis axis = SessionAxis::AShareFutures();
    EXPECT_NO_THROW(c.SetSessionAxis(axis));
    // Plot session-axis coordinates: 0 → session open 09:30.
    c.AppendSample(id, axis.ToAxis(9 * 3600 + 30 * 60), 1.0f);
    EXPECT_EQ(axis.FormatAxis(0.0), "09:30");
}

// ── Y 轴边距算术(2026-07-22 需求:[min·0.95, max·1.05])────────────────────────
// 纯算术,不需要 ImPlot 帧。四种形态各钉一条:正区间按用户原始口径;负区间与跨零
// 必须【向外】扩(naive 公式会把负数据夹在轴外);全平恰为 0 时兜底 [-1,1]。
TEST(TimeSeriesChartTest, PadRange_PositiveData_MatchesUserSpec) {
    auto [lo, hi] = TimeSeriesChart::PadRange(100.0, 200.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, 95.0);    // min × 0.95
    EXPECT_DOUBLE_EQ(hi, 210.0);   // max × 1.05
}
TEST(TimeSeriesChartTest, PadRange_NegativeData_PadsOutward) {
    auto [lo, hi] = TimeSeriesChart::PadRange(-200.0, -100.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, -210.0);  // 向下扩,naive 的 -190 会切掉数据
    EXPECT_DOUBLE_EQ(hi, -95.0);
}
TEST(TimeSeriesChartTest, PadRange_CrossZero_PadsBothSides) {
    auto [lo, hi] = TimeSeriesChart::PadRange(-100.0, 200.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, -105.0);
    EXPECT_DOUBLE_EQ(hi, 210.0);
}
TEST(TimeSeriesChartTest, PadRange_FlatAtZero_NeverDegenerate) {
    auto [lo, hi] = TimeSeriesChart::PadRange(0.0, 0.0, 0.05);
    EXPECT_LT(lo, hi);             // 轴绝不允许塌成零高度
    EXPECT_DOUBLE_EQ(lo, -1.0);
    EXPECT_DOUBLE_EQ(hi, 1.0);
}
TEST(TimeSeriesChartTest, PadRange_FlatNonZero_StillHasSpan) {
    auto [lo, hi] = TimeSeriesChart::PadRange(100.0, 100.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, 95.0);    // 平但非零:乘法边距天然给出高度
    EXPECT_DOUBLE_EQ(hi, 105.0);
}
