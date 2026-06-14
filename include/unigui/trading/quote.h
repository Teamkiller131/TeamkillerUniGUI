#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// Trading row/value types  (namespace unigui::trading)
//
// Plain, presentation-oriented data structures shared by the trading widgets
// (blotters, watchlist, order ticket). Pure data + trivial derived getters — no
// ImGui, no I/O, no feed logic. The embedder fills these from its own market
// data / OMS; the widgets only display them.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <string>

namespace unigui::trading {

enum class Side { Buy, Sell };

inline const char* SideName(Side s) {
    return s == Side::Buy ? "Buy" : "Sell";
}

/// Top-of-book quote for a symbol (watchlist / quote board row).
struct Quote {
    std::string symbol;
    double bid = 0.0;
    double ask = 0.0;
    double last = 0.0;
    double prevClose = 0.0;
    std::int64_t bidSize = 0;
    std::int64_t askSize = 0;
    std::int64_t volume = 0;

    double Mid() const { return (bid + ask) * 0.5; }
    double Spread() const { return ask - bid; }
    /// Absolute change vs. previous close.
    double Change() const { return last - prevClose; }
    /// Fractional change vs. previous close (e.g. 0.0125 = +1.25%); 0 if no base.
    double ChangePct() const { return prevClose != 0.0 ? (last - prevClose) / prevClose : 0.0; }
};

/// A net position in a symbol (positions blotter row).
struct Position {
    std::string symbol;
    std::int64_t qty = 0; // signed: > 0 long, < 0 short
    double avgPrice = 0.0;
    double last = 0.0;

    bool IsLong() const { return qty > 0; }
    bool IsShort() const { return qty < 0; }
    bool IsFlat() const { return qty == 0; }
    double MarketValue() const { return static_cast<double>(qty) * last; }
    double UnrealizedPnL() const { return static_cast<double>(qty) * (last - avgPrice); }
};

enum class OrderStatus { New, PartiallyFilled, Filled, Cancelled, Rejected };

inline const char* OrderStatusName(OrderStatus s) {
    switch (s) {
    case OrderStatus::New:
        return "New";
    case OrderStatus::PartiallyFilled:
        return "PartiallyFilled";
    case OrderStatus::Filled:
        return "Filled";
    case OrderStatus::Cancelled:
        return "Cancelled";
    case OrderStatus::Rejected:
        return "Rejected";
    }
    return "Unknown";
}

/// A working or completed order (orders blotter row).
struct Order {
    std::string id;
    std::string symbol;
    Side side = Side::Buy;
    double price = 0.0;
    std::int64_t qty = 0;
    std::int64_t filled = 0;
    OrderStatus status = OrderStatus::New;

    std::int64_t Remaining() const { return qty - filled; }
    bool IsDone() const {
        return status == OrderStatus::Filled || status == OrderStatus::Cancelled ||
               status == OrderStatus::Rejected;
    }
    /// Fraction filled in [0, 1]; 0 if qty is 0.
    double FillRatio() const {
        return qty != 0 ? static_cast<double>(filled) / static_cast<double>(qty) : 0.0;
    }
};

/// An executed trade / fill (time & sales tape row).
struct Trade {
    std::string id;
    std::string symbol;
    Side side = Side::Buy;
    double price = 0.0;
    std::int64_t qty = 0;
    double timestamp = 0.0; // epoch seconds

    double Notional() const { return price * static_cast<double>(qty); }
};

} // namespace unigui::trading
