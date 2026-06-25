// Reactive wiring for the trading value types. Now that Quote/Position/Order/
// Trade are equality-comparable, they compose directly with the core reactive
// layer: Observable<T> change-detection, Computed<T> derivations, and Bind.

#include <unigui/core/observable.h>
#include <unigui/trading/quote.h>

#include <gtest/gtest.h>
#include <string>

using unigui::Bind;
using unigui::Computed;
using unigui::Observable;
using namespace unigui::trading;

TEST(TradingReactiveTest, QuoteObservableNotifiesOnlyOnChange) {
    Observable<Quote> q{
        Quote{.symbol = "AAPL", .bid = 100.0, .ask = 100.1, .last = 100.0, .prevClose = 99.0}};
    int notifications = 0;
    auto sub = q.Subscribe([&](const Quote&) { ++notifications; });

    // Re-setting an identical quote is a no-op (value equality).
    EXPECT_FALSE(q.Set(q.Get()));
    EXPECT_EQ(notifications, 0);

    // A changed field notifies exactly once.
    Quote moved = q.Get();
    moved.last = 101.0;
    EXPECT_TRUE(q.Set(moved));
    EXPECT_EQ(notifications, 1);
}

TEST(TradingReactiveTest, ComputedDerivesLiveQuoteMetrics) {
    Observable<Quote> q{Quote{.bid = 10.0, .ask = 10.2, .last = 10.0, .prevClose = 10.0}};
    Computed<double> mid{[](const Quote& x) { return x.Mid(); }, q};
    Computed<double> changePct{[](const Quote& x) { return x.ChangePct(); }, q};
    EXPECT_NEAR(mid.Get(), 10.1, 1e-9);
    EXPECT_NEAR(changePct.Get(), 0.0, 1e-9);

    Quote up = q.Get();
    up.bid = 10.4;
    up.ask = 10.6;
    up.last = 10.5;
    q.Set(up);
    EXPECT_NEAR(mid.Get(), 10.5, 1e-9);
    EXPECT_NEAR(changePct.Get(), 0.05, 1e-9); // (10.5 - 10.0) / 10.0
}

TEST(TradingReactiveTest, MutateUpdatesQuoteFieldInPlaceAndNotifies) {
    Observable<Quote> q{Quote{.symbol = "MSFT", .last = 400.0, .prevClose = 400.0}};
    double seenChange = -1.0;
    auto sub = q.Subscribe([&](const Quote& x) { seenChange = x.Change(); });
    q.Mutate([](Quote& x) { x.last = 410.0; });
    EXPECT_NEAR(seenChange, 10.0, 1e-9);
}

TEST(TradingReactiveTest, BindComputedQuoteTagToSink) {
    Observable<Quote> q{Quote{.symbol = "NVDA", .last = 120.0, .prevClose = 120.0}};
    Computed<std::string> tag{
        [](const Quote& x) { return x.symbol + (x.Change() >= 0 ? " +" : " -"); }, q};
    std::string sink;
    auto sub = Bind(tag, [&](const std::string& s) { sink = s; });
    EXPECT_EQ(sink, "NVDA +"); // immediate
    q.Mutate([](Quote& x) { x.last = 118.0; });
    EXPECT_EQ(sink, "NVDA -"); // recomputed and delivered
}

TEST(TradingReactiveTest, PositionOrderTradeAreEqualityComparable) {
    Position p1{.symbol = "X", .qty = 10, .avgPrice = 5.0, .last = 5.5};
    Position p2 = p1;
    EXPECT_EQ(p1, p2);
    p2.last = 6.0;
    EXPECT_NE(p1, p2);

    Order o1{.id = "1", .symbol = "X", .side = Side::Buy, .price = 5.0, .qty = 10};
    Order o2 = o1;
    EXPECT_EQ(o1, o2);
    o2.status = OrderStatus::Filled;
    EXPECT_NE(o1, o2);

    Trade t1{.id = "t", .symbol = "X", .side = Side::Sell, .price = 5.0, .qty = 3};
    Trade t2 = t1;
    EXPECT_EQ(t1, t2);
    t2.qty = 4;
    EXPECT_NE(t1, t2);
}
