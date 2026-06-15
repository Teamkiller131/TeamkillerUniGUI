#pragma once

#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// SegmentedControl — a compact, single-select button group sharing one rounded
/// frame (the iOS-style "1D / 1W / 1M" timeframe selector). More compact than a
/// TabWidget and well suited to toolbars and chart headers. The selected segment
/// is highlighted with the theme accent; clicking changes the selection and
/// fires the optional onChange callback.
class SegmentedControl : public FluentWidget<SegmentedControl> {
public:
    explicit SegmentedControl(std::string name, std::vector<std::string> segments = {});

    void Render() override;

    // ── Segments ────────────────────────────────────────────────────────
    void SetSegments(std::vector<std::string> segments);
    void AddSegment(std::string label);
    void Clear();
    std::size_t SegmentCount() const { return segments_.size(); }
    const std::vector<std::string>& GetSegments() const { return segments_; }

    // ── Selection ───────────────────────────────────────────────────────
    /// Select by index; out-of-range values are ignored. Does not fire onChange
    /// (that is reserved for user clicks).
    void SetSelected(int index);
    int GetSelected() const { return selected_; }
    /// Currently-selected label, or "" if none / empty.
    std::string GetSelectedLabel() const;
    void SetOnChange(std::function<void(int, const std::string&)> cb) { onChange_ = std::move(cb); }

    // ── Appearance ──────────────────────────────────────────────────────
    /// Per-segment horizontal padding (height follows the frame height).
    void SetSegmentPadding(float px) { segPad_ = px; }
    /// Stretch the control to the full available width, dividing it evenly.
    void SetFillWidth(bool on) { fillWidth_ = on; }

    // ── Fluent (typed) ──────────────────────────────────────────────────
    SegmentedControl& WithSegments(std::vector<std::string> s) {
        SetSegments(std::move(s));
        return *this;
    }
    SegmentedControl& WithSelected(int index) {
        SetSelected(index);
        return *this;
    }
    SegmentedControl& WithOnChange(std::function<void(int, const std::string&)> cb) {
        SetOnChange(std::move(cb));
        return *this;
    }
    SegmentedControl& WithSegmentPadding(float px) {
        SetSegmentPadding(px);
        return *this;
    }
    SegmentedControl& WithFillWidth(bool on = true) {
        SetFillWidth(on);
        return *this;
    }

private:
    std::vector<std::string> segments_;
    int selected_ = 0;
    float segPad_ = 14.f;
    bool fillWidth_ = false;
    std::function<void(int, const std::string&)> onChange_;
};

} // namespace unigui
