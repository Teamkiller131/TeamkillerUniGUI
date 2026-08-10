#pragma once
#include <unigui/core/session_axis.h>
#include <unigui/widgets/widget_base.h>

#include <imgui.h>
#include <implot.h>

#include <cmath>
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
    /// Y-axis padding ratio for auto-fit: the fitted range is
    /// `[min − r·span, max + r·span]` where `span = max − min`, so the margin is
    /// proportional to how much the series actually MOVES, not to how large its
    /// values happen to be. (The original 2026-07-22 spec padded by a fraction of
    /// the value — `[min·(1−r), max·(1+r)]` — which flattened any series sitting
    /// far from zero but swinging only slightly; see PadRange below.)
    /// Default r = 0.05. Set 0 to restore ImPlot's raw AutoFit behavior.
    void SetYPadRatio(double r) { yPadRatio_ = r < 0 ? 0.0 : r; }
    /// Pure padding math (public so tests can pin the edge cases without an ImPlot
    /// frame): returns the padded [lo, hi]. Span-relative and therefore sign-agnostic
    /// — negative and cross-zero ranges pad outward by construction, with no special
    /// case. A degenerate (flat / single-point) input yields [lo−1, hi+1] so the axis
    /// never collapses to zero height.
    static std::pair<double, double> PadRange(double lo, double hi, double r) {
        // 按【数据跨度 span=hi-lo】外扩,而非按【绝对值】。旧实现 lo*(1−r)/hi*(1+r) 对
        // "偏离 0 很远但波动很小"的序列(如比价/价差 ~7000 而当日仅波动 ~60)会按【值】的
        // 5% 外扩(±350),把 60 的真实信号压成一条平线 + 粗刻度(200/500),看不清走势。
        // span 相对外扩(±3)才与波动成比例:近零数据两种口径几乎一致,偏移数据这里才正确。
        const double span = hi - lo;
        if (span > 0.0) {
            const double pad = span * r;
            return {lo - pad, hi + pad};
        }
        // 退化(单点 / 全平):给一个不塌缩的极小窗口(±1),避免零高度轴;
        // 想要"平线固定高度"的调用方应改用 SetYAxisMinSpan()。
        return {lo - 1.0, hi + 1.0};
    }
    /// Minimum Y-axis span (height) enforced while auto-fitting. 0 (default)
    /// disables the floor. When the data inside the visible X window spans less
    /// than `span`, the Y axis is held at exactly `span` (centered on the data)
    /// instead of zooming in tighter — so a near-flat series doesn't render its
    /// micro-noise as full-height swings. Has no effect when auto-fit is off.
    void SetYAxisMinSpan(double span) { minYSpan_ = span; }
    /// Manual Y axis range (also turns auto-fit off).
    ///
    /// Applies the range on the next frame and then **releases the axis** — the user can
    /// pan and zoom Y freely afterwards, and the range is not re-imposed until a
    /// *different* range is requested. Re-asserting the same values every frame from a
    /// render loop is therefore harmless (it will not fight the user's drag).
    /// Use SetYAxisRangeLocked(true) for the opposite behaviour.
    void SetYAxisRange(double min, double max);
    /// Re-impose the manual Y range on EVERY frame instead of applying it once and
    /// releasing the axis.
    ///
    /// This makes Y un-pannable and un-zoomable: every drag snaps back on the next frame.
    /// Only use it when the axis is genuinely a readout the user must not move. If the
    /// range is derived from live data (a centre that tracks the latest value, say),
    /// locking is almost always wrong — the axis will visibly jitter as the data updates,
    /// and any attempt to drag it fights the code. Prefer plain SetYAxisRange(), which
    /// applies the new range once and then hands the axis to the user.
    void SetYAxisRangeLocked(bool on) { yRangeLocked_ = on; }

    /// Pin the Y axis **height** (span) while still allowing the user to pan.
    ///
    /// `span > 0` enables it; `0` (default) disables. Panning only moves the centre, so
    /// it passes through untouched; zooming (wheel / rubber-band) changes the span and is
    /// undone on the next frame — the view stays where the user put it, at the height the
    /// caller demanded. This is what "固定纵轴" means to a trader: the height is a fixed
    /// yardstick they read swings against, so a stray wheel click must not silently
    /// rescale it.
    ///
    /// Why not an axis flag: ImPlot's `Lock`/`LockMin`/`LockMax` kill panning too, and
    /// this widget's SetPanEnabled/SetZoomEnabled only map to `ImPlotAxisFlags_NoMenus`
    /// (they do NOT gate input — see Render, where the computed flags are `(void)`-ed).
    /// So the span is restored after the fact rather than prevented up front; the visible
    /// cost is a one-frame bounce on zoom.
    void SetYAxisSpanLock(double span) { ySpanLock_ = span > 0.0 ? span : 0.0; }

    /// Pure span-restore math (public so tests can pin it without an ImPlot frame):
    /// keeps the midpoint of [lo, hi] and forces the width to `span`.
    /// A non-positive `span`, or an already-correct width, returns [lo, hi] unchanged —
    /// callers rely on the identity to detect "nothing to correct".
    static std::pair<double, double> RestoreSpan(double lo, double hi, double span) {
        if (!(span > 0.0) || !(hi > lo)) return {lo, hi};
        const double cur = hi - lo;
        // 容差按 span 相对取，不用绝对值：比价量纲 ~7000 与价差 ~4 差三个数量级，
        // 固定绝对 epsilon 在大量纲下会把真实缩放当成噪声放过去。
        if (std::fabs(cur - span) <= span * 1e-9) return {lo, hi};
        const double mid = (lo + hi) * 0.5;
        return {mid - span * 0.5, mid + span * 0.5};
    }
    /// Y gridline/label spacing in data units: `1` labels 0,1,2,3…, `2` labels 0,2,4,6…
    /// `0` (default) leaves ImPlot's automatic tick selection alone.
    ///
    /// Only applies when auto-fit is OFF — ticks must be declared during axis setup, and
    /// with auto-fit the range isn't known until after it. Pair it with
    /// SetYAxisRange()+SetYAxisRangeLocked(): a caller who wants to dictate the tick step
    /// wants to dictate the range too, otherwise the tick count swings with the data.
    ///
    /// A step that would need more than `kMaxYTicks` labels for the current range is
    /// ignored (auto ticks are used instead) rather than honoured: the guard is against a
    /// user typing 1 with a range of 100000 and freezing the UI while ImPlot lays out
    /// 100k labels. Silently drawing a black smear of overlapping text would be no better.
    void SetYAxisTickSpacing(double step) { yTickSpacing_ = step > 0.0 ? step : 0.0; }
    static constexpr int kMaxYTicks = 200;
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
    bool yRangeLocked_ = false;        // hold the manual range every frame (blocks pan/zoom)
    bool yRangeApplyPending_ = true;   // apply a newly-requested range for one frame, then release
    double lastYMin_ = 0.0, lastYMax_ = 0.0;  // actual Y range last frame; drives explicit ticks
    double ySpanLock_ = 0.0;     // >0 = 固定纵轴高度(只钉跨度,平移照常);见 SetYAxisSpanLock
    double yTickSpacing_ = 0.0;  // 0 = ImPlot's automatic ticks
    std::vector<double> yTickBuf_;  // reused across frames; ImPlot copies during setup
    double minYSpan_ = 0.0;    // 0 = disabled; else Y-axis height floor (auto-fit only)
    double yPadRatio_ = 0.05;  // auto-fit padding: ±r × 数据跨度(span=max−min),见 PadRange()
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
