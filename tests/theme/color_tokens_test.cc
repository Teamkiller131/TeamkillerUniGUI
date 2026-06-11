#include <unigui/theme/color_tokens.h>
#include <unigui/theme/presets/registry.h>
#include <unigui/theme/theme.h>

#include <imgui.h>

#include <gtest/gtest.h>

using unigui::theme::AccentFromStyle;
using unigui::theme::AllSemantics;
using unigui::theme::ApplyColorTokens;
using unigui::theme::ColorTokens;
using unigui::theme::DeriveColorTokens;
using unigui::theme::Semantic;
using unigui::theme::SemanticColor;
using unigui::theme::SemanticName;
using unigui::theme::StyleIsDark;

class ColorTokensTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(ColorTokensTest, DeriveProducesHoverAndActiveFromAccent) {
    ImVec4 accent(0.40f, 0.58f, 0.93f, 1.00f);
    ColorTokens t = DeriveColorTokens(accent, /*dark=*/true);
    // Hover brighter, active darker, info == accent, alpha preserved.
    EXPECT_GT(t.accent_hover.x, accent.x);
    EXPECT_LT(t.accent_active.x, accent.x);
    EXPECT_FLOAT_EQ(t.info.x, accent.x);
    EXPECT_FLOAT_EQ(t.accent_hover.w, 1.00f);
}

TEST_F(ColorTokensTest, DarkAndLightSemanticsDiffer) {
    ImVec4 accent(0.40f, 0.58f, 0.93f, 1.00f);
    ColorTokens dark = DeriveColorTokens(accent, true);
    ColorTokens light = DeriveColorTokens(accent, false);
    // Dark semantics are brighter than the matching light ones.
    EXPECT_GT(dark.success.y, light.success.y);
    EXPECT_GT(dark.danger.x, light.danger.x);
}

TEST_F(ColorTokensTest, SemanticColorSelectsRole) {
    ImVec4 accent(0.40f, 0.58f, 0.93f, 1.00f);
    ColorTokens t = DeriveColorTokens(accent, true);
    EXPECT_FLOAT_EQ(SemanticColor(t, Semantic::Accent).x, t.accent.x);
    EXPECT_FLOAT_EQ(SemanticColor(t, Semantic::Success).y, t.success.y);
    EXPECT_FLOAT_EQ(SemanticColor(t, Semantic::Info).x, t.accent.x);
}

TEST_F(ColorTokensTest, ApplyWritesAccentDrivenSlots) {
    ImVec4 accent(0.40f, 0.58f, 0.93f, 1.00f);
    auto& s = ImGui::GetStyle();
    ApplyColorTokens(s, accent, /*dark=*/true);
    auto& c = s.Colors;
    EXPECT_FLOAT_EQ(c[ImGuiCol_CheckMark].x, accent.x);
    EXPECT_FLOAT_EQ(c[ImGuiCol_SliderGrab].x, accent.x);
    EXPECT_FLOAT_EQ(c[ImGuiCol_SeparatorActive].x, accent.x);
    EXPECT_FLOAT_EQ(c[ImGuiCol_NavHighlight].x, accent.x);
    // Docking preview / selection are accent-tinted but translucent.
    EXPECT_FLOAT_EQ(c[ImGuiCol_DockingPreview].w, 0.70f);
    EXPECT_FLOAT_EQ(c[ImGuiCol_TextSelectedBg].w, 0.35f);
}

TEST_F(ColorTokensTest, ApplyUpdatesActiveSemanticPalette) {
    ImVec4 accent(0.10f, 0.70f, 0.40f, 1.00f);
    ApplyColorTokens(ImGui::GetStyle(), accent, /*dark=*/true);
    // Active queries reflect the just-applied tokens.
    EXPECT_FLOAT_EQ(unigui::theme::GetSemanticColor(Semantic::Accent).x, accent.x);
    EXPECT_FLOAT_EQ(unigui::GetSemanticColor(Semantic::Info).y, accent.y);
    EXPECT_FLOAT_EQ(unigui::GetColorTokens().accent.z, accent.z);
}

TEST_F(ColorTokensTest, AllSemanticsAreNamed) {
    const auto& all = AllSemantics();
    EXPECT_EQ(all.size(), 5u);
    for (auto sem : all)
        EXPECT_STRNE(SemanticName(sem), "");
    EXPECT_STREQ(SemanticName(Semantic::Danger), "Danger");
}

TEST_F(ColorTokensTest, AccentFromStyleReadsCheckMark) {
    auto& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_CheckMark] = ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
    ImVec4 a = AccentFromStyle(s);
    EXPECT_FLOAT_EQ(a.x, 0.2f);
    EXPECT_FLOAT_EQ(a.z, 0.8f);
}

TEST_F(ColorTokensTest, StyleIsDarkHeuristic) {
    auto& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
    EXPECT_TRUE(StyleIsDark(s));
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0.96f, 0.96f, 0.97f, 1.0f);
    EXPECT_FALSE(StyleIsDark(s));
}

// ── Integration with ApplyTheme / ThemeRegistry ──────────────────────────────

TEST_F(ColorTokensTest, ApplyThemeDerivesAccentSlotsFromCheckMark) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark});
    auto& c = ImGui::GetStyle().Colors;
    // The accent-driven slots all match the accent (CheckMark) after theming.
    EXPECT_FLOAT_EQ(c[ImGuiCol_SeparatorActive].x, c[ImGuiCol_CheckMark].x);
    EXPECT_FLOAT_EQ(c[ImGuiCol_ResizeGripActive].y, c[ImGuiCol_CheckMark].y);
    EXPECT_FLOAT_EQ(c[ImGuiCol_DragDropTarget].z, c[ImGuiCol_CheckMark].z);
}

TEST_F(ColorTokensTest, AllPresetsShareAccentRelationship) {
    unigui::theme::RegisterAllThemes();
    auto& reg = unigui::theme::ThemeRegistry::Instance();
    for (const auto& name : reg.List()) {
        ImGui::GetStyle() = ImGuiStyle{}; // reset
        ASSERT_TRUE(reg.Apply(name)) << name;
        auto& c = ImGui::GetStyle().Colors;
        // Every preset's accent-driven slots track its own accent (CheckMark),
        // so the accent→hover→active relationship is consistent across themes.
        EXPECT_FLOAT_EQ(c[ImGuiCol_SeparatorActive].x, c[ImGuiCol_CheckMark].x) << name;
        EXPECT_FLOAT_EQ(c[ImGuiCol_NavHighlight].y, c[ImGuiCol_CheckMark].y) << name;
        EXPECT_FLOAT_EQ(c[ImGuiCol_DockingPreview].w, 0.70f) << name;
    }
}
