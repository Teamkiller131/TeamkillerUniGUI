#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// OrderTicket — order-entry widget  (namespace unigui::trading)
//
// Retained-mode order-entry form: symbol, side (Buy/Sell), order type
// (Market/Limit/Stop/StopLimit), quantity, limit price, stop price, and
// time-in-force (Day/GTC/IOC/FOK). The widget owns a single editable
// `OrderDraft`; validation is a pure, headless-testable method (`Validate()`)
// and submission snaps prices to the configured tick size before firing the
// submit callback. An optional confirmation modal gates submission.
//
// Presentation + a thin draft model — the widget never routes orders itself; it
// hands a validated `OrderDraft` to the embedder's OMS via the submit callback.
// Gated by UNIGUI_MODULE_TRADING.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/trading/quote.h> // Side
#include <unigui/widgets/shortcut.h>
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <cstdint>
#include <functional>
#include <string>

namespace unigui::trading {

enum class OrderType { Market, Limit, Stop, StopLimit };

inline const char* OrderTypeName(OrderType t) {
    switch (t) {
    case OrderType::Market:
        return "Market";
    case OrderType::Limit:
        return "Limit";
    case OrderType::Stop:
        return "Stop";
    case OrderType::StopLimit:
        return "StopLimit";
    }
    return "Unknown";
}

enum class TimeInForce { Day, GTC, IOC, FOK };

inline const char* TimeInForceName(TimeInForce t) {
    switch (t) {
    case TimeInForce::Day:
        return "Day";
    case TimeInForce::GTC:
        return "GTC";
    case TimeInForce::IOC:
        return "IOC";
    case TimeInForce::FOK:
        return "FOK";
    }
    return "Unknown";
}

/// The editable order being composed. Pure data + trivial derived getters.
struct OrderDraft {
    std::string symbol;
    Side side = Side::Buy;
    OrderType type = OrderType::Limit;
    TimeInForce tif = TimeInForce::Day;
    std::int64_t qty = 0;
    double price = 0.0;     // limit price (Limit / StopLimit)
    double stopPrice = 0.0; // stop trigger (Stop / StopLimit)

    /// A limit price is required for Limit and StopLimit orders.
    bool NeedsPrice() const { return type == OrderType::Limit || type == OrderType::StopLimit; }
    /// A stop trigger is required for Stop and StopLimit orders.
    bool NeedsStop() const { return type == OrderType::Stop || type == OrderType::StopLimit; }
};

/// Result of validating a draft. Empty `message` iff `ok`.
struct OrderValidation {
    bool ok = false;
    std::string message;
    explicit operator bool() const { return ok; }
};

class OrderTicket : public FluentWidget<OrderTicket> {
public:
    /// Fired with a validated, tick-snapped draft when the order is submitted.
    using SubmitCallback = std::function<void(const OrderDraft&)>;

    explicit OrderTicket(std::string name);

    void Render() override;

    // ── Draft access ─────────────────────────────────────────────────────────
    OrderDraft& Draft() { return draft_; }
    const OrderDraft& Draft() const { return draft_; }
    void SetSymbol(std::string s) { draft_.symbol = std::move(s); }
    void SetSide(Side s) { draft_.side = s; }
    void SetOrderType(OrderType t) { draft_.type = t; }
    void SetTimeInForce(TimeInForce t) { draft_.tif = t; }
    void SetQuantity(std::int64_t q) { draft_.qty = q; }
    void SetPrice(double p) { draft_.price = p; }
    void SetStopPrice(double p) { draft_.stopPrice = p; }

    // ── Validation / submission ──────────────────────────────────────────────
    /// Pure check of the current draft against the configured constraints.
    /// Never touches ImGui — safe to call headlessly.
    OrderValidation Validate() const;
    /// Validate, snap prices to the tick size, and (if valid) fire the submit
    /// callback. Returns the validation result. Does not require an ImGui frame.
    OrderValidation Submit();

    // ── Configuration ────────────────────────────────────────────────────────
    /// Tick size used for the price input step and to snap prices on submit.
    /// <= 0 (default 0.01) disables snapping but keeps a sane input step.
    void SetTickSize(double t);
    double TickSize() const { return tickSize_; }
    /// Quantity input step. Default 1 (clamped to >= 1).
    void SetQuantityStep(std::int64_t step);
    std::int64_t QuantityStep() const { return qtyStep_; }
    /// Largest allowed quantity (0 = unbounded). Used by Validate().
    void SetMaxQuantity(std::int64_t maxQty) { maxQty_ = maxQty < 0 ? 0 : maxQty; }
    std::int64_t MaxQuantity() const { return maxQty_; }
    void SetPriceDecimals(int d);
    int PriceDecimals() const { return priceDecimals_; }
    /// Require a confirmation modal before the submit callback fires (default off).
    void SetConfirm(bool on) { confirm_ = on; }
    bool Confirm() const { return confirm_; }
    /// Submit on Ctrl+Enter while the ticket is focused (default on).
    void SetHotkeySubmit(bool on) { hotkeySubmit_ = on; }
    /// Explicit widget size; (0,0) (default) auto-sizes to content.
    void SetSize(const ImVec2& s) { size_ = s; }

    // ── Callbacks ────────────────────────────────────────────────────────────
    void SetOnSubmit(SubmitCallback cb) { onSubmit_ = std::move(cb); }

    // ── Fluent wrappers ──────────────────────────────────────────────────────
    OrderTicket& WithSymbol(std::string s) {
        SetSymbol(std::move(s));
        return *this;
    }
    OrderTicket& WithSide(Side s) {
        SetSide(s);
        return *this;
    }
    OrderTicket& WithOrderType(OrderType t) {
        SetOrderType(t);
        return *this;
    }
    OrderTicket& WithTimeInForce(TimeInForce t) {
        SetTimeInForce(t);
        return *this;
    }
    OrderTicket& WithQuantity(std::int64_t q) {
        SetQuantity(q);
        return *this;
    }
    OrderTicket& WithPrice(double p) {
        SetPrice(p);
        return *this;
    }
    OrderTicket& WithStopPrice(double p) {
        SetStopPrice(p);
        return *this;
    }
    OrderTicket& WithTickSize(double t) {
        SetTickSize(t);
        return *this;
    }
    OrderTicket& WithQuantityStep(std::int64_t step) {
        SetQuantityStep(step);
        return *this;
    }
    OrderTicket& WithMaxQuantity(std::int64_t m) {
        SetMaxQuantity(m);
        return *this;
    }
    OrderTicket& WithPriceDecimals(int d) {
        SetPriceDecimals(d);
        return *this;
    }
    OrderTicket& WithConfirm(bool on = true) {
        SetConfirm(on);
        return *this;
    }
    OrderTicket& WithHotkeySubmit(bool on = true) {
        SetHotkeySubmit(on);
        return *this;
    }
    OrderTicket& WithSize(const ImVec2& s) {
        SetSize(s);
        return *this;
    }
    OrderTicket& WithOnSubmit(SubmitCallback cb) {
        SetOnSubmit(std::move(cb));
        return *this;
    }

private:
    void DrawSideToggle();
    void DrawTypeAndTif();
    void DrawQtyAndPrice();
    // Validate then either open the confirm modal or submit immediately. Called
    // from the submit button and the Ctrl+Enter hotkey (needs an ImGui frame).
    void RequestSubmit();

    OrderDraft draft_;
    double tickSize_ = 0.01;
    std::int64_t qtyStep_ = 1;
    std::int64_t maxQty_ = 0;
    int priceDecimals_ = 2;
    bool confirm_ = false;
    bool hotkeySubmit_ = true;
    bool confirmPending_ = false; // confirm modal is open / awaiting decision
    ImVec2 size_ = ImVec2(0, 0);

    SubmitCallback onSubmit_;
    ShortcutManager hotkeys_;
};

} // namespace unigui::trading
