#include <unigui/core/accessibility.h>
#include <unigui/trading/depth_ladder.h>
#include <unigui/trading/order_book.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui::trading;

// Headless ImGui frame fixture (no ImPlot needed — the ladder draws via the
// window draw-list and standard widgets only).
class DepthLadderTest : public ::testing::Test {
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

    // A small deterministic book: 4 bids + 4 asks around 100.
    static OrderBook MakeBook() {
        OrderBook b;
        for (int i = 0; i < 4; ++i) {
            b.SetBid(100.0 - i * 0.1, 100 + i * 10);
            b.SetAsk(100.1 + i * 0.1, 120 + i * 10);
        }
        return b;
    }
};

TEST_F(DepthLadderTest, Defaults) {
    DepthLadder dom("d1");
    EXPECT_EQ(dom.Book(), nullptr);
    EXPECT_EQ(dom.Depth(), 0);
    EXPECT_FLOAT_EQ(dom.RowHeight(), 20.0f);
    EXPECT_TRUE(dom.ShowSpreadRow());
    EXPECT_FALSE(dom.AutoCenter());
    EXPECT_NE(dom.BidColor(), dom.AskColor());
}

TEST_F(DepthLadderTest, SetBook_Binds) {
    OrderBook b = MakeBook();
    DepthLadder dom("d2");
    dom.SetBook(&b);
    EXPECT_EQ(dom.Book(), &b);
}

TEST_F(DepthLadderTest, Setters_Clamped) {
    DepthLadder dom("d3");
    dom.SetRowHeight(1000.0f);
    EXPECT_FLOAT_EQ(dom.RowHeight(), 64.0f);
    dom.SetRowHeight(1.0f);
    EXPECT_FLOAT_EQ(dom.RowHeight(), 8.0f);

    dom.SetPriceColumnWidth(5.0f);
    EXPECT_FLOAT_EQ(dom.PriceColumnWidth(), 24.0f);

    dom.SetBarOpacity(5.0f);
    EXPECT_FLOAT_EQ(dom.BarOpacity(), 1.0f);
    dom.SetBarOpacity(-1.0f);
    EXPECT_FLOAT_EQ(dom.BarOpacity(), 0.0f);

    dom.SetPriceDecimals(-3);
    EXPECT_EQ(dom.PriceDecimals(), 0);

    dom.SetDepth(-5);
    EXPECT_EQ(dom.Depth(), 0); // negative coerced to "all"

    dom.SetSizeColumnWidth(-10.0f);
    EXPECT_FLOAT_EQ(dom.SizeColumnWidth(), 0.0f); // negative coerced to auto-split
}

TEST_F(DepthLadderTest, Fluent_Chaining_ReturnsDerived) {
    OrderBook b = MakeBook();
    DepthLadder dom("d4");
    DepthLadder& ref = dom.WithBook(&b).WithDepth(3).WithShowSpreadRow(false).WithAutoCenter(true);
    EXPECT_EQ(&ref, &dom);
    EXPECT_EQ(dom.Book(), &b);
    EXPECT_EQ(dom.Depth(), 3);
    EXPECT_FALSE(dom.ShowSpreadRow());
    EXPECT_TRUE(dom.AutoCenter());
}

TEST_F(DepthLadderTest, Render_NullBook_NoCrash) {
    DepthLadder dom("d5");
    ImGui::Begin("w");
    dom.Render(); // no book bound — early-out without crashing
    ImGui::End();
}

TEST_F(DepthLadderTest, Render_WithBook_NoCrash) {
    OrderBook b = MakeBook();
    DepthLadder dom("d6");
    dom.SetBook(&b);
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
}

TEST_F(DepthLadderTest, Render_EmptyBook_NoCrash) {
    OrderBook b; // no levels
    DepthLadder dom("d7");
    dom.SetBook(&b);
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
}

TEST_F(DepthLadderTest, Render_NoSpreadRow_NoCrash) {
    OrderBook b = MakeBook();
    DepthLadder dom("d8");
    dom.SetBook(&b);
    dom.SetShowSpreadRow(false);
    dom.SetBorder(false);
    dom.SetThemeBackground(false);
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
}

TEST_F(DepthLadderTest, Render_AutoCenter_NoCrash) {
    OrderBook b = MakeBook();
    DepthLadder dom("d9");
    dom.SetBook(&b);
    dom.SetAutoCenter(true);
    dom.SetSize(ImVec2(200, 80)); // smaller than content → exercises scrolling
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
}

TEST_F(DepthLadderTest, CenterOnMarket_OneShot_NoCrash) {
    OrderBook b = MakeBook();
    DepthLadder dom("d10");
    dom.SetBook(&b);
    dom.SetSize(ImVec2(200, 80));
    dom.CenterOnMarket();
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
}

TEST_F(DepthLadderTest, Render_BidsOnly_NoCrash) {
    OrderBook b;
    b.SetBid(99.0, 50);
    b.SetBid(98.9, 70);
    DepthLadder dom("d11");
    dom.SetBook(&b);
    ImGui::Begin("w");
    dom.Render(); // spread row must handle a one-sided book
    ImGui::End();
}

TEST_F(DepthLadderTest, Render_Hidden_DoesNothing) {
    OrderBook b = MakeBook();
    DepthLadder dom("d12");
    dom.SetBook(&b);
    dom.Hide();
    ImGui::Begin("w");
    dom.Render(); // hidden → early-out
    ImGui::End();
    EXPECT_FALSE(dom.IsVisible());
}

TEST_F(DepthLadderTest, Render_DepthLimited_NoCrash) {
    OrderBook b = MakeBook(); // 4 levels each side
    DepthLadder dom("d13");
    dom.SetBook(&b);
    dom.SetDepth(2); // only top 2 per side rendered
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
}

// ── Accessibility: the ladder registers with the inside market as its value ──
TEST_F(DepthLadderTest, A11y_RegistersInsideMarket) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    OrderBook b = MakeBook(); // best bid 100.0x100, best ask 100.1x120
    DepthLadder dom("d_a11y");
    dom.SetBook(&b);
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
    bool saw = false;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::Table) {
            saw = true;
            EXPECT_NE(n.value.find("bid 100.00 x 100"), std::string::npos) << n.value;
            EXPECT_NE(n.value.find("ask 100.10 x 120"), std::string::npos) << n.value;
            EXPECT_NE(n.value.find("spread 0.10"), std::string::npos) << n.value;
        }
    }
    EXPECT_TRUE(saw);
    unigui::a11y::SetEnabled(false);
}

TEST_F(DepthLadderTest, A11y_EmptyBook_ReportsEmpty) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    OrderBook b; // no levels
    DepthLadder dom("d_a11y_empty");
    dom.SetBook(&b);
    ImGui::Begin("w");
    dom.Render();
    ImGui::End();
    bool saw = false;
    for (const auto& n : unigui::a11y::Tree())
        if (n.role == unigui::a11y::Role::Table && n.value == "empty book")
            saw = true;
    EXPECT_TRUE(saw);
    unigui::a11y::SetEnabled(false);
}
