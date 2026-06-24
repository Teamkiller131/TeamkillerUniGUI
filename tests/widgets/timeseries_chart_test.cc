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
