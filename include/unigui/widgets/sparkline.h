#pragma once

#include <unigui/widgets/widget_base.h>

#include <string>
#include <vector>

namespace unigui {

/// Sparkline — a compact, axis-less trend chart for inline use in tables,
/// watchlists, KPI cards, and dashboards. Renders a small line / area / bar
/// plot of a numeric series via the window draw-list (no ImPlot dependency).
///
/// Presentation-only and allocation-light: feed it values with SetData() or a
/// rolling PushValue(), size it, and it auto-ranges to the data (or a fixed
/// range via SetRange). Pure geometry, so it is fully headless-testable.
class Sparkline : public FluentWidget<Sparkline> {
public:
    enum class Mode { Line, Area, Bar };

    explicit Sparkline(std::string name, Mode mode = Mode::Line);

    void Render() override;

    // ── Data ────────────────────────────────────────────────────────────
    void SetData(std::vector<float> values);
    /// Append a value; if a cap is set (SetMaxPoints), drops the oldest so the
    /// series scrolls — the natural "streaming price" use.
    void PushValue(float v);
    void Clear();
    const std::vector<float>& GetData() const { return data_; }
    std::size_t PointCount() const { return data_.size(); }

    /// Cap the retained point count for PushValue (0 = unbounded, the default).
    void SetMaxPoints(std::size_t n) { maxPoints_ = n; }
    std::size_t GetMaxPoints() const { return maxPoints_; }

    // ── Appearance ──────────────────────────────────────────────────────
    void SetMode(Mode m) { mode_ = m; }
    Mode GetMode() const { return mode_; }
    void SetSize(float w, float h) { size_ = ImVec2(w, h); }
    ImVec2 GetSize() const { return size_; }
    void SetLineColor(ImU32 rgba) { lineColor_ = rgba; }
    void SetFillColor(ImU32 rgba) { fillColor_ = rgba; }
    void SetLineThickness(float t) { thickness_ = t; }
    /// Draw a dot on the most recent point (handy for live series).
    void SetShowLastDot(bool on) { showLastDot_ = on; }
    /// Tint the line/fill green when the series ends up vs. its first point,
    /// red when down. Overrides the explicit line colour while enabled.
    void SetColorByTrend(bool on) { colorByTrend_ = on; }

    // ── Range ───────────────────────────────────────────────────────────
    /// Fix the vertical range. By default the sparkline auto-fits to its data.
    void SetRange(float minV, float maxV);
    /// Revert to auto-ranging (the constructed default).
    void SetAutoRange() { autoRange_ = true; }
    bool IsAutoRange() const { return autoRange_; }

    // ── Fluent (typed) ──────────────────────────────────────────────────
    Sparkline& WithData(std::vector<float> v) {
        SetData(std::move(v));
        return *this;
    }
    Sparkline& WithMode(Mode m) {
        SetMode(m);
        return *this;
    }
    Sparkline& WithSize(float w, float h) {
        SetSize(w, h);
        return *this;
    }
    Sparkline& WithLineColor(ImU32 rgba) {
        SetLineColor(rgba);
        return *this;
    }
    Sparkline& WithFillColor(ImU32 rgba) {
        SetFillColor(rgba);
        return *this;
    }
    Sparkline& WithLineThickness(float t) {
        SetLineThickness(t);
        return *this;
    }
    Sparkline& WithShowLastDot(bool on = true) {
        SetShowLastDot(on);
        return *this;
    }
    Sparkline& WithColorByTrend(bool on = true) {
        SetColorByTrend(on);
        return *this;
    }
    Sparkline& WithRange(float minV, float maxV) {
        SetRange(minV, maxV);
        return *this;
    }

private:
    std::vector<float> data_;
    Mode mode_ = Mode::Line;
    ImVec2 size_ = ImVec2(80.f, 20.f);
    ImU32 lineColor_ = 0; // 0 = derive from theme
    ImU32 fillColor_ = 0; // 0 = derive from line colour
    float thickness_ = 1.5f;
    bool showLastDot_ = false;
    bool colorByTrend_ = false;
    bool autoRange_ = true;
    float rangeMin_ = 0.f, rangeMax_ = 1.f;
    std::size_t maxPoints_ = 0;
};

} // namespace unigui
