#include <unigui/trading/candlestick_chart.h>
#include <unigui/trading/ohlc_series.h>

#include <imgui.h>
#include <implot.h>

#include <gtest/gtest.h>

using namespace unigui::trading;

// Fixture: a headless ImGui + ImPlot frame, mirroring the im-layer test fixture
// but adding an ImPlot context (required for the candlestick widget's plotting).
class CandlestickChartTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1024, 768);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    // Populate a series with a handful of deterministic bars.
    static OhlcSeries MakeSeries(int bars = 5, double interval = 60.0) {
        OhlcSeries s(interval);
        for (int i = 0; i < bars; ++i) {
            const double t = i * interval + 1.0;
            const double base = 100.0 + i;
            s.AddTick(t, base);         // open
            s.AddTick(t + 1, base + 2); // high
            s.AddTick(t + 2, base - 1); // low
            s.AddTick(t + 3, base + 1); // close
        }
        return s;
    }
};

TEST_F(CandlestickChartTest, Defaults) {
    CandlestickChart chart("c1");
    EXPECT_EQ(chart.Series(), nullptr);
    EXPECT_FLOAT_EQ(chart.CandleWidth(), 0.5f);
    EXPECT_FALSE(chart.VolumePanel());
    // Default colors are distinct (bull vs bear).
    EXPECT_NE(chart.BullColor(), chart.BearColor());
}

TEST_F(CandlestickChartTest, SetSeries_Binds) {
    OhlcSeries s = MakeSeries();
    CandlestickChart chart("c2");
    chart.SetSeries(&s);
    EXPECT_EQ(chart.Series(), &s);
}

TEST_F(CandlestickChartTest, CandleWidth_Clamped) {
    CandlestickChart chart("c3");
    chart.SetCandleWidth(5.0f); // above 1.0
    EXPECT_FLOAT_EQ(chart.CandleWidth(), 1.0f);
    chart.SetCandleWidth(0.0f); // below 0.05
    EXPECT_FLOAT_EQ(chart.CandleWidth(), 0.05f);
}

TEST_F(CandlestickChartTest, Fluent_Chaining_ReturnsDerived) {
    OhlcSeries s = MakeSeries();
    CandlestickChart chart("c4");
    CandlestickChart& ref = chart.WithSeries(&s).WithVolumePanel(true).WithCandleWidth(0.7f);
    EXPECT_EQ(&ref, &chart);
    EXPECT_EQ(chart.Series(), &s);
    EXPECT_TRUE(chart.VolumePanel());
    EXPECT_FLOAT_EQ(chart.CandleWidth(), 0.7f);
}

TEST_F(CandlestickChartTest, Render_NullSeries_NoCrash) {
    CandlestickChart chart("c5");
    ImGui::Begin("w");
    chart.Render(); // no series bound — must early-out without crashing
    ImGui::End();
}

TEST_F(CandlestickChartTest, Render_WithSeries_NoCrash) {
    OhlcSeries s = MakeSeries(8);
    CandlestickChart chart("c6");
    chart.SetSeries(&s);
    ImGui::Begin("w");
    chart.Render();
    ImGui::End();
}

TEST_F(CandlestickChartTest, Render_WithVolumePanel_NoCrash) {
    OhlcSeries s = MakeSeries(8);
    CandlestickChart chart("c7");
    chart.SetSeries(&s);
    chart.SetVolumePanel(true);
    ImGui::Begin("w");
    chart.Render();
    ImGui::End();
}

TEST_F(CandlestickChartTest, Render_EmptySeries_NoCrash) {
    OhlcSeries s(60.0); // no bars
    CandlestickChart chart("c8");
    chart.SetSeries(&s);
    ImGui::Begin("w");
    chart.Render();
    ImGui::End();
}

TEST_F(CandlestickChartTest, Render_Hidden_DoesNothing) {
    OhlcSeries s = MakeSeries();
    CandlestickChart chart("c9");
    chart.SetSeries(&s);
    chart.Hide();
    ImGui::Begin("w");
    chart.Render(); // hidden → early-out
    ImGui::End();
    EXPECT_FALSE(chart.IsVisible());
}

TEST_F(CandlestickChartTest, PlotCandlesticks_FreeFunction_NoCrash) {
    OhlcSeries s = MakeSeries(6);
    auto xs = s.Times();
    auto opens = s.Opens();
    auto highs = s.Highs();
    auto lows = s.Lows();
    auto closes = s.Closes();
    ImGui::Begin("w");
    if (ImPlot::BeginPlot("##free")) {
        PlotCandlesticks("OHLC", xs.data(), opens.data(), closes.data(), lows.data(), highs.data(),
                         static_cast<int>(xs.size()), s.Interval() * 0.25, IM_COL32(0, 255, 0, 255),
                         IM_COL32(255, 0, 0, 255));
        ImPlot::EndPlot();
    }
    ImGui::End();
}

TEST_F(CandlestickChartTest, PlotCandlesticks_ZeroCount_NoCrash) {
    ImGui::Begin("w");
    if (ImPlot::BeginPlot("##empty")) {
        double dummy = 0.0;
        PlotCandlesticks("OHLC", &dummy, &dummy, &dummy, &dummy, &dummy, 0, 1.0,
                         IM_COL32(0, 255, 0, 255), IM_COL32(255, 0, 0, 255));
        ImPlot::EndPlot();
    }
    ImGui::End();
}
