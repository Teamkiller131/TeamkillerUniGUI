// Interaction tests for the trading widgets — the engine drives the module-gated
// ticket / ladder / chart through real input. Compiled only when BOTH
// UNIGUI_TEST_ENGINE and UNIGUI_MODULE_TRADING are on (the
// windows-msvc-debug-testengine-modules preset provides the combination).
#include <unigui/trading/depth_ladder.h>
#include <unigui/trading/order_book.h>
#include <unigui/trading/order_ticket.h>

#include <string>

#include "interaction_harness.h"

namespace tr = unigui::trading;

class TradingInteractionTest : public itest::InteractionFixture {};

// ── OrderTicket: the submit button fires OnSubmit with the validated draft ────
TEST_F(TradingInteractionTest, OrderTicket_ValidDraft_SubmitFiresOnSubmit) {
    tr::OrderTicket ticket("ti_ticket");
    tr::OrderDraft& d = ticket.Draft();
    d.symbol = "IF2509";
    d.side = tr::Side::Buy;
    d.type = tr::OrderType::Limit;
    d.qty = 1;
    d.price = 7000.0;
    bool fired = false;
    tr::OrderDraft submitted;
    ticket.SetOnSubmit([&](const tr::OrderDraft& od) {
        submitted = od;
        fired = true;
    });

    const auto st = Run(
        "trading_ticket_submit", [&] { ticket.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Buy Limit"); // the submit button (side + type label)
        });

    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(fired);
    EXPECT_EQ(submitted.symbol, "IF2509");
    EXPECT_EQ(submitted.price, 7000.0);
}

// ── OrderTicket: an invalid draft keeps the gate closed ───────────────────────
TEST_F(TradingInteractionTest, OrderTicket_InvalidPrice_BlocksSubmit) {
    tr::OrderTicket ticket("ti_ticket_inv");
    tr::OrderDraft& d = ticket.Draft();
    d.symbol = "IF2509";
    d.side = tr::Side::Buy;
    d.type = tr::OrderType::Limit;
    d.qty = 1;
    d.price = 0.0; // invalid: limit orders need a price
    int fired = 0;
    ticket.SetOnSubmit([&](const tr::OrderDraft&) { ++fired; });

    const auto st = Run(
        "trading_ticket_blocked", [&] { ticket.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Buy Limit");
        });

    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(fired, 0) << "a disabled (invalid) submit button must not fire OnSubmit";
}

// ── DepthLadder: clicking a level fires OnLevelClick with that level's data ───
TEST_F(TradingInteractionTest, DepthLadder_LevelClick_FiresCallback) {
    tr::OrderBook book;
    book.ApplyAskSnapshot({{7001.0, 5}, {7000.0, 3}});
    book.ApplyBidSnapshot({{6999.0, 2}, {6998.0, 4}});

    tr::DepthLadder ladder("ti_ladder");
    ladder.SetBook(&book);
    bool fired = false;
    tr::DepthLadder::Side side = tr::DepthLadder::Side::Bid;
    double price = 0.0;
    std::int64_t size = 0;
    ladder.SetOnLevelClick([&](tr::DepthLadder::Side s, double p, std::int64_t z) {
        side = s;
        price = p;
        size = z;
        fired = true;
    });

    const auto st = Run(
        "trading_ladder_click", [&] { ladder.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            // The topmost level row (rowIndex 0) is the best ask.
            ctx->ItemClick("**/row");
        });

    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(fired);
    EXPECT_EQ(side, tr::DepthLadder::Side::Ask);
    EXPECT_DOUBLE_EQ(price, 7001.0);
    EXPECT_EQ(size, 5);
}

// ── CandlestickChart: NOT driven under the engine. ImPlot (BeginPlot's window and
// input hooks) is incompatible with the engine's per-frame state manipulation — a
// plain render crashes inside the engine (yield assert + access violation). The
// chart keeps its headless frame tests (tests/trading/candlestick_chart_test.cc);
// engine-driving it needs engine-aware ImPlot handling, tracked as a follow-up.
