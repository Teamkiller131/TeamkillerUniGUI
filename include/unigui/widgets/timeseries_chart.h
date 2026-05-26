#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <deque>
#include <imgui.h>
#include <implot.h>

namespace unigui {

struct TimeSeriesDef {
    std::string label;
    ImU32 color = IM_COL32(14, 165, 233, 255);
    float lineWeight = 1.5f;
};

/// TimeSeriesChart — real-time time-series plot via implot.
/// Supports sliding window, auto-fit Y axis, crosshair, legend.
class TimeSeriesChart : public Widget {
public:
    TimeSeriesChart(std::string name);

    void Render() override;

    /// Add a new data series. Returns series ID.
    int AddSeries(TimeSeriesDef def);
    void RemoveSeries(int id);

    /// Append a data point (value) with optional timestamp.
    /// If timestamp < 0, uses internal frame counter.
    void AppendPoint(int seriesId, float value, double timestamp = -1.0);

    /// Clear all series data.
    void ClearAll();

    /// Set sliding window size (number of points).
    void SetSlidingWindow(int maxPoints);

    /// Auto-fit Y axis range.
    void SetYAxisAutoFit(bool on);
    /// Manual Y axis range.
    void SetYAxisRange(double min, double max);

    /// X axis label.
    void SetXAxisLabel(const std::string& label);
    void SetYAxisLabel(const std::string& label);

    /// Show crosshair on hover.
    void SetCrosshairEnabled(bool on) { crosshair_ = on; }
    /// Show legend.
    void SetLegendEnabled(bool on) { legend_ = on; }

    /// Grid and background colors.
    void SetGridColor(ImU32 c) { gridColor_ = c; }

private:
    struct Series {
        int id; TimeSeriesDef def;
        std::deque<std::pair<double, float>> points;  // timestamp → value
    };
    std::vector<Series> series_;
    int nextId_ = 1;
    int slidingWindow_ = 500;
    bool yAutoFit_ = true;
    double yMin_ = 0, yMax_ = 100;
    std::string xLabel_, yLabel_;
    bool crosshair_ = false;
    bool legend_ = true;
    ImU32 gridColor_ = IM_COL32(60, 60, 70, 70);
    double frameCounter_ = 0;
};

} // namespace unigui
