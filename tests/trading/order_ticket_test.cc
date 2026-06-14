#include <unigui/trading/order_ticket.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui::trading;

// Headless ImGui frame fixture for the render smoke tests. The Validate()/
// Submit() logic is pure and needs no frame, but Render() does.
class OrderTicketTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1024, 768);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

// ── Validation (pure, no frame) ──────────────────────────────────────────────

TEST_F(OrderTicketTest, Validate_EmptySymbol_Fails) {
    OrderTicket t("t1");
    t.WithQuantity(100).WithPrice(10.0);
    EXPECT_FALSE(t.Validate().ok);
}

TEST_F(OrderTicketTest, Validate_NonPositiveQty_Fails) {
    OrderTicket t("t2");
    t.WithSymbol("AAPL").WithQuantity(0).WithPrice(10.0);
    EXPECT_FALSE(t.Validate());
    t.SetQuantity(-5);
    EXPECT_FALSE(t.Validate());
}

TEST_F(OrderTicketTest, Validate_LimitRequiresPrice) {
    OrderTicket t("t3");
    t.WithSymbol("AAPL").WithOrderType(OrderType::Limit).WithQuantity(100).WithPrice(0.0);
    EXPECT_FALSE(t.Validate());
    t.SetPrice(123.45);
    EXPECT_TRUE(t.Validate().ok);
}

TEST_F(OrderTicketTest, Validate_MarketNeedsNoPrice) {
    OrderTicket t("t4");
    t.WithSymbol("AAPL").WithOrderType(OrderType::Market).WithQuantity(100).WithPrice(0.0);
    EXPECT_TRUE(t.Validate().ok); // market order: price not required
}

TEST_F(OrderTicketTest, Validate_StopRequiresStopPrice) {
    OrderTicket t("t5");
    t.WithSymbol("AAPL").WithOrderType(OrderType::Stop).WithQuantity(100);
    EXPECT_FALSE(t.Validate()); // no stop price
    t.SetStopPrice(99.0);
    EXPECT_TRUE(t.Validate().ok);
}

TEST_F(OrderTicketTest, Validate_StopLimitRequiresBoth) {
    OrderTicket t("t6");
    t.WithSymbol("AAPL").WithOrderType(OrderType::StopLimit).WithQuantity(100);
    EXPECT_FALSE(t.Validate());
    t.SetPrice(100.0);
    EXPECT_FALSE(t.Validate()); // still missing stop
    t.SetStopPrice(99.0);
    EXPECT_TRUE(t.Validate().ok);
}

TEST_F(OrderTicketTest, Validate_MaxQuantityEnforced) {
    OrderTicket t("t7");
    t.WithSymbol("AAPL").WithOrderType(OrderType::Market).WithQuantity(5000).WithMaxQuantity(1000);
    EXPECT_FALSE(t.Validate());
    t.SetQuantity(1000);
    EXPECT_TRUE(t.Validate().ok);
}

// ── Submit (pure, no frame) ──────────────────────────────────────────────────

TEST_F(OrderTicketTest, Submit_InvalidDoesNotFire) {
    OrderTicket t("t8");
    int fired = 0;
    t.SetOnSubmit([&](const OrderDraft&) { ++fired; });
    t.WithSymbol("").WithQuantity(100); // invalid: empty symbol
    OrderValidation v = t.Submit();
    EXPECT_FALSE(v.ok);
    EXPECT_EQ(fired, 0);
}

TEST_F(OrderTicketTest, Submit_ValidFiresWithDraft) {
    OrderTicket t("t9");
    OrderDraft got;
    int fired = 0;
    t.SetOnSubmit([&](const OrderDraft& d) {
        got = d;
        ++fired;
    });
    t.WithSymbol("MSFT")
        .WithSide(Side::Sell)
        .WithOrderType(OrderType::Limit)
        .WithQuantity(250)
        .WithPrice(412.34)
        .WithTimeInForce(TimeInForce::GTC);
    OrderValidation v = t.Submit();
    EXPECT_TRUE(v.ok);
    EXPECT_EQ(fired, 1);
    EXPECT_EQ(got.symbol, "MSFT");
    EXPECT_EQ(got.side, Side::Sell);
    EXPECT_EQ(got.qty, 250);
    EXPECT_EQ(got.tif, TimeInForce::GTC);
}

TEST_F(OrderTicketTest, Submit_SnapsPriceToTick) {
    OrderTicket t("t10");
    OrderDraft got;
    t.SetOnSubmit([&](const OrderDraft& d) { got = d; });
    t.WithSymbol("AAPL")
        .WithOrderType(OrderType::Limit)
        .WithQuantity(100)
        .WithTickSize(0.05)
        .WithPrice(100.123);
    OrderValidation v = t.Submit();
    ASSERT_TRUE(v.ok);
    EXPECT_NEAR(got.price, 100.10, 1e-9); // snapped to nearest 0.05
}

TEST_F(OrderTicketTest, Submit_StopLimitSnapsBothPrices) {
    OrderTicket t("t11");
    OrderDraft got;
    t.SetOnSubmit([&](const OrderDraft& d) { got = d; });
    t.WithSymbol("AAPL")
        .WithOrderType(OrderType::StopLimit)
        .WithQuantity(100)
        .WithTickSize(0.10)
        .WithPrice(50.06)
        .WithStopPrice(49.94);
    ASSERT_TRUE(t.Submit().ok);
    EXPECT_NEAR(got.price, 50.10, 1e-9);
    EXPECT_NEAR(got.stopPrice, 49.90, 1e-9);
}

// ── Setters / fluent ─────────────────────────────────────────────────────────

TEST_F(OrderTicketTest, Defaults) {
    OrderTicket t("t12");
    EXPECT_EQ(t.Draft().side, Side::Buy);
    EXPECT_EQ(t.Draft().type, OrderType::Limit);
    EXPECT_EQ(t.Draft().tif, TimeInForce::Day);
    EXPECT_EQ(t.QuantityStep(), 1);
    EXPECT_FALSE(t.Confirm());
}

TEST_F(OrderTicketTest, QuantityStep_ClampedToAtLeastOne) {
    OrderTicket t("t13");
    t.SetQuantityStep(0);
    EXPECT_EQ(t.QuantityStep(), 1);
    t.SetQuantityStep(-10);
    EXPECT_EQ(t.QuantityStep(), 1);
    t.SetQuantityStep(50);
    EXPECT_EQ(t.QuantityStep(), 50);
}

TEST_F(OrderTicketTest, Fluent_Chaining_ReturnsDerived) {
    OrderTicket t("t14");
    OrderTicket& ref = t.WithSymbol("NVDA").WithSide(Side::Buy).WithQuantity(10).WithConfirm(true);
    EXPECT_EQ(&ref, &t);
    EXPECT_EQ(t.Draft().symbol, "NVDA");
    EXPECT_TRUE(t.Confirm());
}

TEST_F(OrderTicketTest, OrderTypeName_TimeInForceName) {
    EXPECT_STREQ(OrderTypeName(OrderType::StopLimit), "StopLimit");
    EXPECT_STREQ(TimeInForceName(TimeInForce::FOK), "FOK");
}

// ── Render smoke tests ───────────────────────────────────────────────────────

TEST_F(OrderTicketTest, Render_NoCrash) {
    OrderTicket t("t15");
    t.WithSymbol("AAPL").WithQuantity(100).WithPrice(190.0);
    ImGui::Begin("w");
    t.Render();
    ImGui::End();
}

TEST_F(OrderTicketTest, Render_AllTypes_NoCrash) {
    for (auto ty : {OrderType::Market, OrderType::Limit, OrderType::Stop, OrderType::StopLimit}) {
        OrderTicket t("ty");
        t.WithSymbol("AAPL").WithOrderType(ty).WithQuantity(100).WithPrice(10.0).WithStopPrice(9.0);
        ImGui::Begin("w");
        t.Render();
        ImGui::End();
    }
}

TEST_F(OrderTicketTest, Render_InvalidShowsNoCrash) {
    OrderTicket t("t16");
    // Invalid (empty symbol) — the submit button is disabled and a message shows.
    ImGui::Begin("w");
    t.Render();
    ImGui::End();
}

TEST_F(OrderTicketTest, Render_Hidden_DoesNothing) {
    OrderTicket t("t17");
    t.Hide();
    ImGui::Begin("w");
    t.Render();
    ImGui::End();
    EXPECT_FALSE(t.IsVisible());
}
