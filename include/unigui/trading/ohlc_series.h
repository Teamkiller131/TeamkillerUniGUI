#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// OhlcSeries — rolling candlestick aggregator  (namespace unigui::trading)
//
// Aggregates incoming ticks into fixed-interval OHLC bars (open/high/low/close
// + volume) and keeps an optional rolling window of the most recent bars, so a
// candlestick/OHLC chart widget can render them. Pure model: no ImGui, no I/O.
// Column extractors (Opens()/Highs()/…) return the contiguous arrays ImPlot's
// candlestick path expects.
// ─────────────────────────────────────────────────────────────────────────────

#include <cmath>
#include <cstdint>
#include <deque>
#include <vector>

namespace unigui::trading {

struct Bar {
    double time = 0.0; // bar open time (epoch seconds, bucket-aligned)
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    std::int64_t volume = 0;

    bool Bullish() const { return close >= open; }
};

class OhlcSeries {
public:
    explicit OhlcSeries(double intervalSeconds = 60.0, std::size_t maxBars = 0)
            : interval_(intervalSeconds > 0 ? intervalSeconds : 60.0)
            , maxBars_(maxBars) {}

    void SetInterval(double seconds) {
        if (seconds > 0)
            interval_ = seconds;
    }
    double Interval() const { return interval_; }

    /// 0 = unbounded. Trims oldest bars immediately if the new cap is smaller.
    void SetMaxBars(std::size_t n) {
        maxBars_ = n;
        Trim();
    }

    /// Fold a tick into the current bar, or open a new bar when it crosses an
    /// interval boundary. Ticks older than the current bar's bucket are ignored.
    void AddTick(double time, double price, std::int64_t volume = 0) {
        const double bucket = std::floor(time / interval_) * interval_;
        if (bars_.empty() || bucket > bars_.back().time) {
            Bar b;
            b.time = bucket;
            b.open = b.high = b.low = b.close = price;
            b.volume = volume;
            bars_.push_back(b);
            Trim();
        } else if (bucket == bars_.back().time) {
            Bar& b = bars_.back();
            if (price > b.high)
                b.high = price;
            if (price < b.low)
                b.low = price;
            b.close = price;
            b.volume += volume;
        }
        // else: late/out-of-order tick before the current bucket — ignored.
    }

    /// Append a fully-formed bar (e.g. historical backfill). Caller is
    /// responsible for ordering.
    void AddBar(const Bar& bar) {
        bars_.push_back(bar);
        Trim();
    }

    void Clear() { bars_.clear(); }
    bool Empty() const { return bars_.empty(); }
    std::size_t Size() const { return bars_.size(); }

    const Bar& At(std::size_t i) const { return bars_[i]; }
    const Bar& Back() const { return bars_.back(); }

    std::vector<Bar> Bars() const { return {bars_.begin(), bars_.end()}; }

    std::vector<double> Times() const { return Column(&Bar::time); }
    std::vector<double> Opens() const { return Column(&Bar::open); }
    std::vector<double> Highs() const { return Column(&Bar::high); }
    std::vector<double> Lows() const { return Column(&Bar::low); }
    std::vector<double> Closes() const { return Column(&Bar::close); }
    std::vector<double> Volumes() const {
        std::vector<double> out;
        out.reserve(bars_.size());
        for (const auto& b : bars_)
            out.push_back(static_cast<double>(b.volume));
        return out;
    }

private:
    std::vector<double> Column(double Bar::*field) const {
        std::vector<double> out;
        out.reserve(bars_.size());
        for (const auto& b : bars_)
            out.push_back(b.*field);
        return out;
    }
    void Trim() {
        if (maxBars_ == 0)
            return;
        while (bars_.size() > maxBars_)
            bars_.pop_front();
    }

    double interval_;
    std::size_t maxBars_;
    std::deque<Bar> bars_;
};

} // namespace unigui::trading
