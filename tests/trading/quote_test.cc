#include <unigui/trading/quote.h>

#include <gtest/gtest.h>

using namespace unigui::trading;

TEST(Quote, DerivedFields) {
    Quote q;
    q.bid = 10.0;
    q.ask = 10.5;
    q.last = 10.2;
    q.prevClose = 10.0;
    EXPECT_NEAR(q.Mid(), 10.25, 1e-9);
    EXPECT_NEAR(q.Spread(), 0.5, 1e-9);
    EXPECT_NEAR(q.Change(), 0.2, 1e-9);
    EXPECT_NEAR(q.ChangePct(), 0.02, 1e-9);
}

TEST(Quote, ChangePctZeroBaseSafe) {
    Quote q;
    q.last = 10.0;
    q.prevClose = 0.0;
    EXPECT_EQ(q.ChangePct(), 0.0);
}

TEST(Position, LongShortAndPnL) {
    Position lp{"AAPL", 100, 185.0, 190.0};
    EXPECT_TRUE(lp.IsLong());
    EXPECT_NEAR(lp.UnrealizedPnL(), 500.0, 1e-9);
    EXPECT_NEAR(lp.MarketValue(), 19000.0, 1e-9);

    Position sp{"AAPL", -100, 190.0, 185.0};
    EXPECT_TRUE(sp.IsShort());
    EXPECT_NEAR(sp.UnrealizedPnL(), 500.0, 1e-9);

    Position flat;
    EXPECT_TRUE(flat.IsFlat());
}

TEST(Order, RemainingFillAndDone) {
    Order o{"1", "AAPL", Side::Buy, 100.0, 100, 40, OrderStatus::PartiallyFilled};
    EXPECT_EQ(o.Remaining(), 60);
    EXPECT_NEAR(o.FillRatio(), 0.4, 1e-9);
    EXPECT_FALSE(o.IsDone());

    o.status = OrderStatus::Filled;
    o.filled = 100;
    EXPECT_TRUE(o.IsDone());
    EXPECT_EQ(o.Remaining(), 0);
}

TEST(Trade, Notional) {
    Trade t{"t1", "AAPL", Side::Sell, 50.0, 10, 0.0};
    EXPECT_NEAR(t.Notional(), 500.0, 1e-9);
    EXPECT_STREQ(SideName(t.side), "Sell");
}
