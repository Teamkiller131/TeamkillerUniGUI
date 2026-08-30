# Trading Toolkit

UniGUI ships building blocks for data-dense, real-time trading UIs. Following the
project's "domain-ready" principle, these are **presentation + thin models**: the
market feed, order routing, and strategy live in your application — the toolkit
only formats and displays.

This guide covers the shipped trading toolkit: the formatting helpers, the
in-memory models, the **candlestick chart**, the **depth-of-market ladder**, the
**order ticket**, the **blotter / watchlist / tape** templates, and a runnable
**`trading_dashboard`** example that assembles them. This completes Horizon 3 of
`DEVELOPMENT_PLAN.md`.

## Enabling

The models below are header-only and dependency-free, so they are usable without
any build flag. The trading **widgets** (e.g. `CandlestickChart`) and the umbrella
`#include <unigui/unigui.h>` exposure of the trading headers are gated by the
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

## Candlestick chart (`CandlestickChart`)

`<unigui/trading/candlestick_chart.h>` (module-gated) — a retained-mode widget
that draws an `OhlcSeries` as candlesticks via ImPlot. It is presentation-only
and **non-owning**: keep feeding the model, and the chart renders whatever it
currently holds.

```cpp
unigui::trading::OhlcSeries series(60.0, 500);
unigui::trading::CandlestickChart chart("price");
chart.WithSeries(&series)
     .WithVolumePanel(true)      // volume bars in a linked-X sub-panel
     .WithCandleWidth(0.6f)      // body width as a fraction of the bar interval
     .WithTimeAxis(true)         // format the X axis as date/time
     .WithCrosshair(true);

// in the render loop, inside a window:
chart.Render();
```

Features: bull/bear candle colours (`SetBullColor`/`SetBearColor`), an optional
volume sub-panel with direction-coloured bars (`SetVolumePanel` /
`SetVolumePanelRatio`), an OHLCV hover tooltip (`SetHoverTooltip`, formatted via
`core/format_num.h`), theme-aware background (`SetThemeBackground`), axis labels,
and the standard fluent `With*` chain.

For full control you can drive ImPlot yourself and call the low-level renderer
between `BeginPlot`/`EndPlot`:

```cpp
if (ImPlot::BeginPlot("chart")) {
    unigui::trading::PlotCandlesticks(
        "OHLC", t.data(), o.data(), c.data(), l.data(), h.data(), (int)t.size(),
        /*halfWidth*/ series.Interval() * 0.25,
        IM_COL32(38,166,91,255), IM_COL32(217,60,60,255));
    ImPlot::EndPlot();
}
```

## Depth-of-market ladder (`DepthLadder`)

`<unigui/trading/depth_ladder.h>` (module-gated) — a retained-mode widget that
draws an `OrderBook` as a classic vertical price ladder: asks stacked
highest→lowest on top, an optional spread/mid divider row, then bids
highest→lowest below. Each level draws a horizontal depth bar (scaled to the
largest visible size), the aggregated size, and the price. Presentation-only and
**non-owning** — keep feeding the book, and the ladder renders what it holds.

```cpp
unigui::trading::OrderBook book;
unigui::trading::DepthLadder dom("dom");
dom.WithBook(&book)
   .WithDepth(10)              // top 10 levels per side (0 = all)
   .WithAutoCenter(true)       // keep the inside market vertically centred
   .WithPriceDecimals(2)
   .WithOnLevelClick([](auto side, double price, std::int64_t size) {
       // click-to-trade: route an order at this level
       bool buy = side == unigui::trading::DepthLadder::Side::Ask; // lift the offer
       (void)buy; (void)price; (void)size;
   });

// in the render loop, inside a window:
dom.Render();
```

Features: bid/ask colours (`SetBidColor`/`SetAskColor`), translucent depth bars
(`SetBarOpacity`), a spread/mid divider (`SetShowSpreadRow`), per-side
click-to-trade callback (`SetOnLevelClick`), theme-aware background
(`SetThemeBackground`), configurable row/column geometry (`SetRowHeight`,
`SetSizeColumnWidth`, `SetPriceColumnWidth`), depth limiting (`SetDepth`),
auto-/one-shot centring on the inside market (`SetAutoCenter` /
`CenterOnMarket()`), and the standard fluent `With*` chain. Scale the depth bars
off `OrderBook::MaxSize(depth)`, which the widget queries for you.

## Order ticket (`OrderTicket`)

`<unigui/trading/order_ticket.h>` (module-gated) — an order-entry form over a
single editable `OrderDraft` (symbol, side, order type, time-in-force, quantity,
limit/stop price). Validation is **pure and headless-testable**; submission snaps
prices to the tick size and hands the validated draft to your OMS via a callback.
The widget never routes orders itself.

```cpp
unigui::trading::OrderTicket ticket("ticket");
ticket.WithSymbol("AAPL")
      .WithOrderType(unigui::trading::OrderType::Limit)
      .WithTimeInForce(unigui::trading::TimeInForce::Day)
      .WithTickSize(0.01)
      .WithMaxQuantity(10'000)
      .WithConfirm(true)               // require a confirm modal before submit
      .WithOnSubmit([](const unigui::trading::OrderDraft& o) {
          // route o to your OMS — already validated and tick-snapped
      });

// in the render loop, inside a window:
ticket.Render();
```

`OrderType` is `Market` / `Limit` / `Stop` / `StopLimit`; `TimeInForce` is `Day` /
`GTC` / `IOC` / `FOK` (both have `*Name()` helpers). `OrderDraft::NeedsPrice()` /
`NeedsStop()` decide which inputs are required and which are enabled. Drive it
headlessly without rendering:

```cpp
ticket.SetSymbol("MSFT");
ticket.SetQuantity(250);
ticket.SetPrice(412.34);
if (auto v = ticket.Validate(); !v.ok)
    log(v.message);     // e.g. "Limit price must be greater than 0"
ticket.Submit();        // validates, tick-snaps, fires the callback if valid
```

UI niceties: a colour-coded Buy/Sell toggle, type/TIF combos, price/stop inputs
that grey out when the order type doesn't use them, an inline validation message,
a submit button disabled until the draft is valid, an optional confirmation modal
(`SetConfirm`), and a **Ctrl+Enter** submit hotkey (reuses `ShortcutManager`).

## Blotters, watchlist & tape (`blotters.h`)

`<unigui/trading/blotters.h>` (module-gated, header-only) ships pre-built
`DataTable<T>` configurations over the row models, so a positions blotter or
quote board is one call plus a data binding:

```cpp
std::vector<unigui::trading::Position> positions = /* from your OMS */;

auto blotter = unigui::trading::MakePositionsBlotter("positions");
blotter.SetDataSource(&positions);   // zero-copy: table references your vector

// in the render loop, inside a window:
blotter.Render();
```

Factories: `MakePositionsBlotter` (`Position`), `MakeOrdersBlotter` (`Order`),
`MakeTradesTape` (`Trade` — time & sales), `MakeWatchlist` (`Quote` — quote
board). Each wires the columns, a financial cell formatter (via
`core/format_num.h`), sign-aware cell colours (green/red with ▲/▼ delta arrows),
and a **pinned leading column** so the symbol/key stays visible as the rest
scroll horizontally.

The cell formatters and colour/format helpers are plain functions you can reuse
or test on their own — no ImGui frame needed:

```cpp
using namespace unigui::trading;
PositionCell(5, pos);          // uPnL column, e.g. "+500.00"
QuoteCell(3, quote);           // change %, e.g. "20.00%"
FormatClock(3661.0);           // "01:01:01" (UTC HH:MM:SS)
DeltaColor(pnl);               // green / red / 0 (= no override)
WithArrow(chg, "1.25");        // "▲ 1.25" / "▼ 1.25" / "1.25"
```

The factories returned are ordinary `DataTable<T>` widgets, so the full table
API still applies — sorting, virtual scroll, `FlashRow`, row/cell colour, context
menus, selection callbacks, and the new freeze-pane:

```cpp
blotter.SetVirtualScroll(true);
blotter.SetStickyHeader(true);
blotter.SetFrozenColumns(2);   // pin the first two columns (freeze-pane)
blotter.SetOnSelect([](int row){ /* ... */ });
```

### In-cell mini renderers (`cell_renderers.h`)

`<unigui/trading/cell_renderers.h>` (header-only) is the custom-draw half of the
"mini sparkline/bar in a blotter cell" item: `DataTable::SetCellRenderer` shipped
the per-cell hook, and these are the batteries built on it. Each factory returns a
`CellRenderFn` that draws inside the cell rect and reserves its height:

```cpp
#include <unigui/trading/cell_renderers.h>

using namespace unigui::trading;

blotter.SetCellRenderer(/*col*/ 2, SparklineCell<Position>([](int row, const Position& p) {
    return pnlHistory[row];                 // std::vector<float> per row
}));

blotter.SetCellRenderer(/*col*/ 3, BarCell<Position>(
    [](int row, const Position& p) { return p.UnrealizedPnL(); }, 1000.0));
```

- `SparklineCell<T>(valuesOf, width = 64, height = 16, colorOf = nullptr)` — a
  1.2 px polyline normalized to the row's own min/max; default colour is the
  active theme's `PlotLines` token. Rows with fewer than two values only reserve
  the space.
- `BarCell<T>(valueOf, maxAbs, width = 64, height = 12, colorOf = nullptr, pol)`
  — a signed horizontal bar mapped into [−maxAbs, +maxAbs] from the cell centre;
  default colour is sign-aware `DeltaColor` (theme polarity). Flat rows draw
  nothing (the height is still reserved).

Both are theme-aware, per-frame allocation-light, and covered by headless
geometry tests (`tests/trading/cell_renderers_test.cc`).

## Price ticker (`PriceTicker`)

The scrolling quote marquee is the general widget `unigui::PriceTicker`
(`<unigui/widgets/priceticker.h>` — not trading-gated): one line that wraps
seamlessly through `SYMBOL price ▲/▼` items, sign-tinted, with speed/pause
controls:

```cpp
#include <unigui/widgets/priceticker.h>

unigui::PriceTicker ticker("ticker");
for (const auto& q : quotes)                 // std::vector<trading::Quote> from your feed
    ticker.AddItem({q.symbol, format::Fixed(q.last, 2), (float) q.ChangePct() * 100.f});
ticker.SetSpeed(80.0f);                      // px/s
ticker.SetPaused(focused_other_ui);          // pause on demand

// in the render loop:
ticker.Render();
```

`Item{ symbol, price, change }` is plain data — the `change` sign drives the
colour + arrow (theme green/red by default, overridable with `SetUpColor` /
`SetDownColor`), the price is an already-formatted string, so the ticker works
for any feed, not just the trading models.

## Putting it together: `examples/trading_dashboard`

`examples/trading_dashboard` assembles the whole toolkit into one screen — the
candlestick chart, the depth-of-market ladder, the order ticket, and the
positions / orders / watchlist / time-&-sales blotters — driven by small
in-memory models with synthetic, self-animating data (a deterministic
pseudo-random walk stands in for both the market feed and the OMS). The order
ticket's submit callback appends to the orders blotter and the tape, showing how
the presentation widgets and thin models wire to an embedder's flow.

It builds when both `UNIGUI_MODULE_WIDGETS` and `UNIGUI_MODULE_TRADING` are on,
and runs headless for screenshots / CI smoke:

```bash
./build/examples/trading_dashboard/trading_dashboard --frames 10
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

## Reactive models (Observable / Computed)

The trading value types (`Quote`, `Position`, `Order`, `Trade`) are value-equality
comparable, so they plug straight into the reactive layer
(`<unigui/core/observable.h>`) — no extra glue. Wrap a row in an `Observable<T>`
for change-detected updates, and derive live metrics with `Computed<T>`:

```cpp
using namespace unigui;
using namespace unigui::trading;

Observable<Quote> quote{ Quote{.symbol="AAPL", .bid=190.0, .ask=190.1, .last=190.0,
                               .prevClose=188.0} };

// Derived, recomputed automatically whenever `quote` changes:
Computed<double> mid      { [](const Quote& q){ return q.Mid(); },       quote };
Computed<double> changePct{ [](const Quote& q){ return q.ChangePct(); }, quote };

// Push a feed update — observers fire only if a field actually changed:
quote.Mutate([](Quote& q){ q.last = 191.25; });   // mid & changePct recompute

// Bind a derived value to a widget or any sink:
Label tag{"px_tag"};
auto sub = Bind(changePct, [&](double pct){
    tag.SetText(pct >= 0 ? "▲" : "▼");
});
```

Value widgets bind the same way via `ValueWidget<T>::BindValue` (two-way) and
`Label::BindText` (one-way) — see [`core/observable.h`](../include/unigui/core/observable.h).
Keep updating the models from the feed thread and marshal to the render thread as
above; the reactive notifications run wherever you call `Set`/`Mutate`.
