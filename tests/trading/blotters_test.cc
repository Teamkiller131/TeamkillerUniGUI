#include <unigui/trading/blotters.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <vector>

using namespace unigui::trading;

// Pure formatter / colour helpers need no ImGui frame; the factory render
// smoke tests do, so set up a headless frame for the whole suite.
class BlottersTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1280, 800);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

// ── Pure helpers ─────────────────────────────────────────────────────────────

TEST_F(BlottersTest, DeltaColor_SignAware) {
    EXPECT_NE(DeltaColor(1.0), 0u);  // up → coloured
    EXPECT_NE(DeltaColor(-1.0), 0u); // down → coloured
    EXPECT_EQ(DeltaColor(0.0), 0u);  // flat → no override
    EXPECT_NE(DeltaColor(1.0), DeltaColor(-1.0));
}

TEST_F(BlottersTest, SideColor_DistinguishesSides) {
    EXPECT_NE(SideColor(Side::Buy), SideColor(Side::Sell));
}

TEST_F(BlottersTest, WithArrow_PrefixesGlyph) {
    EXPECT_EQ(WithArrow(0.0, "0.00"), "0.00");  // flat → unchanged
    EXPECT_NE(WithArrow(1.0, "1.00"), "1.00");  // up → prefixed
    EXPECT_NE(WithArrow(-1.0, "1.00"), "1.00"); // down → prefixed
    EXPECT_NE(WithArrow(1.0, "1.00"), WithArrow(-1.0, "1.00"));
}

TEST_F(BlottersTest, FormatClock_UtcHhMmSs) {
    EXPECT_EQ(FormatClock(0.0), "00:00:00");     // epoch
    EXPECT_EQ(FormatClock(3661.0), "01:01:01");  // 1h 1m 1s
    EXPECT_EQ(FormatClock(86399.0), "23:59:59"); // last second of the day
}

// ── Cell formatters ──────────────────────────────────────────────────────────

TEST_F(BlottersTest, PositionCell_Columns) {
    Position p{"AAPL", -100, /*avg*/ 190.0, /*last*/ 185.0};
    EXPECT_EQ(PositionCell(0, p), "AAPL");
    EXPECT_EQ(PositionCell(1, p), "-100");
    EXPECT_EQ(PositionCell(2, p), "190.00");
    EXPECT_EQ(PositionCell(3, p), "185.00");
    // uPnL = qty*(last-avg) = -100*(-5) = +500
    EXPECT_EQ(PositionCell(5, p), "+500.00");
}

TEST_F(BlottersTest, OrderCell_Columns) {
    Order o{"O1", "MSFT", Side::Sell, 412.5, 250, 100, OrderStatus::PartiallyFilled};
    EXPECT_EQ(OrderCell(0, o), "O1");
    EXPECT_EQ(OrderCell(1, o), "MSFT");
    EXPECT_EQ(OrderCell(2, o), "Sell");
    EXPECT_EQ(OrderCell(3, o), "412.50");
    EXPECT_EQ(OrderCell(4, o), "250");
    EXPECT_EQ(OrderCell(5, o), "100");
    EXPECT_EQ(OrderCell(6, o), "PartiallyFilled");
}

TEST_F(BlottersTest, TradeCell_Columns) {
    Trade tr{"T1", "AAPL", Side::Buy, 100.25, 10, /*ts*/ 3661.0};
    EXPECT_EQ(TradeCell(0, tr), "01:01:01");
    EXPECT_EQ(TradeCell(1, tr), "AAPL");
    EXPECT_EQ(TradeCell(2, tr), "Buy");
    EXPECT_EQ(TradeCell(3, tr), "100.25");
    EXPECT_EQ(TradeCell(4, tr), "10");
    EXPECT_EQ(TradeCell(5, tr), "1,002.50"); // notional = 100.25 * 10
}

TEST_F(BlottersTest, QuoteCell_Columns) {
    Quote q;
    q.symbol = "NVDA";
    q.last = 120.0;
    q.prevClose = 100.0; // +20 / +20%
    q.bid = 119.5;
    q.ask = 120.5;
    q.volume = 1234567;
    EXPECT_EQ(QuoteCell(0, q), "NVDA");
    EXPECT_EQ(QuoteCell(1, q), "120.00");
    EXPECT_EQ(QuoteCell(4, q), "119.50");
    EXPECT_EQ(QuoteCell(5, q), "120.50");
    EXPECT_EQ(QuoteCell(6, q), "1,234,567");
    // Chg% column
    EXPECT_EQ(QuoteCell(3, q), "20.00%");
}

// ── Factories: freeze-pane + render smoke ────────────────────────────────────

TEST_F(BlottersTest, PositionsBlotter_FreezesSymbol_AndRenders) {
    auto blotter = MakePositionsBlotter("pos");
    EXPECT_EQ(blotter.GetFrozenColumns(), 1);
    std::vector<Position> rows{{"AAPL", 100, 185.0, 190.0}, {"MSFT", -50, 410.0, 405.0}};
    blotter.SetDataSource(&rows);
    ImGui::Begin("w");
    blotter.Render();
    ImGui::End();
}

TEST_F(BlottersTest, OrdersBlotter_Renders) {
    auto blotter = MakeOrdersBlotter("ord");
    EXPECT_EQ(blotter.GetFrozenColumns(), 2);
    std::vector<Order> rows{{"O1", "AAPL", Side::Buy, 100.0, 10, 0, OrderStatus::New}};
    blotter.SetDataSource(&rows);
    ImGui::Begin("w");
    blotter.Render();
    ImGui::End();
}

TEST_F(BlottersTest, TradesTape_Renders) {
    auto tape = MakeTradesTape("tape");
    std::vector<Trade> rows{{"T1", "AAPL", Side::Buy, 100.0, 10, 0.0},
                            {"T2", "AAPL", Side::Sell, 100.1, 5, 1.0}};
    tape.SetDataSource(&rows);
    ImGui::Begin("w");
    tape.Render();
    ImGui::End();
}

TEST_F(BlottersTest, Watchlist_Renders) {
    auto wl = MakeWatchlist("wl");
    std::vector<Quote> rows(1);
    rows[0].symbol = "AAPL";
    rows[0].last = 190.0;
    rows[0].prevClose = 188.0;
    wl.SetDataSource(&rows);
    ImGui::Begin("w");
    wl.Render();
    ImGui::End();
}

TEST_F(BlottersTest, EmptyData_Renders) {
    auto blotter = MakePositionsBlotter("pos2");
    std::vector<Position> empty;
    blotter.SetDataSource(&empty);
    ImGui::Begin("w");
    blotter.Render();
    ImGui::End();
}
