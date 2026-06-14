#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// DepthLadder — depth-of-market (DOM) price ladder widget (namespace
// unigui::trading)
//
// Presentation-only retained-mode widget bound to a non-owning `OrderBook`
// model (see order_book.h). Renders the aggregated book as a classic vertical
// price ladder: asks stacked highest→lowest on top, an optional spread row in
// the middle, then bids highest→lowest below. Each level draws a horizontal
// depth bar scaled to the largest visible size, the aggregated size, and the
// price. The widget never mutates the model — the embedder feeds
// snapshots/deltas into the `OrderBook` and the ladder draws what it holds.
//
// Behaviours: optional auto-/one-shot scroll so the inside market stays
// centered, and click-to-trade callbacks fired when a bid or ask level is
// clicked (the price/size of the clicked level are passed back). Gated by
// UNIGUI_MODULE_TRADING.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/trading/order_book.h>
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <cstdint>
#include <functional>
#include <string>

namespace unigui::trading {

class DepthLadder : public FluentWidget<DepthLadder> {
public:
    enum class Side { Bid, Ask };
    /// Fired when a price level is clicked. @p side is the side of the clicked
    /// level, @p price / @p size are that level's aggregated values.
    using LevelCallback = std::function<void(Side side, double price, std::int64_t size)>;

    explicit DepthLadder(std::string name);

    void Render() override;

    // ── Model binding ────────────────────────────────────────────────────────
    /// Bind a non-owning OrderBook. Caller keeps it alive for the ladder's life.
    void SetBook(const OrderBook* book) { book_ = book; }
    const OrderBook* Book() const { return book_; }

    // ── Layout ───────────────────────────────────────────────────────────────
    /// Max levels rendered per side. 0 (default) shows every level in the book.
    void SetDepth(int levels);
    int Depth() const { return depth_; }
    /// Per-row height in pixels. Default 20.
    void SetRowHeight(float h);
    float RowHeight() const { return rowH_; }
    /// Width of each size column in pixels. 0 (default) splits the leftover
    /// width (after the price column) evenly between the two size columns.
    void SetSizeColumnWidth(float w);
    float SizeColumnWidth() const { return sizeColW_; }
    /// Width of the centre price column in pixels. Default 72.
    void SetPriceColumnWidth(float w);
    float PriceColumnWidth() const { return priceColW_; }
    /// Explicit widget size; (-1,-1) (default) fills the available region.
    void SetSize(const ImVec2& size) { size_ = size; }
    ImVec2 Size() const { return size_; }

    // ── Appearance ───────────────────────────────────────────────────────────
    void SetBidColor(ImU32 c) { bid_ = c; }
    void SetAskColor(ImU32 c) { ask_ = c; }
    ImU32 BidColor() const { return bid_; }
    ImU32 AskColor() const { return ask_; }
    /// Opacity (0..1) of the depth bars behind each level. Default 0.35.
    void SetBarOpacity(float a);
    float BarOpacity() const { return barOpacity_; }
    /// Decimal places for prices. Default 2.
    void SetPriceDecimals(int d);
    int PriceDecimals() const { return priceDecimals_; }
    /// Show the spread/mid divider row between asks and bids (default on).
    void SetShowSpreadRow(bool on) { showSpread_ = on; }
    bool ShowSpreadRow() const { return showSpread_; }
    /// Follow the active ImGui theme for the ladder background/text (default on).
    void SetThemeBackground(bool on) { themeBackground_ = on; }
    bool ThemeBackground() const { return themeBackground_; }
    /// Draw a border around the ladder region (default on).
    void SetBorder(bool on) { border_ = on; }

    // ── Behaviour ────────────────────────────────────────────────────────────
    /// Keep the inside market vertically centred every frame (default off).
    void SetAutoCenter(bool on) { autoCenter_ = on; }
    bool AutoCenter() const { return autoCenter_; }
    /// Request a one-shot re-centre on the inside market on the next render.
    void CenterOnMarket() { centerRequested_ = true; }
    /// Callback fired when any level row is clicked (click-to-trade).
    void SetOnLevelClick(LevelCallback cb) { onLevelClick_ = std::move(cb); }

    // ── Fluent wrappers ──────────────────────────────────────────────────────
    DepthLadder& WithBook(const OrderBook* b) {
        SetBook(b);
        return *this;
    }
    DepthLadder& WithDepth(int n) {
        SetDepth(n);
        return *this;
    }
    DepthLadder& WithRowHeight(float h) {
        SetRowHeight(h);
        return *this;
    }
    DepthLadder& WithSizeColumnWidth(float w) {
        SetSizeColumnWidth(w);
        return *this;
    }
    DepthLadder& WithPriceColumnWidth(float w) {
        SetPriceColumnWidth(w);
        return *this;
    }
    DepthLadder& WithSize(const ImVec2& s) {
        SetSize(s);
        return *this;
    }
    DepthLadder& WithBidColor(ImU32 c) {
        SetBidColor(c);
        return *this;
    }
    DepthLadder& WithAskColor(ImU32 c) {
        SetAskColor(c);
        return *this;
    }
    DepthLadder& WithBarOpacity(float a) {
        SetBarOpacity(a);
        return *this;
    }
    DepthLadder& WithPriceDecimals(int d) {
        SetPriceDecimals(d);
        return *this;
    }
    DepthLadder& WithShowSpreadRow(bool on = true) {
        SetShowSpreadRow(on);
        return *this;
    }
    DepthLadder& WithThemeBackground(bool on = true) {
        SetThemeBackground(on);
        return *this;
    }
    DepthLadder& WithBorder(bool on = true) {
        SetBorder(on);
        return *this;
    }
    DepthLadder& WithAutoCenter(bool on = true) {
        SetAutoCenter(on);
        return *this;
    }
    DepthLadder& WithOnLevelClick(LevelCallback cb) {
        SetOnLevelClick(std::move(cb));
        return *this;
    }

private:
    // Draw a single ladder row at the current cursor; returns true if clicked.
    bool DrawLevelRow(int rowIndex, Side side, const Level& level, std::int64_t maxSize,
                      float totalW);
    void DrawSpreadRow(float totalW);
    // Translucent bar colour derived from a side's base colour + barOpacity_.
    ImU32 BarColor(ImU32 base) const;

    const OrderBook* book_ = nullptr;
    int depth_ = 0;
    float rowH_ = 20.0f;
    float sizeColW_ = 0.0f; // 0 ⇒ auto-split
    float priceColW_ = 72.0f;
    ImVec2 size_ = ImVec2(-1, -1);

    ImU32 bid_ = IM_COL32(38, 166, 91, 255); // green
    ImU32 ask_ = IM_COL32(217, 60, 60, 255); // red
    float barOpacity_ = 0.35f;
    int priceDecimals_ = 2;
    bool showSpread_ = true;
    bool themeBackground_ = true;
    bool border_ = true;

    bool autoCenter_ = false;
    bool centerRequested_ = false;
    LevelCallback onLevelClick_;
};

} // namespace unigui::trading
