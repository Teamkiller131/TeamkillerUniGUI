#pragma once

#include <unigui/widgets/widget_base.h>

#include <string>

namespace unigui {

/// Gauge — a circular / radial progress dial for dashboards and KPI tiles.
/// Complements the linear ProgressBar with a ring (or speedometer-style arc)
/// showing a value within a range, an optional centre label, and a
/// theme-derived accent fill. Drawn entirely via the window draw-list.
class Gauge : public FluentWidget<Gauge> {
public:
    explicit Gauge(std::string name, float value = 0.f);

    void Render() override;

    // ── Value / range ───────────────────────────────────────────────────
    void SetValue(float v) { value_ = v; }
    float GetValue() const { return value_; }
    void SetRange(float minV, float maxV);
    float GetMin() const { return min_; }
    float GetMax() const { return max_; }
    /// Fraction of the arc filled (0..1), clamped — derived from value/range.
    float GetFraction() const;

    // ── Geometry ────────────────────────────────────────────────────────
    void SetRadius(float r) { radius_ = r; }
    float GetRadius() const { return radius_; }
    void SetThickness(float t) { thickness_ = t; }
    /// Arc span in degrees, measured clockwise from the bottom. 360 = full ring,
    /// 270 = classic open-bottom speedometer (the default).
    void SetSweepDegrees(float deg) { sweepDeg_ = deg; }
    float GetSweepDegrees() const { return sweepDeg_; }

    // ── Appearance ──────────────────────────────────────────────────────
    void SetTrackColor(ImU32 rgba) { trackColor_ = rgba; }
    void SetFillColor(ImU32 rgba) { fillColor_ = rgba; }
    /// Show the value as a centred "NN%" label (true) or hide it (false).
    void SetShowPercent(bool on) { showPercent_ = on; }
    /// Override the centre label with custom text (clears the percent display).
    void SetCenterLabel(std::string text);
    const std::string& GetCenterLabel() const { return centerLabel_; }

    // ── Fluent (typed) ──────────────────────────────────────────────────
    Gauge& WithValue(float v) {
        SetValue(v);
        return *this;
    }
    Gauge& WithRange(float minV, float maxV) {
        SetRange(minV, maxV);
        return *this;
    }
    Gauge& WithRadius(float r) {
        SetRadius(r);
        return *this;
    }
    Gauge& WithThickness(float t) {
        SetThickness(t);
        return *this;
    }
    Gauge& WithSweepDegrees(float deg) {
        SetSweepDegrees(deg);
        return *this;
    }
    Gauge& WithTrackColor(ImU32 rgba) {
        SetTrackColor(rgba);
        return *this;
    }
    Gauge& WithFillColor(ImU32 rgba) {
        SetFillColor(rgba);
        return *this;
    }
    Gauge& WithShowPercent(bool on = true) {
        SetShowPercent(on);
        return *this;
    }
    Gauge& WithCenterLabel(std::string text) {
        SetCenterLabel(std::move(text));
        return *this;
    }

private:
    float value_ = 0.f;
    float min_ = 0.f, max_ = 1.f;
    float radius_ = 40.f;
    float thickness_ = 8.f;
    float sweepDeg_ = 270.f;
    ImU32 trackColor_ = 0; // 0 = derive from theme
    ImU32 fillColor_ = 0;  // 0 = derive from theme accent
    bool showPercent_ = true;
    std::string centerLabel_; // non-empty overrides the percent label
};

} // namespace unigui
