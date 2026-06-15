#include <unigui/widgets/pnltext.h>

#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

class PnlTextTest : public ::testing::Test {
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
        theme::SetPolarity(theme::Polarity::RedUp); // restore default
    }
};

// ── Pure role mapping (no ImGui needed) ──────────────────────────────────────

TEST_F(PnlTextTest, PnlRole_ClassifiesSign) {
    EXPECT_TRUE(PnlRole(1.0).isDirectional);
    EXPECT_EQ(PnlRole(1.0).role, theme::Semantic::Up);
    EXPECT_TRUE(PnlRole(-1.0).isDirectional);
    EXPECT_EQ(PnlRole(-1.0).role, theme::Semantic::Down);
    EXPECT_FALSE(PnlRole(0.0).isDirectional); // flat → neutral
}

TEST_F(PnlTextTest, PnlRole_DeadBand) {
    EXPECT_FALSE(PnlRole(0.5, 1.0).isDirectional); // within eps → flat
    EXPECT_TRUE(PnlRole(1.5, 1.0).isDirectional);
}

TEST_F(PnlTextTest, GradedRole_Thresholds) {
    EXPECT_EQ(GradedRole(1.0, 5.0, 10.0), theme::Semantic::Success); // < warn
    EXPECT_EQ(GradedRole(7.0, 5.0, 10.0), theme::Semantic::Warning); // < crit
    EXPECT_EQ(GradedRole(12.0, 5.0, 10.0), theme::Semantic::Danger); // >= crit
}

TEST_F(PnlTextTest, GradedRole_Inverted) {
    // inverted: higher is better
    EXPECT_EQ(GradedRole(12.0, 10.0, 5.0, true), theme::Semantic::Success);
    EXPECT_EQ(GradedRole(1.0, 10.0, 5.0, true), theme::Semantic::Danger);
}

// ── Polarity affects Up/Down semantic colour ─────────────────────────────────

TEST_F(PnlTextTest, Polarity_FlipsUpDownColors) {
    theme::SetPolarity(theme::Polarity::RedUp);
    const ImVec4 upRed = theme::GetSemanticColor(theme::Semantic::Up);
    const ImVec4 danger = theme::GetSemanticColor(theme::Semantic::Danger);
    EXPECT_FLOAT_EQ(upRed.x, danger.x); // CN: up == danger (red) hue
    EXPECT_FLOAT_EQ(upRed.y, danger.y);

    theme::SetPolarity(theme::Polarity::GreenUp);
    const ImVec4 upGreen = theme::GetSemanticColor(theme::Semantic::Up);
    const ImVec4 success = theme::GetSemanticColor(theme::Semantic::Success);
    EXPECT_FLOAT_EQ(upGreen.x, success.x); // Western: up == success (green) hue
    EXPECT_FLOAT_EQ(upGreen.y, success.y);
}

TEST_F(PnlTextTest, GetDirectionColor_FlatIsNeutral) {
    const ImVec4 flat(0.1f, 0.2f, 0.3f, 1.f);
    const ImVec4 got = theme::GetDirectionColor(0.0, flat);
    EXPECT_FLOAT_EQ(got.x, flat.x);
    EXPECT_FLOAT_EQ(got.z, flat.z);
}

// ── Render paths don't crash ─────────────────────────────────────────────────

TEST_F(PnlTextTest, Render_DoesNotCrash) {
    EXPECT_NO_THROW({
        PnlText(3.5, "+3.50");
        PnlText(-2.0, "-2.00");
        PnlText(0.0, "0.00");
        PnlText(1.25, 2);
        StatusText(true, "ONLINE", "OFFLINE");
        StatusText(false, "ONLINE", "OFFLINE");
        GradedText(7.0, 5.0, 10.0, "7.0%");
    });
}
