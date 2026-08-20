#include <unigui/unigui.h>
#include <unigui/widgets/riskbar.h>

#include <imgui.h>

#include <gtest/gtest.h>

class RiskBarTest : public ::testing::Test {
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

TEST_F(RiskBarTest, DefaultsToZero) {
    unigui::RiskBar bar("risk");
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 0.0);
}

TEST_F(RiskBarTest, DefaultConstructor_Renders) {
    unigui::RiskBar bar("risk");
    bar.Render(); // 0.0 ratio
}

// ---- ratio set/get ----

TEST_F(RiskBarTest, SetRatio_Works) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.75);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 0.75);
}

TEST_F(RiskBarTest, SetRatio_Zero) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.0);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 0.0);
    bar.Render();
}

TEST_F(RiskBarTest, SetRatio_Half) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.5);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 0.5);
    bar.Render();
}

TEST_F(RiskBarTest, SetRatio_One) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(1.0);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 1.0);
    bar.Render();
}

TEST_F(RiskBarTest, SetRatio_Negative) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(-0.3);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), -0.3);
    bar.Render(); // Edge case: negative ratio
}

TEST_F(RiskBarTest, SetRatio_OverMax) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(1.5);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 1.5);
    bar.Render(); // Edge case: > maxRatio
}

TEST_F(RiskBarTest, SetRatio_LargeValue) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(999.0);
    EXPECT_DOUBLE_EQ(bar.GetRatio(), 999.0);
    bar.Render(); // Edge case: very large value
}

// ---- max ratio ----

TEST_F(RiskBarTest, SetMaxRatio_Works) {
    unigui::RiskBar bar("risk");
    bar.SetMaxRatio(100.0);
    bar.SetRatio(50.0);
    bar.Render();
}

TEST_F(RiskBarTest, MaxRatio_Zero) {
    unigui::RiskBar bar("risk");
    bar.SetMaxRatio(0.0);
    bar.SetRatio(0.5);
    bar.Render(); // Edge case: maxRatio = 0 (prevents division by zero)
}

// ---- display text ----

TEST_F(RiskBarTest, DisplayText_DoesNotCrash) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.5);
    bar.SetDisplayText("230.21万/450.22万");
    bar.Render();
}

TEST_F(RiskBarTest, DisplayText_Empty) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.7);
    bar.SetDisplayText("");
    bar.Render();
}

TEST_F(RiskBarTest, DisplayText_Long) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.3);
    bar.SetDisplayText("This is a very long display text string on a risk bar widget component");
    bar.Render();
}

// ---- thresholds ----

TEST_F(RiskBarTest, WarnThreshold_DoesNotCrash) {
    unigui::RiskBar bar("risk");
    bar.SetWarnThreshold(50.0);
    bar.SetMaxRatio(100.0);
    bar.SetRatio(60.0); // Above warn threshold → yellow
    bar.Render();
}

TEST_F(RiskBarTest, DangerThreshold_DoesNotCrash) {
    unigui::RiskBar bar("risk");
    bar.SetDangerThreshold(85.0);
    bar.SetMaxRatio(100.0);
    bar.SetRatio(90.0); // Above danger threshold → red
    bar.Render();
}

TEST_F(RiskBarTest, BelowWarnThreshold_Green) {
    unigui::RiskBar bar("risk");
    bar.SetMaxRatio(100.0);
    bar.SetWarnThreshold(50.0);
    bar.SetDangerThreshold(85.0);
    bar.SetRatio(30.0); // Below warn → green
    bar.Render();
}

// ---- inverted ----

TEST_F(RiskBarTest, Inverted_DoesNotCrash) {
    unigui::RiskBar bar("risk");
    bar.SetInverted(true);
    bar.SetRatio(0.9); // High ratio → green when inverted
    bar.Render();
}

TEST_F(RiskBarTest, Inverted_LowRatio) {
    unigui::RiskBar bar("risk");
    bar.SetInverted(true);
    bar.SetRatio(0.1); // Low ratio → red when inverted
    bar.Render();
}

// ---- animated ----

TEST_F(RiskBarTest, Animated_DoesNotCrash) {
    unigui::RiskBar bar("risk");
    bar.SetAnimated(true);
    bar.SetRatio(0.5);
    bar.Render();
    bar.Render(); // Second frame for lerp
}

TEST_F(RiskBarTest, Animated_Transitions) {
    unigui::RiskBar bar("risk");
    bar.SetAnimated(true);
    bar.SetRatio(0.0);
    bar.Render();
    bar.SetRatio(1.0);
    bar.Render(); // Frame 1
    bar.Render(); // Frame 2
    bar.Render(); // Frame 3
}

// ---- combined scenarios ----

TEST_F(RiskBarTest, FullRender) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(75.0);
    bar.SetMaxRatio(100.0);
    bar.SetDisplayText("75万/100万");
    bar.SetWarnThreshold(70.0);
    bar.SetDangerThreshold(85.0);
    bar.SetInverted(false);
    bar.SetAnimated(true);
    bar.Render();
}

TEST_F(RiskBarTest, InvertedAnimated) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(10.0);
    bar.SetMaxRatio(100.0);
    bar.SetInverted(true);
    bar.SetAnimated(true);
    bar.SetDisplayText("10/100");
    bar.Render();
}

// ---- visibility ----

TEST_F(RiskBarTest, Hidden_DoesNotRender) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.5);
    bar.Hide();
    bar.Render();
    EXPECT_FALSE(bar.IsVisible());
}

TEST_F(RiskBarTest, Show_AfterHide) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.5);
    bar.Hide();
    EXPECT_FALSE(bar.IsVisible());
    bar.Show();
    EXPECT_TRUE(bar.IsVisible());
    bar.Render();
}

// ---- base Widget features ----

TEST_F(RiskBarTest, GetName_ReturnsName) {
    unigui::RiskBar bar("risk_id");
    EXPECT_EQ(bar.GetName(), "risk_id");
}

TEST_F(RiskBarTest, Tooltip_DoesNotCrash) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.7);
    bar.SetTooltip("Risk level: 70%");
    bar.Render();
}

// ---- PushID/PopID safety ----

TEST_F(RiskBarTest, PushPopID_Safety) {
    // Multiple renders with different names should not conflict
    unigui::RiskBar bar1("risk_one");
    unigui::RiskBar bar2("risk_two");
    bar1.SetRatio(0.3);
    bar2.SetRatio(0.7);
    bar1.Render();
    bar2.Render();
    bar1.Render(); // Rerender bar1 after bar2 — ID stack should be clean
}

TEST_F(RiskBarTest, PushPopID_SameName) {
    // Two renders of the same bar should not corrupt the ID stack
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.5);
    bar.Render();
    bar.Render();
}

// ---- secondary segment ----

TEST_F(RiskBarTest, Secondary_DisabledByDefault) {
    unigui::RiskBar bar("risk");
    EXPECT_DOUBLE_EQ(bar.GetSecondaryRatio(), 0.0);
    bar.SetRatio(0.6);
    bar.SetSecondaryRatio(0.2);
    // Not enabled: secondary value is stored but must not affect rendering
    bar.Render();
    EXPECT_DOUBLE_EQ(bar.GetSecondaryRatio(), 0.2);
}

TEST_F(RiskBarTest, Secondary_Renders) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.6);
    bar.SetSecondaryRatio(0.25);
    bar.SetSecondaryEnabled(true);
    bar.Render();
}

TEST_F(RiskBarTest, Secondary_GetSet) {
    unigui::RiskBar bar("risk");
    bar.SetSecondaryRatio(0.3);
    EXPECT_DOUBLE_EQ(bar.GetSecondaryRatio(), 0.3);
    bar.SetSecondaryRatio(-0.5); // caller may pass junk; getter echoes, render clamps
    EXPECT_DOUBLE_EQ(bar.GetSecondaryRatio(), -0.5);
}

TEST_F(RiskBarTest, Secondary_TotalOverTrack) {
    // main + secondary beyond maxRatio: rendering clamps, no crash, no overflow
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.9);
    bar.SetSecondaryRatio(0.9);
    bar.SetSecondaryEnabled(true);
    bar.Render();
}

TEST_F(RiskBarTest, Secondary_ZeroMain) {
    // All usage in the secondary segment (e.g. 100% money funds)
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.0);
    bar.SetSecondaryRatio(0.7);
    bar.SetSecondaryEnabled(true);
    bar.Render();
}

TEST_F(RiskBarTest, Secondary_Animated) {
    unigui::RiskBar bar("risk");
    bar.SetAnimated(true);
    bar.SetRatio(0.5);
    bar.SetSecondaryRatio(0.2);
    bar.SetSecondaryEnabled(true);
    bar.Render();
    bar.Render();
    bar.Render();
}

TEST_F(RiskBarTest, Secondary_WithTextAndTooltip) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.65);
    bar.SetSecondaryRatio(0.15);
    bar.SetSecondaryEnabled(true);
    bar.SetDisplayText("65% + 15%");
    bar.SetTooltip("main 65% / secondary 15% / idle 20%");
    bar.Render();
}

TEST_F(RiskBarTest, Secondary_ReenableAfterDisable) {
    unigui::RiskBar bar("risk");
    bar.SetRatio(0.4);
    bar.SetSecondaryRatio(0.1);
    bar.SetSecondaryEnabled(true);
    bar.Render();
    bar.SetSecondaryEnabled(false);
    bar.Render();
    bar.SetSecondaryEnabled(true);
    bar.Render();
}
