#pragma once

#include <unigui/widgets/widget_base.h>

#include <string>
#include <vector>

namespace unigui {

/// PriceTicker — a horizontally scrolling marquee of symbol / price / change
/// items, the classic strip along the top of a trading screen. Items scroll
/// right-to-left and wrap continuously; each item is tinted green/red with a
/// ▲/▼ arrow by the sign of its change. Presentation-only — feed it a snapshot
/// of items each frame (or update them in place).
class PriceTicker : public FluentWidget<PriceTicker> {
public:
    struct Item {
        std::string symbol;
        std::string price;
        float change = 0.f; // sign drives colour + arrow; magnitude unused
    };

    explicit PriceTicker(std::string name, std::vector<Item> items = {});

    void Render() override;

    // ── Items ───────────────────────────────────────────────────────────
    void SetItems(std::vector<Item> items);
    void AddItem(Item item);
    void Clear();
    std::size_t ItemCount() const { return items_.size(); }
    const std::vector<Item>& GetItems() const { return items_; }

    // ── Behaviour / appearance ──────────────────────────────────────────
    /// Scroll speed in pixels per second (default 60). Negative scrolls right.
    void SetSpeed(float pxPerSec) { speed_ = pxPerSec; }
    float GetSpeed() const { return speed_; }
    /// Pause/resume the scroll without losing the current offset.
    void SetPaused(bool on) { paused_ = on; }
    bool IsPaused() const { return paused_; }
    /// Strip height (defaults to the current frame height).
    void SetHeight(float h) { height_ = h; }
    /// Strip width (0 = use the full available content width, the default).
    void SetWidth(float w) { width_ = w; }
    void SetUpColor(ImU32 rgba) { upColor_ = rgba; }
    void SetDownColor(ImU32 rgba) { downColor_ = rgba; }
    /// Current scroll offset in pixels (exposed for testing / persistence).
    float GetScrollOffset() const { return offset_; }

    // ── Fluent (typed) ──────────────────────────────────────────────────
    PriceTicker& WithItems(std::vector<Item> items) {
        SetItems(std::move(items));
        return *this;
    }
    PriceTicker& WithSpeed(float pxPerSec) {
        SetSpeed(pxPerSec);
        return *this;
    }
    PriceTicker& WithPaused(bool on = true) {
        SetPaused(on);
        return *this;
    }
    PriceTicker& WithHeight(float h) {
        SetHeight(h);
        return *this;
    }
    PriceTicker& WithWidth(float w) {
        SetWidth(w);
        return *this;
    }

private:
    std::vector<Item> items_;
    float speed_ = 60.f;
    float offset_ = 0.f;
    float contentW_ = 0.f; // measured total width of one item cycle
    bool paused_ = false;
    float height_ = 0.f; // 0 = frame height
    float width_ = 0.f;  // 0 = available width
    ImU32 upColor_ = IM_COL32(0x2e, 0xd1, 0x5e, 0xFF);
    ImU32 downColor_ = IM_COL32(0xe5, 0x3e, 0x3e, 0xFF);
};

} // namespace unigui
