#pragma once

#include <unigui/theme/color_tokens.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

/// MetricCard — a bordered, rounded KPI/status tile: an optional left accent
/// rail, a header row (optional status dot + title + right-aligned action slot),
/// and a body that is either a built-in value / delta / subtext layout or a
/// caller-supplied draw callback.
///
/// Replaces the hand-rolled "BeginChild + accent-bar AddRectFilled + status dot
/// + measured right-aligned button group" pod/account card that recurs across
/// the strategy tabs and fund panels. Delta values are coloured by sign through
/// the active theme `Up`/`Down` tokens (so CN red-up / Western green-up is one
/// `SetPolarity` away). Presentation-only; actions are caller callbacks.
class MetricCard : public FluentWidget<MetricCard> {
public:
    explicit MetricCard(std::string name);

    void Render() override;

    // ── Header ──────────────────────────────────────────────────────────
    MetricCard& WithTitle(std::string title) {
        title_ = std::move(title);
        return *this;
    }
    /// Right-aligned action slot drawn in the header row (e.g. a ButtonGroup).
    MetricCard& WithHeaderActions(std::function<void()> draw) {
        headerActions_ = std::move(draw);
        return *this;
    }
    /// Small status dot before the title, coloured by a semantic role.
    MetricCard& WithStatusDot(theme::Semantic role) {
        statusRole_ = role;
        hasStatus_ = true;
        return *this;
    }

    // ── Body (value layout) ─────────────────────────────────────────────
    MetricCard& WithValue(std::string value) {
        value_ = std::move(value);
        return *this;
    }
    /// A signed delta line under the value, coloured by the sign of `value`.
    MetricCard& WithDelta(double value, std::string display) {
        deltaValue_ = value;
        deltaText_ = std::move(display);
        hasDelta_ = true;
        return *this;
    }
    MetricCard& WithSubtext(std::string s) {
        subtext_ = std::move(s);
        return *this;
    }
    /// Custom body — replaces the value/delta/subtext layout entirely.
    MetricCard& WithBody(std::function<void()> draw) {
        body_ = std::move(draw);
        return *this;
    }

    // ── Geometry ────────────────────────────────────────────────────────
    /// Card size; (0,0) = auto (full available width, height from content).
    MetricCard& WithSize(float w, float h) {
        size_ = ImVec2(w, h);
        return *this;
    }
    MetricCard& WithAccentRail(bool on = true) {
        accentRail_ = on;
        return *this;
    }

    const std::string& GetTitle() const { return title_; }
    const std::string& GetValue() const { return value_; }

private:
    std::string title_, value_, subtext_, deltaText_;
    double deltaValue_ = 0.0;
    bool hasDelta_ = false;
    bool accentRail_ = true;
    bool hasStatus_ = false;
    theme::Semantic statusRole_ = theme::Semantic::Success;
    ImVec2 size_ = ImVec2(0.f, 0.f);
    std::function<void()> headerActions_;
    std::function<void()> body_;
};

} // namespace unigui
