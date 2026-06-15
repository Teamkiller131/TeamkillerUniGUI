#include <unigui/widgets/priceticker.h>

#include <imgui.h>

#include <gtest/gtest.h>

class PriceTickerTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

using unigui::PriceTicker;

static std::vector<PriceTicker::Item> SampleItems() {
    return {{"AAPL", "192.30", +1.2f}, {"MSFT", "410.10", -0.8f}, {"BTC", "64,200", 0.f}};
}

TEST_F(PriceTickerTest, Render_EmptyDoesNotCrash) {
    PriceTicker t("tk");
    EXPECT_NO_THROW(t.Render());
    EXPECT_EQ(t.ItemCount(), 0u);
}

TEST_F(PriceTickerTest, Constructor_TakesItems) {
    PriceTicker t("tk", SampleItems());
    EXPECT_EQ(t.ItemCount(), 3u);
    EXPECT_EQ(t.GetItems()[0].symbol, "AAPL");
}

TEST_F(PriceTickerTest, AddItem_Appends) {
    PriceTicker t("tk");
    t.AddItem({"ETH", "3,500", +2.f});
    EXPECT_EQ(t.ItemCount(), 1u);
    EXPECT_EQ(t.GetItems()[0].symbol, "ETH");
}

TEST_F(PriceTickerTest, Clear_ResetsItemsAndOffset) {
    PriceTicker t("tk", SampleItems());
    t.Render();
    t.Clear();
    EXPECT_EQ(t.ItemCount(), 0u);
    EXPECT_FLOAT_EQ(t.GetScrollOffset(), 0.f);
}

TEST_F(PriceTickerTest, Speed_DefaultsAndSets) {
    PriceTicker t("tk");
    EXPECT_FLOAT_EQ(t.GetSpeed(), 60.f);
    t.SetSpeed(120.f);
    EXPECT_FLOAT_EQ(t.GetSpeed(), 120.f);
}

TEST_F(PriceTickerTest, Scroll_AdvancesWithTime) {
    PriceTicker t("tk", SampleItems());
    t.SetSpeed(100.f);
    ImGui::GetIO().DeltaTime = 0.1f;
    t.Render();
    // 100 px/s * 0.1s = ~10px (mod cycle width); must have advanced from 0.
    EXPECT_GT(t.GetScrollOffset(), 0.f);
}

TEST_F(PriceTickerTest, Paused_DoesNotAdvance) {
    PriceTicker t("tk", SampleItems());
    t.SetPaused(true);
    ImGui::GetIO().DeltaTime = 0.5f;
    t.Render();
    EXPECT_FLOAT_EQ(t.GetScrollOffset(), 0.f);
    EXPECT_TRUE(t.IsPaused());
}

TEST_F(PriceTickerTest, Render_MixedSignsDoesNotCrash) {
    PriceTicker t("tk", SampleItems());
    ImGui::GetIO().DeltaTime = 0.016f;
    for (int i = 0; i < 5; ++i)
        EXPECT_NO_THROW(t.Render());
}

TEST_F(PriceTickerTest, Fluent_ChainsAndKeepsType) {
    PriceTicker t("tk");
    PriceTicker& ref = t.WithItems(SampleItems()).WithSpeed(80.f).WithHeight(24.f).WithWidth(400.f);
    EXPECT_EQ(&ref, &t);
    EXPECT_EQ(t.ItemCount(), 3u);
    EXPECT_FLOAT_EQ(t.GetSpeed(), 80.f);
}
