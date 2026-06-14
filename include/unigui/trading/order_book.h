#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// OrderBook — a thin depth-of-market model  (namespace unigui::trading)
//
// Maintains aggregated bid/ask size per price level so a depth ladder (DOM)
// widget can render it. Pure data structure: apply snapshots/deltas from the
// embedder's feed; query best bid/ask, spread, mid, and the top-N levels.
// Bids are ordered high→low, asks low→high. No ImGui, no I/O.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

namespace unigui::trading {

struct Level {
    double price = 0.0;
    std::int64_t size = 0;
};

class OrderBook {
public:
    /// Set the aggregated size at a bid price. A size <= 0 removes the level.
    void SetBid(double price, std::int64_t size) {
        if (size <= 0)
            bids_.erase(price);
        else
            bids_[price] = size;
    }
    /// Set the aggregated size at an ask price. A size <= 0 removes the level.
    void SetAsk(double price, std::int64_t size) {
        if (size <= 0)
            asks_.erase(price);
        else
            asks_[price] = size;
    }

    /// Replace the entire bid side with a snapshot.
    void ApplyBidSnapshot(const std::vector<Level>& levels) {
        bids_.clear();
        for (const auto& l : levels)
            SetBid(l.price, l.size);
    }
    /// Replace the entire ask side with a snapshot.
    void ApplyAskSnapshot(const std::vector<Level>& levels) {
        asks_.clear();
        for (const auto& l : levels)
            SetAsk(l.price, l.size);
    }

    void ClearBids() { bids_.clear(); }
    void ClearAsks() { asks_.clear(); }
    void Clear() {
        bids_.clear();
        asks_.clear();
    }

    bool HasBids() const { return !bids_.empty(); }
    bool HasAsks() const { return !asks_.empty(); }
    std::size_t BidLevels() const { return bids_.size(); }
    std::size_t AskLevels() const { return asks_.size(); }

    /// Best (highest) bid price, or 0 if none.
    double BestBid() const { return bids_.empty() ? 0.0 : bids_.begin()->first; }
    /// Best (lowest) ask price, or 0 if none.
    double BestAsk() const { return asks_.empty() ? 0.0 : asks_.begin()->first; }
    std::int64_t BestBidSize() const { return bids_.empty() ? 0 : bids_.begin()->second; }
    std::int64_t BestAskSize() const { return asks_.empty() ? 0 : asks_.begin()->second; }

    /// ask - bid; 0 if either side is empty.
    double Spread() const { return (bids_.empty() || asks_.empty()) ? 0.0 : BestAsk() - BestBid(); }
    /// Mid price; 0 if either side is empty.
    double Mid() const {
        return (bids_.empty() || asks_.empty()) ? 0.0 : (BestBid() + BestAsk()) * 0.5;
    }

    /// Bid levels high→low. `depth` 0 returns all.
    std::vector<Level> Bids(int depth = 0) const {
        std::vector<Level> out;
        for (const auto& [price, size] : bids_) {
            out.push_back({price, size});
            if (depth > 0 && static_cast<int>(out.size()) >= depth)
                break;
        }
        return out;
    }
    /// Ask levels low→high. `depth` 0 returns all.
    std::vector<Level> Asks(int depth = 0) const {
        std::vector<Level> out;
        for (const auto& [price, size] : asks_) {
            out.push_back({price, size});
            if (depth > 0 && static_cast<int>(out.size()) >= depth)
                break;
        }
        return out;
    }

    /// Largest size across the visible top-N levels of both sides — handy for
    /// scaling depth bars in a DOM widget. `depth` 0 considers all levels.
    std::int64_t MaxSize(int depth = 0) const {
        std::int64_t m = 0;
        for (const auto& l : Bids(depth))
            m = l.size > m ? l.size : m;
        for (const auto& l : Asks(depth))
            m = l.size > m ? l.size : m;
        return m;
    }

private:
    std::map<double, std::int64_t, std::greater<double>> bids_; // high → low
    std::map<double, std::int64_t> asks_;                       // low → high
};

} // namespace unigui::trading
