# Trading Toolkit

UniGUI ships building blocks for data-dense, real-time trading UIs. Following the
project's "domain-ready" principle, these are **presentation + thin models**: the
market feed, order routing, and strategy live in your application — the toolkit
only formats and displays.

This guide covers what has shipped so far. The full roadmap (candlestick chart,
DOM ladder, order ticket, blotters/watchlist, example app) is tracked in
`DEVELOPMENT_PLAN.md` Horizon 3.

## Enabling

The models below are header-only and dependency-free, so they are usable without
any build flag. The forthcoming trading **widgets** (and the umbrella
`#include <unigui/unigui.h>` exposure of the trading headers) are gated by the
optional module:

```bash
cmake -B build -S . -DUNIGUI_MODULE_TRADING=ON ...
```

You can also include the headers directly regardless of the module:

```cpp
#include <unigui/core/format_num.h>
#include <unigui/trading/quote.h>
#include <unigui/trading/order_book.h>
#include <unigui/trading/ohlc_series.h>
```

## Numeric & financial formatting (`unigui::format`)

`<unigui/core/format_num.h>` — locale-neutral, pure formatting helpers. (For
Chinese 万/手 formatting see `<unigui/core/format_cn.h>`.)

```cpp
using namespace unigui::format;

Thousands(1234567);            // "1,234,567"
Fixed(1234567.891, 2);         // "1,234,567.89"
Currency(-1234.5, "$");        // "-$1,234.50"
Currency(50, "€", 0);          // "€50"
Percent(0.0425);               // "4.25%"   (input is a ratio)
Percent(4.25, 2, false);       // "4.25%"   (input already in percent units)
SignedDelta(1.5);              // "+1.50"
SignedDelta(-1.5);             // "-1.50"
TickAlign(100.123, 0.05);      // 100.10    (snap to tick size)

// Colour P&L / change cells at the call site (header stays ImGui-free):
switch (Sign(pnl)) {
case Direction::Up:   /* green */ break;
case Direction::Down: /* red   */ break;
case Direction::Flat: /* muted */ break;
}
```

## Row / value types (`unigui::trading`)

`<unigui/trading/quote.h>` — plain structs for blotter/watchlist rows, each with
trivial derived getters:

| Type | Key fields | Derived getters |
|------|-----------|-----------------|
| `Quote` | symbol, bid/ask/last, prevClose, sizes, volume | `Mid()`, `Spread()`, `Change()`, `ChangePct()` |
| `Position` | symbol, `qty` (signed), avgPrice, last | `IsLong/Short/Flat()`, `MarketValue()`, `UnrealizedPnL()` |
| `Order` | id, symbol, side, price, qty, filled, status | `Remaining()`, `FillRatio()`, `IsDone()` |
| `Trade` | id, symbol, side, price, qty, timestamp | `Notional()` |

```cpp
unigui::trading::Position p{"AAPL", -100, /*avg*/190.0, /*last*/185.0};
p.IsShort();            // true
p.UnrealizedPnL();      // 500.0  (short, price fell)
```

## Depth-of-market model (`OrderBook`)

`<unigui/trading/order_book.h>` — aggregated size per price level, bids high→low
and asks low→high. Feed it snapshots or per-level deltas; query the top of book
and the levels a DOM ladder needs.

```cpp
unigui::trading::OrderBook ob;
ob.ApplyBidSnapshot({{100.0, 3}, {99.0, 5}});
ob.ApplyAskSnapshot({{101.0, 4}, {102.0, 9}});
ob.SetBid(100.5, 8);        // delta update (size 0 removes a level)

ob.BestBid();               // 100.5
ob.Spread();                // ask - bid
ob.Mid();                   // midpoint
auto top5 = ob.Bids(5);     // top 5 bid levels, high→low
ob.MaxSize(10);             // largest size in the top 10 — scale depth bars by this
```

## Candlestick aggregation (`OhlcSeries`)

`<unigui/trading/ohlc_series.h>` — folds ticks into fixed-interval OHLC bars and
keeps an optional rolling window. Column extractors return the contiguous arrays
ImPlot's candlestick path expects.

```cpp
unigui::trading::OhlcSeries series(/*interval*/60.0, /*maxBars*/500);
series.AddTick(t, price, volume);   // aggregates into the current/next bar
series.AddBar(historicalBar);       // backfill

series.Size();
const auto& last = series.Back();   // { time, open, high, low, close, volume }
last.Bullish();

// For charting:
auto t = series.Times();
auto o = series.Opens();
auto h = series.Highs();
auto l = series.Lows();
auto c = series.Closes();
```

## Streaming from a feed thread

Update models from a market-data/WebSocket thread, then marshal UI mutations back
to the render thread with the main-thread dispatcher
(`<unigui/core/main_thread.h>`):

```cpp
// feed thread:
ws.OnMessage([&](const std::string& msg){
    auto tick = parse(msg);
    unigui::InvokeOnMainThread([&, tick]{ series.AddTick(tick.t, tick.px, tick.qty); });
});

// render loop:
unigui::NewFrame();
unigui::ProcessMainThreadTasks();   // apply queued model updates
// ... render widgets bound to the models ...
unigui::Render();
```

The optional `network/` module (`<unigui/network/network.h>`) provides HTTP +
WebSocket clients you can pair with this pattern.
