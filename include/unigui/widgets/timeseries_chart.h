#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>
#include <implot.h>

#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace unigui {

struct TimeSeriesDef {
    std::string label;
    ImU32 color = IM_COL32(14, 165, 233, 255);
    float lineWeight = 1.5f;
    int yAxisId = 1;
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

    /// Time-first append — reads naturally as (when, what) and avoids transposing
    /// the value/timestamp arguments of AppendPoint. Equivalent to
    /// AppendPoint(seriesId, value, timestamp).
    void AppendSample(int seriesId, double timestamp, float value) {
        AppendPoint(seriesId, value, timestamp);
    }

    /// Replace a series' entire point set in one shot (x = timestamps, y = values).
    /// Use this when the caller owns a complete, possibly out-of-order history buffer
    /// (e.g. live ticks + late multi-packet backfill): the points are sorted by X and
    /// trimmed to the sliding window, giving a single source of truth with no
    /// double-counting and no dropped late arrivals.
    void SetSeriesData(int seriesId, const std::vector<double>& xs, const std::vector<double>& ys);

    /// Clear all series data.
    void ClearAll();

    /// Set sliding window size (number of points).
    void SetSlidingWindow(int maxPoints);

    /// Auto-fit Y axis range.
    void SetYAxisAutoFit(bool on);
    /// When true (default) and auto-fit is on, the Y axis fits ONLY to data that
    /// falls inside the currently visible X range (ImPlotAxisFlags_RangeFit), so
    /// zooming/panning the X axis rescales Y to the visible window instead of the
    /// full dataset. When false, Y auto-fits to the entire dataset.
    void SetYRangeFit(bool on);
    /// Manual Y axis range (takes effect only when auto-fit is off).
    void SetYAxisRange(double min, double max);
    /// Fixed X axis range. When set, the X axis always shows [min, max] even with no data.
    void SetXAxisRange(double min, double max);

    /// X axis label.
    void SetXAxisLabel(const std::string& label);
    void SetYAxisLabel(const std::string& label);

    /// Show crosshair on hover.
    void SetCrosshairEnabled(bool on) { crosshair_ = on; }
    /// Show legend.
    void SetLegendEnabled(bool on) { legend_ = on; }
    /// Enable mouse pan (drag to scroll).
    void SetPanEnabled(bool on) { panEnabled_ = on; }
    /// Enable mouse wheel zoom.
    void SetZoomEnabled(bool on) { zoomEnabled_ = on; }

    /// Grid and background colors.
    void SetGridColor(ImU32 c) { gridColor_ = c; }
    /// When true (default), the plot background / border / grid follow the
    /// active ImGui theme palette. Set false to use fixed dark colors.
    void SetThemeBackground(bool on) { themeBackground_ = on; }
    void SetCrosshairFormatter(std::function<std::string(double, const std::vector<double>&)> fn) {
        crosshairFmt_ = std::move(fn);
    }
    void SetXAxisFormatter(std::function<int(double, char*, int, void*)> fn) {
        xAxisFmt_ = std::move(fn);
    }
    void SetRubberBandZoom(bool on) { rubberBandZoom_ = on; }
    int AddRefLine(std::string label, double value, ImU32 color);
    void RemoveRefLine(int id);

private:
    struct Series {
        int id;
        TimeSeriesDef def;
        std::deque<std::pair<double, float>> points; // timestamp → value
    };
    struct RefLine {
        int id;
        std::string label;
        double value;
        ImU32 color;
    };
    std::vector<Series> series_;
    std::vector<RefLine> refLines_;
    int nextId_ = 1;
    int nextRefId_ = 1;
    int slidingWindow_ = 500;
    bool yAutoFit_ = true;
    bool yRangeFit_ = true;
    double yMin_ = 0, yMax_ = 100;
    bool xRangeSet_ = false;
    double xMin_ = 0, xMax_ = 1;
    std::string xLabel_, yLabel_;
    bool crosshair_ = false;
    std::function<std::string(double, const std::vector<double>&)> crosshairFmt_;
    std::function<int(double, char*, int, void*)> xAxisFmt_;
    bool legend_ = true;
    bool panEnabled_ = true;
    bool zoomEnabled_ = true;
    bool rubberBandZoom_ = true;
    bool themeBackground_ = true;
    ImU32 gridColor_ = IM_COL32(60, 60, 70, 70);
    double frameCounter_ = 0;
};

} // namespace unigui
