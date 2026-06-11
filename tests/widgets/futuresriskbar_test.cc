#include <unigui/unigui.h>
#include <unigui/widgets/futuresriskbar.h>

#include <imgui.h>

#include <gtest/gtest.h>

class FuturesRiskBarTest : public ::testing::Test {
protected:
    void SetUp() override {
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

// ---- defaults ----

TEST_F(FuturesRiskBarTest, DefaultConstructor_Renders) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.Render();
}

// ---- account name ----

TEST_F(FuturesRiskBarTest, SetAccountName_Renders) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAccountName("Account123");
    bar.Render();
}

TEST_F(FuturesRiskBarTest, SetAccountName_Empty) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAccountName("");
    bar.Render();
}

TEST_F(FuturesRiskBarTest, SetAccountName_Long) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAccountName("This is a very long account name for testing");
    bar.Render();
}

// ---- margin text ----

TEST_F(FuturesRiskBarTest, SetMarginText_Renders) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetMarginText("230.21万/450.22万");
    bar.Render();
}

TEST_F(FuturesRiskBarTest, SetMarginText_Empty) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetMarginText("");
    bar.Render();
}

// ---- actual ratio (green fill) ----

TEST_F(FuturesRiskBarTest, ActualRatio_Zero) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(0.0);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, ActualRatio_Half) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(0.5);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, ActualRatio_One) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(1.0);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, ActualRatio_Negative) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(-0.3);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, ActualRatio_OverOne) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(1.5);
    bar.Render();
}

// ---- estimated ratio (yellow solid line) ----

TEST_F(FuturesRiskBarTest, EstimatedRatio_Zero) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetEstimatedRatio(0.0);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, EstimatedRatio_Mid) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetEstimatedRatio(0.7);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, EstimatedRatio_One) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetEstimatedRatio(1.0);
    bar.Render();
}

// ---- overnight ratio (red dashed line) ----

TEST_F(FuturesRiskBarTest, OvernightRatio_Zero) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetOvernightRatio(0.0);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, OvernightRatio_Mid) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetOvernightRatio(0.5);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, OvernightRatio_One) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetOvernightRatio(1.0);
    bar.Render();
}

// ---- animated ----

TEST_F(FuturesRiskBarTest, Animated_DoesNotCrash) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAnimated(true);
    bar.SetActualRatio(0.5);
    bar.SetEstimatedRatio(0.7);
    bar.SetOvernightRatio(0.85);
    bar.Render();
    bar.Render();
}

TEST_F(FuturesRiskBarTest, Animated_Transitions) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAnimated(true);
    bar.SetActualRatio(0.0);
    bar.SetEstimatedRatio(0.0);
    bar.SetOvernightRatio(0.0);
    bar.Render();
    bar.SetActualRatio(1.0);
    bar.SetEstimatedRatio(0.9);
    bar.SetOvernightRatio(0.8);
    bar.Render();
    bar.Render();
    bar.Render();
}

// ---- combined scenarios ----

TEST_F(FuturesRiskBarTest, FullRender) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAccountName("Account888");
    bar.SetMarginText("230.21万/450.22万");
    bar.SetActualRatio(0.51);
    bar.SetEstimatedRatio(0.75);
    bar.SetOvernightRatio(0.90);
    bar.SetAnimated(true);
    bar.Render();
}

TEST_F(FuturesRiskBarTest, FullRenderNoAnimation) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetAccountName("TestAccount");
    bar.SetMarginText("100万/200万");
    bar.SetActualRatio(0.5);
    bar.SetEstimatedRatio(0.6);
    bar.SetOvernightRatio(0.8);
    bar.Render();
}

// ---- visibility ----

TEST_F(FuturesRiskBarTest, Hidden_DoesNotRender) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(0.5);
    bar.Hide();
    bar.Render();
    EXPECT_FALSE(bar.IsVisible());
}

TEST_F(FuturesRiskBarTest, Show_AfterHide) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(0.5);
    bar.Hide();
    EXPECT_FALSE(bar.IsVisible());
    bar.Show();
    EXPECT_TRUE(bar.IsVisible());
    bar.Render();
}

// ---- base Widget features ----

TEST_F(FuturesRiskBarTest, GetName_ReturnsName) {
    unigui::FuturesRiskBar bar("futures_risk_id");
    EXPECT_EQ(bar.GetName(), "futures_risk_id");
}

TEST_F(FuturesRiskBarTest, Tooltip_DoesNotCrash) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(0.7);
    bar.SetTooltip("Futures risk level: 70%");
    bar.Render();
}

// ---- PushID/PopID safety ----

TEST_F(FuturesRiskBarTest, PushPopID_Safety) {
    unigui::FuturesRiskBar bar1("risk_one");
    unigui::FuturesRiskBar bar2("risk_two");
    bar1.SetActualRatio(0.3);
    bar2.SetActualRatio(0.7);
    bar1.Render();
    bar2.Render();
    bar1.Render();
}

TEST_F(FuturesRiskBarTest, PushPopID_SameName) {
    unigui::FuturesRiskBar bar("futures_risk");
    bar.SetActualRatio(0.5);
    bar.Render();
    bar.Render();
}
