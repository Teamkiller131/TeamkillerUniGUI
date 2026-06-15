#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// Trading blotter / watchlist / tape templates  (namespace unigui::trading)
//
// Pre-built `DataTable<T>` configurations for the common data-dense trading
// surfaces, bound to the thin row models in <unigui/trading/quote.h>:
//
//   • MakePositionsBlotter  → DataTable<Position>
//   • MakeOrdersBlotter     → DataTable<Order>
//   • MakeTradesTape        → DataTable<Trade>     (time & sales)
//   • MakeWatchlist         → DataTable<Quote>     (quote board)
//
// Each factory wires columns, a numeric/financial cell formatter (reusing
// `core/format_num.h`), sign-aware cell colours (delta arrows + green/red), and
// a pinned leading column (freeze-pane). The cell formatters and colour helpers
// are exposed as **pure functions** so they are unit-testable without an ImGui
// frame; the factories assemble them into a ready table the embedder binds data
// to via `SetDataSource()`.
//
// Header-only (DataTable is a template). The factories pull in the widgets
// layer, so include this only where `UNIGUI_MODULE_WIDGETS` is built.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/core/format_num.h>
#include <unigui/theme/color_tokens.h>
#include <unigui/trading/quote.h>
#include <unigui/widgets/datatable.h>

#include <imgui.h>

#include <ctime>
#include <string>

namespace unigui::trading {

// ── Shared colour helpers (pure; alpha 0 == "no override") ───────────────────

// Up/down colour by sign. `pol` selects the market convention: GreenUp (Western,
// the default — preserves existing behaviour) or RedUp (Chinese: a rise is red).
inline ImU32 DeltaColor(double v, double eps = 0.0,
                        theme::Polarity pol = theme::Polarity::GreenUp) {
    constexpr ImU32 kGreen = IM_COL32(38, 166, 91, 255);
    constexpr ImU32 kRed = IM_COL32(217, 60, 60, 255);
    const ImU32 up = (pol == theme::Polarity::GreenUp) ? kGreen : kRed;
    const ImU32 down = (pol == theme::Polarity::GreenUp) ? kRed : kGreen;
    switch (format::Sign(v, eps)) {
    case format::Direction::Up:
        return up;
    case format::Direction::Down:
        return down;
    case format::Direction::Flat:
        return 0; // no override
    }
    return 0;
}

// Buy/sell colour. Follows the same polarity: under RedUp, Buy is red and Sell
// green (the CN convention), matching limit-up/down colouring.
inline ImU32 SideColor(Side s, theme::Polarity pol = theme::Polarity::GreenUp) {
    constexpr ImU32 kGreen = IM_COL32(38, 166, 91, 255);
    constexpr ImU32 kRed = IM_COL32(217, 60, 60, 255);
    const ImU32 buy = (pol == theme::Polarity::GreenUp) ? kGreen : kRed;
    const ImU32 sell = (pol == theme::Polarity::GreenUp) ? kRed : kGreen;
    return s == Side::Buy ? buy : sell;
}

/// Prefix a formatted delta with an up/down arrow glyph (▲/▼), flat → none.
inline std::string WithArrow(double v, const std::string& body, double eps = 0.0) {
    switch (format::Sign(v, eps)) {
    case format::Direction::Up:
        return "\xE2\x96\xB2 " + body; // ▲
    case format::Direction::Down:
        return "\xE2\x96\xBC " + body; // ▼
    case format::Direction::Flat:
        return body;
    }
    return body;
}

/// epoch-seconds → "HH:MM:SS" (UTC). Pure; never throws.
inline std::string FormatClock(double epochSeconds) {
    const std::time_t t = static_cast<std::time_t>(epochSeconds);
    std::tm tmv{};
#if defined(_WIN32)
    gmtime_s(&tmv, &t);
#else
    gmtime_r(&t, &tmv);
#endif
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return buf;
}

// ── Positions blotter ────────────────────────────────────────────────────────
// Columns: Symbol | Qty | Avg | Last | Mkt Val | uPnL
inline std::string PositionCell(int col, const Position& p) {
    switch (col) {
    case 0:
        return p.symbol;
    case 1:
        return format::Thousands(static_cast<long long>(p.qty));
    case 2:
        return format::Fixed(p.avgPrice);
    case 3:
        return format::Fixed(p.last);
    case 4:
        return format::Fixed(p.MarketValue());
    case 5:
        return format::SignedDelta(p.UnrealizedPnL());
    default:
        return "";
    }
}

inline DataTable<Position> MakePositionsBlotter(std::string name,
                                                theme::Polarity pol = theme::Polarity::GreenUp) {
    DataTable<Position> t(std::move(name), {{"Symbol", 90.f},
                                            {"Qty", 70.f},
                                            {"Avg", 80.f},
                                            {"Last", 80.f},
                                            {"Mkt Val", 100.f},
                                            {"uPnL", 100.f}});
    t.SetCellFormatter([](int, int col, const Position& p) { return PositionCell(col, p); });
    t.SetCellColor([pol](int, int col, const Position& p) -> ImU32 {
        if (col == 5)
            return DeltaColor(p.UnrealizedPnL(), 0.0, pol);
        if (col == 1)
            return DeltaColor(static_cast<double>(p.qty), 0.0, pol);
        return 0;
    });
    t.SetFrozenColumns(1);
    return t;
}

// ── Orders blotter ───────────────────────────────────────────────────────────
// Columns: ID | Symbol | Side | Price | Qty | Filled | Status
inline std::string OrderCell(int col, const Order& o) {
    switch (col) {
    case 0:
        return o.id;
    case 1:
        return o.symbol;
    case 2:
        return SideName(o.side);
    case 3:
        return format::Fixed(o.price);
    case 4:
        return format::Thousands(static_cast<long long>(o.qty));
    case 5:
        return format::Thousands(static_cast<long long>(o.filled));
    case 6:
        return OrderStatusName(o.status);
    default:
        return "";
    }
}

inline DataTable<Order> MakeOrdersBlotter(std::string name,
                                          theme::Polarity pol = theme::Polarity::GreenUp) {
    DataTable<Order> t(std::move(name), {{"ID", 80.f},
                                         {"Symbol", 90.f},
                                         {"Side", 60.f},
                                         {"Price", 80.f},
                                         {"Qty", 70.f},
                                         {"Filled", 70.f},
                                         {"Status", 110.f}});
    t.SetCellFormatter([](int, int col, const Order& o) { return OrderCell(col, o); });
    t.SetCellColor([pol](int, int col, const Order& o) -> ImU32 {
        if (col == 2)
            return SideColor(o.side, pol);
        return 0;
    });
    t.SetFrozenColumns(2);
    return t;
}

// ── Trades tape (time & sales) ───────────────────────────────────────────────
// Columns: Time | Symbol | Side | Price | Qty | Notional
inline std::string TradeCell(int col, const Trade& tr) {
    switch (col) {
    case 0:
        return FormatClock(tr.timestamp);
    case 1:
        return tr.symbol;
    case 2:
        return SideName(tr.side);
    case 3:
        return format::Fixed(tr.price);
    case 4:
        return format::Thousands(static_cast<long long>(tr.qty));
    case 5:
        return format::Fixed(tr.Notional());
    default:
        return "";
    }
}

inline DataTable<Trade> MakeTradesTape(std::string name,
                                       theme::Polarity pol = theme::Polarity::GreenUp) {
    DataTable<Trade> t(std::move(name), {{"Time", 90.f},
                                         {"Symbol", 90.f},
                                         {"Side", 60.f},
                                         {"Price", 80.f},
                                         {"Qty", 70.f},
                                         {"Notional", 110.f}});
    t.SetCellFormatter([](int, int col, const Trade& tr) { return TradeCell(col, tr); });
    t.SetCellColor([pol](int, int col, const Trade& tr) -> ImU32 {
        if (col == 2 || col == 3)
            return SideColor(tr.side, pol);
        return 0;
    });
    t.SetFrozenColumns(1);
    return t;
}

// ── Watchlist / quote board ──────────────────────────────────────────────────
// Columns: Symbol | Last | Chg | Chg% | Bid | Ask | Volume
inline std::string QuoteCell(int col, const Quote& q) {
    switch (col) {
    case 0:
        return q.symbol;
    case 1:
        return format::Fixed(q.last);
    case 2:
        return WithArrow(q.Change(), format::SignedDelta(q.Change()));
    case 3:
        return format::Percent(q.ChangePct());
    case 4:
        return format::Fixed(q.bid);
    case 5:
        return format::Fixed(q.ask);
    case 6:
        return format::Thousands(static_cast<long long>(q.volume));
    default:
        return "";
    }
}

inline DataTable<Quote> MakeWatchlist(std::string name,
                                      theme::Polarity pol = theme::Polarity::GreenUp) {
    DataTable<Quote> t(std::move(name), {{"Symbol", 90.f},
                                         {"Last", 80.f},
                                         {"Chg", 90.f},
                                         {"Chg%", 80.f},
                                         {"Bid", 80.f},
                                         {"Ask", 80.f},
                                         {"Volume", 100.f}});
    t.SetCellFormatter([](int, int col, const Quote& q) { return QuoteCell(col, q); });
    t.SetCellColor([pol](int, int col, const Quote& q) -> ImU32 {
        if (col == 2 || col == 3)
            return DeltaColor(q.Change(), 0.0, pol);
        return 0;
    });
    t.SetFrozenColumns(1);
    return t;
}

} // namespace unigui::trading
