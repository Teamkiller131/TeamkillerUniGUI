// Performance benchmarks for the trading models under high update rates.
//
// These exercise the pure, header-only models (no ImGui frame / GL context
// needed) so they run reliably in headless CI. The thresholds are deliberately
// generous lower bounds — they guard against order-of-magnitude regressions
// (the Horizon-4 "trading DOM/blotters under high update rates" budget), not
// micro-fluctuations. Gated by UNIGUI_MODULE_TRADING.

#include <unigui/trading/ohlc_series.h>
#include <unigui/trading/order_book.h>

#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace std::chrono;
using unigui::trading::Level;
using unigui::trading::OhlcSeries;
using unigui::trading::OrderBook;

namespace {

template <typename Fn> long long TimeUs(Fn&& fn) {
    const auto t0 = high_resolution_clock::now();
    fn();
    return duration_cast<microseconds>(high_resolution_clock::now() - t0).count();
}

// Wall-clock budgets only assert in optimized builds; a Debug build is far
// slower and host-dependent, so the timing check is skipped there (the work
// still runs). See tests/bench/bench_test.cc for the same rationale.
inline void ExpectUnderBudget(long long actual, long long budget, const std::string& what) {
#ifdef NDEBUG
    EXPECT_LT(actual, budget) << what << ": " << actual << " (budget " << budget << ")";
#else
    (void) actual;
    (void) budget;
    (void) what;
#endif
}

} // namespace

// A realistic depth-of-market churn: a 50-level book updated with 200k price
// deltas (the DOM ladder reads the top N each frame). Should stay well under a
// budget that allows comfortable >60 fps streaming.
TEST(TradingBench, OrderBook_200kDeltas) {
    OrderBook book;
    // Seed a 50-level book on each side around a 100.00 mid.
    for (int i = 0; i < 50; ++i) {
        book.SetBid(100.00 - 0.01 * (i + 1), 100 + i);
        book.SetAsk(100.01 + 0.01 * i, 100 + i);
    }

    constexpr int kDeltas = 200000;
    std::int64_t sink = 0;
    const long long us = TimeUs([&] {
        for (int n = 0; n < kDeltas; ++n) {
            // Update a rotating level on alternating sides.
            const int lvl = n % 50;
            const std::int64_t size = 50 + (n % 500);
            if (n & 1)
                book.SetBid(100.00 - 0.01 * (lvl + 1), size);
            else
                book.SetAsk(100.01 + 0.01 * lvl, size);
            // Read the inside market as a renderer would.
            sink += book.BestBidSize() + book.BestAskSize();
        }
    });
    EXPECT_EQ(book.BidLevels(), 50u);
    EXPECT_EQ(book.AskLevels(), 50u);
    EXPECT_GT(sink, 0);
    // 200k delta+read cycles in well under a second (generous regression floor).
    EXPECT_LT(us, 1000000) << "OrderBook 200k deltas took " << us << "us";
}

// Snapshot rebuilds (full book replace) are the other hot path — 5k full
// 100-level snapshots.
TEST(TradingBench, OrderBook_5kSnapshots) {
    OrderBook book;
    std::vector<Level> bids(100), asks(100);
    const long long us = TimeUs([&] {
        for (int s = 0; s < 5000; ++s) {
            for (int i = 0; i < 100; ++i) {
                bids[i] = {100.00 - 0.01 * (i + 1), 100 + (s + i) % 900};
                asks[i] = {100.01 + 0.01 * i, 100 + (s + i) % 900};
            }
            book.ClearBids();
            book.ClearAsks();
            book.ApplyBidSnapshot(bids);
            book.ApplyAskSnapshot(asks);
        }
    });
    EXPECT_EQ(book.BidLevels(), 100u);
    EXPECT_LT(us, 1000000) << "OrderBook 5k snapshots took " << us << "us";
}

// Tick aggregation: 1M ticks folded into 1-minute bars with a rolling window.
TEST(TradingBench, OhlcSeries_1MTicks) {
    OhlcSeries series(60.0, /*maxBars=*/1024);
    constexpr int kTicks = 1000000;
    double t = 0.0;
    double price = 100.0;
    const long long us = TimeUs([&] {
        for (int n = 0; n < kTicks; ++n) {
            t += 0.5; // two ticks per second
            price += ((n % 7) - 3) * 0.01;
            series.AddTick(t, price, 1);
        }
    });
    // Rolling window caps the retained bars.
    EXPECT_LE(series.Size(), 1024u);
    EXPECT_FALSE(series.Empty());
    ExpectUnderBudget(us, 1000000, "OhlcSeries 1M ticks us");
}

// Column extraction (ImPlot feed) over a full window — done per frame while a
// chart is visible.
TEST(TradingBench, OhlcSeries_ColumnExtraction) {
    OhlcSeries series(60.0, /*maxBars=*/4096);
    double t = 0.0;
    for (int n = 0; n < 500000; ++n) {
        t += 1.0;
        series.AddTick(t, 100.0 + (n % 11) * 0.02, 1);
    }
    std::size_t total = 0;
    const long long us = TimeUs([&] {
        for (int frame = 0; frame < 1000; ++frame) {
            total += series.Closes().size() + series.Highs().size() + series.Lows().size() +
                     series.Opens().size() + series.Times().size();
        }
    });
    EXPECT_GT(total, 0u);
    ExpectUnderBudget(us, 1000000, "OhlcSeries 1k column extractions us");
}
