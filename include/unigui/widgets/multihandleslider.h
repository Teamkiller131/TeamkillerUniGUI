#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <functional>
#include <vector>

namespace unigui {

/// Slider tick entry — position + metadata
struct SliderTick {
    int id = -1;
    float position = 0.f; // 0.0 .. barMax
    ImU32 color = IM_COL32(14, 165, 233, 255);
};

/// MultiHandleSlider — draggable multi-handle slider bar
class MultiHandleSlider : public FluentWidget<MultiHandleSlider> {
public:
    MultiHandleSlider(std::string name);

    void Render() override;

    // ── Tick management ──────────────────────────────────────────────────
    void SetTicks(const std::vector<SliderTick>& ticks);
    const std::vector<SliderTick>& GetTicks() const { return ticks_; }
    void AddTick(SliderTick tick);
    void RemoveTick(int id);

    // ── Range ────────────────────────────────────────────────────────────
    /// Set the value range. `max` is clamped to at least `min + 1` so the bar always
    /// has a non-degenerate span (the renderer divides by it).
    void SetRange(float min, float max);
    float GetRangeMin() const { return rangeMin_; }
    float GetRangeMax() const { return rangeMax_; }

    // ── Callbacks ─────────────────────────────────────────────────────────
    using TickChangedFn = std::function<void(int id, float newPos)>;
    void SetOnTickChanged(TickChangedFn fn) { onChange_ = std::move(fn); }

    // ── Custom overlay per tick ──────────────────────────────────────────
    using TickOverlayFn = std::function<void(int id, int index, float x, float barWidth)>;
    void SetTickOverlay(TickOverlayFn fn) { overlayFn_ = std::move(fn); }

    // ── Current marker ───────────────────────────────────────────────────
    void SetCurrentMarker(float pos, ImU32 color) {
        markerPos_ = pos;
        markerColor_ = color;
        hasMarker_ = true;
    }

    // ── Fluent (chainable) helpers — return MultiHandleSlider& via CRTP base ──────────
    MultiHandleSlider& WithTicks(const std::vector<SliderTick>& ticks) {
        SetTicks(ticks);
        return *this;
    }
    MultiHandleSlider& WithRange(float min, float max) {
        SetRange(min, max);
        return *this;
    }
    MultiHandleSlider& WithOnTickChanged(TickChangedFn fn) {
        SetOnTickChanged(std::move(fn));
        return *this;
    }
    MultiHandleSlider& WithTickOverlay(TickOverlayFn fn) {
        SetTickOverlay(std::move(fn));
        return *this;
    }
    MultiHandleSlider& WithCurrentMarker(float pos, ImU32 color) {
        SetCurrentMarker(pos, color);
        return *this;
    }

private:
    std::vector<SliderTick> ticks_;
    float rangeMin_ = 0.f, rangeMax_ = 100.f;
    float barHeight_ = 8.f;
    int activeTick_ = -1;
    TickChangedFn onChange_;
    TickOverlayFn overlayFn_;
    float markerPos_ = 0.f;
    ImU32 markerColor_ = IM_COL32(255, 255, 255, 180);
    bool hasMarker_ = false;
};

} // namespace unigui
