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

// ── Y 轴边距算术(2026-07-24 修正:按【数据跨度 span】外扩,不按【绝对值】)──────────
// 纯算术,不需要 ImPlot 帧。旧口径 [min·0.95, max·1.05] 对偏移数据(比价 ~7000 波动 ~60)
// 会按值的 5% 外扩(±350)压平信号;改为 ±r×span,与波动成比例。全平兜底 [±1] 防零高度。
TEST(TimeSeriesChartTest, PadRange_PositiveData_SpanRelative) {
    auto [lo, hi] = TimeSeriesChart::PadRange(100.0, 200.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, 95.0);    // 100 − 0.05×(200−100)
    EXPECT_DOUBLE_EQ(hi, 205.0);   // 200 + 0.05×100
}
TEST(TimeSeriesChartTest, PadRange_OffsetSmallSwing_StaysTight) {
    // 回归本次 AA 比价 bug:~6900 附近波动 60,旧口径会扩成 ~6555..7308(span≈753);
    // 新口径只扩 ±3 → 走势清晰可见,ImPlot 细刻度。
    auto [lo, hi] = TimeSeriesChart::PadRange(6900.0, 6960.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, 6897.0);
    EXPECT_DOUBLE_EQ(hi, 6963.0);
    EXPECT_LT(hi - lo, 70.0);      // 跨度受控(旧口径会 >750)
}
TEST(TimeSeriesChartTest, PadRange_NegativeData_PadsOutward) {
    auto [lo, hi] = TimeSeriesChart::PadRange(-200.0, -100.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, -205.0);  // 向外(下)扩 ±span×r
    EXPECT_DOUBLE_EQ(hi, -95.0);
}
TEST(TimeSeriesChartTest, PadRange_CrossZero_PadsBothSides) {
    auto [lo, hi] = TimeSeriesChart::PadRange(-100.0, 200.0, 0.05);
    EXPECT_DOUBLE_EQ(lo, -115.0);  // span=300 → pad=15,两侧对称外扩
    EXPECT_DOUBLE_EQ(hi, 215.0);
}
TEST(TimeSeriesChartTest, PadRange_FlatAtZero_NeverDegenerate) {
    auto [lo, hi] = TimeSeriesChart::PadRange(0.0, 0.0, 0.05);
    EXPECT_LT(lo, hi);             // 轴绝不允许塌成零高度
    EXPECT_DOUBLE_EQ(lo, -1.0);
    EXPECT_DOUBLE_EQ(hi, 1.0);
}
TEST(TimeSeriesChartTest, PadRange_FlatNonZero_StillHasSpan) {
    auto [lo, hi] = TimeSeriesChart::PadRange(100.0, 100.0, 0.05);
    EXPECT_LT(lo, hi);             // span=0 → ±1 兜底,不塌缩
    EXPECT_DOUBLE_EQ(lo, 99.0);
    EXPECT_DOUBLE_EQ(hi, 101.0);
}

// ── [YSPANLOCK-20260810] 固定纵轴 = 高度钉死,平移允许、缩放不允许 ─────────────
//
// 交易员原话:「固定就是固定,就不能缩放了,不固定的时候才允许缩放」。
// 上一版只把量程下发了一次就撒手,ImPlot 的滚轮缩放照样改高度 —— 于是「固定纵轴」
// 勾着,高度却能被一次误触改掉,而那个高度正是她用来横向比较波动幅度的尺子。
//
// 实现是「按结果纠偏」:平移只改中心不改跨度(不触发),缩放改了跨度就按当前中心拉回。
// 下面钉的就是这条纯函数。

TEST(TimeSeriesChartTest, RestoreSpan_PanOnly_IsNotTouched) {
    // 平移:跨度仍是 50,中心从 100 挪到 130 —— 必须原样放行(返回值恒等)。
    auto [lo, hi] = TimeSeriesChart::RestoreSpan(105.0, 155.0, 50.0);
    EXPECT_DOUBLE_EQ(lo, 105.0);
    EXPECT_DOUBLE_EQ(hi, 155.0);
}

TEST(TimeSeriesChartTest, RestoreSpan_ZoomIn_SnapsBackKeepingCentre) {
    // 放大到跨度 20(中心 130) → 拉回跨度 50,中心不动。
    auto [lo, hi] = TimeSeriesChart::RestoreSpan(120.0, 140.0, 50.0);
    EXPECT_DOUBLE_EQ(lo, 105.0);
    EXPECT_DOUBLE_EQ(hi, 155.0);
    EXPECT_DOUBLE_EQ(hi - lo, 50.0);
}

TEST(TimeSeriesChartTest, RestoreSpan_ZoomOut_SnapsBackKeepingCentre) {
    auto [lo, hi] = TimeSeriesChart::RestoreSpan(0.0, 260.0, 50.0);
    EXPECT_DOUBLE_EQ((lo + hi) * 0.5, 130.0);
    EXPECT_DOUBLE_EQ(hi - lo, 50.0);
}

TEST(TimeSeriesChartTest, RestoreSpan_Disabled_IsIdentity) {
    // span<=0 = 未启用固定 → 必须恒等,否则「不固定」时会被反向锁死。
    auto [lo, hi] = TimeSeriesChart::RestoreSpan(10.0, 99.0, 0.0);
    EXPECT_DOUBLE_EQ(lo, 10.0);
    EXPECT_DOUBLE_EQ(hi, 99.0);
    auto [lo2, hi2] = TimeSeriesChart::RestoreSpan(10.0, 99.0, -5.0);
    EXPECT_DOUBLE_EQ(lo2, 10.0);
    EXPECT_DOUBLE_EQ(hi2, 99.0);
}

TEST(TimeSeriesChartTest, RestoreSpan_DegenerateRange_IsIdentity) {
    // 首帧/塌缩量程(hi<=lo):没有可用的中心,别硬造一个,交给上层的定心逻辑。
    auto [lo, hi] = TimeSeriesChart::RestoreSpan(50.0, 50.0, 20.0);
    EXPECT_DOUBLE_EQ(lo, 50.0);
    EXPECT_DOUBLE_EQ(hi, 50.0);
}

TEST(TimeSeriesChartTest, RestoreSpan_ToleranceIsRelative_CatchesLargeMagnitudeZoom) {
    // ★ 容差按 span 相对取。AA 比价量纲 ~7000,固定高度 50;
    //   放大到 49.9 只差 0.1 —— 绝对 epsilon(比如 1e-6 之于 7000)会把它当噪声放过去,
    //   于是大量纲下「固定」又悄悄失效。这条就是钉这个。
    auto [lo, hi] = TimeSeriesChart::RestoreSpan(6975.05, 7024.95, 50.0);
    EXPECT_DOUBLE_EQ(hi - lo, 50.0);
    EXPECT_DOUBLE_EQ((lo + hi) * 0.5, 7000.0);
    // 反向:真正等于 span 的必须恒等,不能每帧都"纠偏"造成抖动。
    auto [lo2, hi2] = TimeSeriesChart::RestoreSpan(6975.0, 7025.0, 50.0);
    EXPECT_DOUBLE_EQ(lo2, 6975.0);
    EXPECT_DOUBLE_EQ(hi2, 7025.0);
}
