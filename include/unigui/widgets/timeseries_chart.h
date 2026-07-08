#pragma once
#include <unigui/core/session_axis.h>
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
class TimeSeriesChart : public FluentWidget<TimeSeriesChart> {
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

    /// Update-or-append a point keyed by timestamp: if the series already has a
    /// point at exactly `timestamp`, its value is replaced in place; otherwise the
    /// point is appended (like AppendPoint). This is the "live forming bar/tick"
    /// pattern — the latest sample updates repeatedly at a fixed timestamp before a
    /// new one starts — without growing the series by one point per frame.
    void UpsertPoint(int seriesId, float value, double timestamp);

    /// Replace a series' entire point set in one shot (x = timestamps, y = values).
    /// Use this when the caller owns a complete, possibly out-of-order history buffer
    /// (e.g. live ticks + late multi-packet backfill): the points are sorted by X and
    /// trimmed to the sliding window, giving a single source of truth with no
    /// double-counting and no dropped late arrivals.
    void SetSeriesData(int seriesId, const std::vector<double>& xs, const std::vector<double>& ys);

    /// Clear all series data.
    void ClearAll();

    /// Number of points currently stored for a series (after sliding-window trim
    /// and any render-point decimation); -1 if the series id is unknown.
    int GetSeriesPointCount(int seriesId) const;

    /// Set sliding window size (number of points).
    void SetSlidingWindow(int maxPoints);

    /// Cap the number of points actually stored/plotted per series. When a series
    /// set via SetSeriesData exceeds this, it is LTTB-decimated (shape-preserving)
    /// down to ~`n` points — so a 100k-tick series renders fast without visual
    /// loss. `0` (default) disables decimation. Applied after the sliding window.
    void SetMaxRenderPoints(int n);

    /// Auto-fit Y axis range.
    void SetYAxisAutoFit(bool on);
    /// When true (default) and auto-fit is on, the Y axis fits ONLY to data that
    /// falls inside the currently visible X range (ImPlotAxisFlags_RangeFit), so
    /// zooming/panning the X axis rescales Y to the visible window instead of the
    /// full dataset. When false, Y auto-fits to the entire dataset.
    void SetYRangeFit(bool on);
    /// Minimum Y-axis span (height) enforced while auto-fitting. 0 (default)
    /// disables the floor. When the data inside the visible X window spans less
    /// than `span`, the Y axis is held at exactly `span` (centered on the data)
    /// instead of zooming in tighter — so a near-flat series doesn't render its
    /// micro-noise as full-height swings. Has no effect when auto-fit is off.
    void SetYAxisMinSpan(double span) { minYSpan_ = span; }
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
    /// Convenience: format the X axis with a `SessionAxis`, so intraday tick
    /// labels read as wall-clock "HH:MM" while the axis itself stays gap-free
    /// (lunch/overnight breaks collapsed). The X values you plot must already be
    /// session-axis coordinates (`SessionAxis::ToAxis(secOfDay)`); this installs
    /// the matching inverse tick formatter. Equivalent to building the lambda by
    /// hand and calling SetXAxisFormatter.
    void SetSessionAxis(SessionAxis axis);
    void SetRubberBandZoom(bool on) { rubberBandZoom_ = on; }
    int AddRefLine(std::string label, double value, ImU32 color);
    void RemoveRefLine(int id);

    // ── Fluent (chainable) helpers — return TimeSeriesChart& via CRTP base ──────────
    TimeSeriesChart& WithSeriesData(int seriesId, const std::vector<double>& xs,
                                    const std::vector<double>& ys) {
        SetSeriesData(seriesId, xs, ys);
        return *this;
    }
    TimeSeriesChart& WithSlidingWindow(int maxPoints) {
        SetSlidingWindow(maxPoints);
        return *this;
    }
    TimeSeriesChart& WithMaxRenderPoints(int n) {
        SetMaxRenderPoints(n);
        return *this;
    }
    TimeSeriesChart& WithYAxisAutoFit(bool on) {
        SetYAxisAutoFit(on);
        return *this;
    }
    TimeSeriesChart& WithYRangeFit(bool on) {
        SetYRangeFit(on);
        return *this;
    }
    TimeSeriesChart& WithYAxisMinSpan(double span) {
        SetYAxisMinSpan(span);
        return *this;
    }
    TimeSeriesChart& WithYAxisRange(double min, double max) {
        SetYAxisRange(min, max);
        return *this;
    }
    TimeSeriesChart& WithXAxisRange(double min, double max) {
        SetXAxisRange(min, max);
        return *this;
    }
    TimeSeriesChart& WithXAxisLabel(const std::string& label) {
        SetXAxisLabel(label);
        return *this;
    }
    TimeSeriesChart& WithYAxisLabel(const std::string& label) {
        SetYAxisLabel(label);
        return *this;
    }
    TimeSeriesChart& WithCrosshairEnabled(bool on) {
        SetCrosshairEnabled(on);
        return *this;
    }
    TimeSeriesChart& WithLegendEnabled(bool on) {
        SetLegendEnabled(on);
        return *this;
    }
    TimeSeriesChart& WithPanEnabled(bool on) {
        SetPanEnabled(on);
        return *this;
    }
    TimeSeriesChart& WithZoomEnabled(bool on) {
        SetZoomEnabled(on);
        return *this;
    }
    TimeSeriesChart& WithGridColor(ImU32 c) {
        SetGridColor(c);
        return *this;
    }
    TimeSeriesChart& WithThemeBackground(bool on) {
        SetThemeBackground(on);
        return *this;
    }
    TimeSeriesChart& WithCrosshairFormatter(
        std::function<std::string(double, const std::vector<double>&)> fn) {
        SetCrosshairFormatter(std::move(fn));
        return *this;
    }
    TimeSeriesChart& WithXAxisFormatter(std::function<int(double, char*, int, void*)> fn) {
        SetXAxisFormatter(std::move(fn));
        return *this;
    }
    TimeSeriesChart& WithSessionAxis(SessionAxis axis) {
        SetSessionAxis(std::move(axis));
        return *this;
    }
    TimeSeriesChart& WithRubberBandZoom(bool on) {
        SetRubberBandZoom(on);
        return *this;
    }

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
    int maxRenderPoints_ = 0; // 0 = no decimation
    bool yAutoFit_ = true;
    bool yRangeFit_ = true;
    double yMin_ = 0, yMax_ = 100;
    double minYSpan_ = 0.0;    // 0 = disabled; else Y-axis height floor (auto-fit only)
    double lastXMin_ = -1e300; // visible X window cached from the previous frame,
    double lastXMax_ = 1e300;  // used to scope the min-span Y fit (see Render)
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
