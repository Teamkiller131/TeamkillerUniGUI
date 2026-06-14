#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// CandlestickChart — OHLC / candlestick chart widget  (namespace unigui::trading)
//
// Presentation-only retained-mode widget bound to a non-owning `OhlcSeries`
// model (see ohlc_series.h). Renders candlesticks via ImPlot's draw-list with
// an optional volume sub-panel, OHLCV crosshair tooltip, theme-aware background
// and date/time X-axis formatting. The widget never mutates the model — the
// embedder feeds ticks/bars into the `OhlcSeries` and the chart draws whatever
// the model currently holds.
//
// Gated by UNIGUI_MODULE_TRADING. The low-level `PlotCandlesticks()` free
// function is exposed for callers who drive their own ImPlot BeginPlot/EndPlot.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/trading/ohlc_series.h>
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <string>

namespace unigui::trading {

/// Low-level candlestick renderer. Call between ImPlot::BeginPlot/EndPlot (it
/// integrates with the current plot's legend and auto-fit). @p halfWidth is the
/// candle half-width expressed in X-axis (data) units. @p bullCol / @p bearCol
/// are used when close ≥ open / close < open respectively.
void PlotCandlesticks(const char* labelId, const double* xs, const double* opens,
                      const double* closes, const double* lows, const double* highs, int count,
                      double halfWidth, ImU32 bullCol, ImU32 bearCol);

class CandlestickChart : public FluentWidget<CandlestickChart> {
public:
    explicit CandlestickChart(std::string name);

    void Render() override;

    // ── Model binding ────────────────────────────────────────────────────────
    /// Bind a non-owning OhlcSeries. Caller keeps it alive for the chart's life.
    void SetSeries(const OhlcSeries* series) { series_ = series; }
    const OhlcSeries* Series() const { return series_; }

    // ── Appearance ───────────────────────────────────────────────────────────
    void SetBullColor(ImU32 c) { bull_ = c; }
    void SetBearColor(ImU32 c) { bear_ = c; }
    ImU32 BullColor() const { return bull_; }
    ImU32 BearColor() const { return bear_; }
    /// Candle body width as a fraction (0..1) of the bar interval. Default 0.5.
    void SetCandleWidth(float fraction);
    float CandleWidth() const { return widthFrac_; }
    /// Explicit chart size; (-1,-1) (default) fills the available region.
    void SetSize(const ImVec2& size) { size_ = size; }
    /// Series label shown in the legend. Default "OHLC".
    void SetSeriesLabel(std::string label) { seriesLabel_ = std::move(label); }
    void SetVolumeLabel(std::string label) { volumeLabel_ = std::move(label); }

    // ── Volume sub-panel ─────────────────────────────────────────────────────
    /// Show a volume bar panel below the price panel (shares the X axis).
    void SetVolumePanel(bool on) { volumePanel_ = on; }
    bool VolumePanel() const { return volumePanel_; }
    /// Height fraction (0..1) of the volume panel. Default 0.25.
    void SetVolumePanelRatio(float ratio);

    // ── Behaviour ────────────────────────────────────────────────────────────
    /// OHLCV tooltip on hover (default on). Distinct from Widget::SetTooltip.
    void SetHoverTooltip(bool on) { hoverTooltip_ = on; }
    /// Render a crosshair (default on).
    void SetCrosshair(bool on) { crosshair_ = on; }
    /// Built-in legend (default on).
    void SetLegend(bool on) { legend_ = on; }
    /// Format the X axis as date/time (epoch-seconds aware). Default on.
    void SetTimeAxis(bool on) { timeAxis_ = on; }
    /// Follow the active ImGui theme for plot background/border/grid (default on).
    void SetThemeBackground(bool on) { themeBackground_ = on; }
    void SetXAxisLabel(std::string label) { xLabel_ = std::move(label); }
    void SetYAxisLabel(std::string label) { yLabel_ = std::move(label); }

    // ── Fluent wrappers ──────────────────────────────────────────────────────
    CandlestickChart& WithSeries(const OhlcSeries* s) {
        SetSeries(s);
        return *this;
    }
    CandlestickChart& WithBullColor(ImU32 c) {
        SetBullColor(c);
        return *this;
    }
    CandlestickChart& WithBearColor(ImU32 c) {
        SetBearColor(c);
        return *this;
    }
    CandlestickChart& WithCandleWidth(float f) {
        SetCandleWidth(f);
        return *this;
    }
    CandlestickChart& WithSize(const ImVec2& s) {
        SetSize(s);
        return *this;
    }
    CandlestickChart& WithVolumePanel(bool on = true) {
        SetVolumePanel(on);
        return *this;
    }
    CandlestickChart& WithVolumePanelRatio(float r) {
        SetVolumePanelRatio(r);
        return *this;
    }
    CandlestickChart& WithHoverTooltip(bool on = true) {
        SetHoverTooltip(on);
        return *this;
    }
    CandlestickChart& WithCrosshair(bool on = true) {
        SetCrosshair(on);
        return *this;
    }
    CandlestickChart& WithLegend(bool on = true) {
        SetLegend(on);
        return *this;
    }
    CandlestickChart& WithTimeAxis(bool on = true) {
        SetTimeAxis(on);
        return *this;
    }
    CandlestickChart& WithThemeBackground(bool on = true) {
        SetThemeBackground(on);
        return *this;
    }
    CandlestickChart& WithXAxisLabel(std::string l) {
        SetXAxisLabel(std::move(l));
        return *this;
    }
    CandlestickChart& WithYAxisLabel(std::string l) {
        SetYAxisLabel(std::move(l));
        return *this;
    }

private:
    void PushThemeColors(int& pushed) const;
    void DrawPricePanel();
    void DrawVolumePanel();
    void DrawHoverTooltip() const;

    const OhlcSeries* series_ = nullptr;
    ImU32 bull_ = IM_COL32(38, 166, 91, 255); // green
    ImU32 bear_ = IM_COL32(217, 60, 60, 255); // red
    float widthFrac_ = 0.5f;
    ImVec2 size_ = ImVec2(-1, -1);
    std::string seriesLabel_ = "OHLC";
    std::string volumeLabel_ = "Volume";
    bool volumePanel_ = false;
    float volRatio_ = 0.25f;
    bool hoverTooltip_ = true;
    bool crosshair_ = true;
    bool legend_ = true;
    bool timeAxis_ = true;
    bool themeBackground_ = true;
    std::string xLabel_, yLabel_;
};

} // namespace unigui::trading
