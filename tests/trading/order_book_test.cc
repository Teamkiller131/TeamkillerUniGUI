#include <unigui/trading/order_book.h>

#include <gtest/gtest.h>

using namespace unigui::trading;

TEST(OrderBook, EmptyDefaults) {
    OrderBook ob;
    EXPECT_FALSE(ob.HasBids());
    EXPECT_FALSE(ob.HasAsks());
    EXPECT_EQ(ob.BestBid(), 0.0);
    EXPECT_EQ(ob.BestAsk(), 0.0);
    EXPECT_EQ(ob.Spread(), 0.0);
    EXPECT_EQ(ob.Mid(), 0.0);
}

TEST(OrderBook, OrderingAndBest) {
    OrderBook ob;
    ob.SetBid(99.0, 5);
    ob.SetBid(98.0, 7);
    ob.SetBid(100.0, 3);
    ob.SetAsk(101.0, 4);
    ob.SetAsk(102.0, 9);

    EXPECT_EQ(ob.BestBid(), 100.0);
    EXPECT_EQ(ob.BestBidSize(), 3);
    EXPECT_EQ(ob.BestAsk(), 101.0);
    EXPECT_EQ(ob.BestAskSize(), 4);
    EXPECT_NEAR(ob.Spread(), 1.0, 1e-9);
    EXPECT_NEAR(ob.Mid(), 100.5, 1e-9);

    auto bids = ob.Bids();
    ASSERT_EQ(bids.size(), 3u);
    EXPECT_EQ(bids.front().price, 100.0); // high → low
    EXPECT_EQ(bids.back().price, 98.0);

    auto asks = ob.Asks();
    ASSERT_EQ(asks.size(), 2u);
    EXPECT_EQ(asks.front().price, 101.0); // low → high
}

TEST(OrderBook, DepthLimitAndMaxSize) {
    OrderBook ob;
    ob.SetBid(99.0, 5);
    ob.SetBid(98.0, 7);
    ob.SetBid(97.0, 2);
    ob.SetAsk(100.0, 9);

    EXPECT_EQ(ob.Bids(2).size(), 2u);
    EXPECT_EQ(ob.Bids(2).back().price, 98.0);
    EXPECT_EQ(ob.MaxSize(), 9);
    EXPECT_EQ(ob.MaxSize(1), 9); // best ask 9 still largest among tops
}

TEST(OrderBook, SetZeroRemovesLevel) {
    OrderBook ob;
    ob.SetBid(100.0, 3);
    ob.SetBid(99.0, 5);
    EXPECT_EQ(ob.BestBid(), 100.0);
    ob.SetBid(100.0, 0); // remove
    EXPECT_EQ(ob.BestBid(), 99.0);
    EXPECT_EQ(ob.BidLevels(), 1u);
}

TEST(OrderBook, Snapshots) {
    OrderBook ob;
    ob.SetBid(1.0, 1);
    ob.ApplyBidSnapshot({{50.0, 10}, {49.0, 20}});
    ob.ApplyAskSnapshot({{51.0, 5}});
    EXPECT_EQ(ob.BidLevels(), 2u);
    EXPECT_EQ(ob.BestBid(), 50.0);
    EXPECT_EQ(ob.BestAsk(), 51.0);
}
