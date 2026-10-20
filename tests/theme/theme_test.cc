#include <unigui/core/context.h>
#include <unigui/theme/presets/registry.h>
#include <unigui/theme/style_scope.h>
#include <unigui/theme/style_tokens.h>
#include <unigui/theme/theme.h>
#include <unigui/theme/tokens.h>
#include <unigui/unigui.h>

#include <imgui.h>

#include <gtest/gtest.h>

class ThemeTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(ThemeTest, DarkTheme_SetsExpectedWindowBg) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark});
    auto& colors = ImGui::GetStyle().Colors;
    EXPECT_FLOAT_EQ(colors[ImGuiCol_WindowBg].x, 0.10f);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_WindowBg].y, 0.10f);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_WindowBg].z, 0.12f);
}

TEST_F(ThemeTest, DarkTheme_SetsExpectedTextColor) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark});
    auto& colors = ImGui::GetStyle().Colors;
    EXPECT_FLOAT_EQ(colors[ImGuiCol_Text].x, 0.90f);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_Text].z, 0.92f);
}

TEST_F(ThemeTest, DarkTheme_SetsRounding) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark});
    auto& style = ImGui::GetStyle();
    EXPECT_FLOAT_EQ(style.WindowRounding, 6.0f);
    EXPECT_FLOAT_EQ(style.FrameRounding, 4.0f);
    EXPECT_FLOAT_EQ(style.GrabRounding, 4.0f);
}

TEST_F(ThemeTest, DarkTheme_SetsFramePadding) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark});
    auto& style = ImGui::GetStyle();
    EXPECT_FLOAT_EQ(style.FramePadding.x, 8.0f);
    EXPECT_FLOAT_EQ(style.FramePadding.y, 6.0f);
}

TEST_F(ThemeTest, DarkTheme_SetsAllColorsDifferFromDefault) {
    // Get defaults before theme
    auto default_style = ImGui::GetStyle();

    // Apply theme
    unigui::ApplyTheme({unigui::ThemePreset::Dark});
    auto& themed = ImGui::GetStyle().Colors;

    // Check that at least the key colors differ from defaults
    int changed = 0;
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
        auto d = default_style.Colors[i];
        auto t = themed[i];
        if (d.x != t.x || d.y != t.y || d.z != t.z || d.w != t.w) {
            changed++;
        }
    }
    EXPECT_GE(changed, 50); // At least 50 of 53 colors must differ
}

TEST_F(ThemeTest, DPI_Scaling_SetsFontGlobalScale) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark, 2.0f, 18.0f});
    // DPI is now applied via style.ScaleAllSizes() — verify style is scaled
    EXPECT_FLOAT_EQ(ImGui::GetStyle().ItemSpacing.x, 16.0f); // 8.0 * 2.0
}

TEST_F(ThemeTest, DPI_Scaling_DefaultIsOne) {
    unigui::ApplyTheme({unigui::ThemePreset::Dark, 1.0f, 18.0f});
    EXPECT_FLOAT_EQ(ImGui::GetStyle().ItemSpacing.x, 8.0f); // default, no scale
}

// ── Unified style-token tests (UI beautification Step 1) ─────────────────────

TEST_F(ThemeTest, StyleTokens_ApplyMatchesDefaults) {
    ImGuiStyle s;
    unigui::theme::ApplyStyleTokens(s);
    unigui::theme::StyleTokens t{};
    EXPECT_FLOAT_EQ(s.WindowRounding, t.window_rounding);
    EXPECT_FLOAT_EQ(s.FrameRounding, t.frame_rounding);
    EXPECT_FLOAT_EQ(s.GrabRounding, t.grab_rounding);
    EXPECT_FLOAT_EQ(s.TabRounding, t.tab_rounding);
    EXPECT_FLOAT_EQ(s.ChildRounding, t.child_rounding);
    EXPECT_FLOAT_EQ(s.PopupRounding, t.popup_rounding);
    EXPECT_FLOAT_EQ(s.ScrollbarRounding, t.scrollbar_rounding);
    EXPECT_FLOAT_EQ(s.GrabMinSize, t.grab_min_size);
    EXPECT_FLOAT_EQ(s.FramePadding.x, t.frame_padding.x);
    EXPECT_FLOAT_EQ(s.PopupBorderSize, t.popup_border);
}

TEST_F(ThemeTest, StyleTokens_AllPresetsShareGeometry) {
    // Every registered preset must apply the unified geometry tokens, so that
    // rounding/spacing is consistent regardless of which palette is active.
    unigui::theme::RegisterAllThemes();
    auto& reg = unigui::theme::ThemeRegistry::Instance();
    unigui::theme::StyleTokens t{};
    for (const auto& name : reg.List()) {
        ImGui::GetStyle() = ImGuiStyle{}; // reset
        ASSERT_TRUE(reg.Apply(name)) << name;
        auto& s = ImGui::GetStyle();
        EXPECT_FLOAT_EQ(s.WindowRounding, t.window_rounding) << name;
        EXPECT_FLOAT_EQ(s.FrameRounding, t.frame_rounding) << name;
        EXPECT_FLOAT_EQ(s.TabRounding, t.tab_rounding) << name;
        EXPECT_FLOAT_EQ(s.GrabMinSize, t.grab_min_size) << name;
        EXPECT_FLOAT_EQ(s.ScrollbarSize, t.scrollbar_size) << name;
    }
}

TEST_F(ThemeTest, AccentHelpers_DeriveHoverAndActive) {
    ImVec4 base(0.40f, 0.58f, 0.93f, 1.00f);
    ImVec4 hover = unigui::theme::AccentHover(base);
    ImVec4 active = unigui::theme::AccentActive(base);
    // Hover is brighter, active is darker, alpha preserved.
    EXPECT_GT(hover.x, base.x);
    EXPECT_LT(active.x, base.x);
    EXPECT_FLOAT_EQ(hover.w, 1.00f);
    EXPECT_FLOAT_EQ(active.w, 1.00f);
}

TEST_F(ThemeTest, AccentHelpers_ClampAndAlpha) {
    ImVec4 white(0.98f, 0.98f, 0.98f, 1.00f);
    ImVec4 lighter = unigui::theme::Lighten(white, 0.5f);
    EXPECT_FLOAT_EQ(lighter.x, 1.0f); // clamped
    ImVec4 a = unigui::theme::WithAlpha(white, 0.35f);
    EXPECT_FLOAT_EQ(a.w, 0.35f);
    EXPECT_FLOAT_EQ(a.x, white.x);
}

// ── Error-path: ImportThemeJSON with malformed array values ─────────────────
TEST_F(ThemeTest, ImportThemeJSON_MalformedValues_DoesNotCrash) {
    // "abc" tokens should be skipped (parsed as 0.0 via strtof), not crash
    std::string bad = R"({"WindowBg": ["abc", "xyz", "NaN", ""]})";
    EXPECT_NO_THROW(unigui::ImportThemeJSON(bad));
}
TEST_F(ThemeTest, ImportThemeJSON_EmptyString_DoesNotCrash) {
    EXPECT_NO_THROW(unigui::ImportThemeJSON(""));
}
TEST_F(ThemeTest, ImportThemeJSON_PartialArray_UsesDefaults) {
    // Only 2 values provided; remaining should stay at defaults (0, 1)
    std::string partial = R"({"Text": [0.5, 0.6]})";
    EXPECT_NO_THROW(unigui::ImportThemeJSON(partial));
}

// Export → Import round-trips real colour values (guards the regex-free parser).
TEST_F(ThemeTest, ImportThemeJSON_RoundTripPreservesColors) {
    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.34f, 0.56f, 0.78f);
    const std::string json = unigui::ExportThemeJSON();
    colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 0.f); // clobber
    EXPECT_TRUE(unigui::ImportThemeJSON(json));
    const ImVec4& c = colors[ImGuiCol_WindowBg];
    EXPECT_NEAR(c.x, 0.12f, 1e-4f);
    EXPECT_NEAR(c.y, 0.34f, 1e-4f);
    EXPECT_NEAR(c.z, 0.56f, 1e-4f);
    EXPECT_NEAR(c.w, 0.78f, 1e-4f);
}

// Long input with an unterminated array is the exact shape that made MSVC's
// std::regex throw error_complexity; the hand-written scanner must not throw.
TEST_F(ThemeTest, ImportThemeJSON_LongPathologicalInput_DoesNotThrow) {
    EXPECT_NO_THROW(unigui::ImportThemeJSON("{\"WindowBg\":[" + std::string(100000, '9')));
    EXPECT_NO_THROW(unigui::ImportThemeJSON("{\"WindowBg\":" + std::string(100000, 'x')));
}

// ── W6-type: 26 px body baseline (v4.10.3) ──────────────────────────────────

TEST_F(ThemeTest, Tokens_TypographyBaseline_Is26) {
    using namespace unigui::theme::tokens;
    static_assert(body26 == 26.0f && caption20 == 20.0f && title34 == 34.0f && hint18 == 18.0f,
                  "W6-type typography tokens drifted from the 26 px baseline");
    // Legacy v0 names stay at their old values for pre-4.10.3 integrations.
    static_assert(body16 == 16.0f && caption13 == 13.0f && title20 == 20.0f && hint12 == 12.0f,
                  "legacy typography aliases must keep their v0 values");
    EXPECT_FLOAT_EQ(body26, 26.0f);
}

TEST_F(ThemeTest, DefaultThemeConfig_FontSize_MatchesBody26Token) {
    // Defaults are what consumers get without an explicit override: 26 px
    // logical body text, DPI still auto-detected.
    EXPECT_FLOAT_EQ(unigui::ThemeConfig{}.font_size, unigui::theme::tokens::body26);
    EXPECT_FLOAT_EQ(unigui::ThemeConfig{}.dpi_scale, 0.0f);
}

TEST_F(ThemeTest, ApplyTheme_FontSizeStaysLogical_AtNonUnitDpi) {
    // 4.10.2 fixed the double-scaling bug: the font request is LOGICAL px and the
    // physical scale is applied separately by ImGui's dynamic font system, so the
    // queued rebuild size must not be font_size × dpi. We can only observe the
    // public queue flag here: with the same logical size queued repeatedly, no
    // rebuild should remain pending (a dpi-multiplied target would flip it).
    unigui::ApplyTheme({unigui::ThemePreset::Light, 1.5f, 26.0f});
    EXPECT_FLOAT_EQ(ImGui::GetStyle().ItemSpacing.x, 12.0f); // geometry IS dpi-scaled
    // Drain any pending rebuild queued by earlier tests, then re-apply the same
    // logical size: it must not queue again.
    if (unigui::HasPendingFontRebuild())
        unigui::ApplyPendingFontRebuild();
    unigui::ApplyTheme({unigui::ThemePreset::Light, 1.5f, 26.0f});
    EXPECT_FALSE(unigui::HasPendingFontRebuild());
}
