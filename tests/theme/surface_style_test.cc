#include <unigui/theme/surface_style.h>
#include <imgui.h>
#include <gtest/gtest.h>

using unigui::theme::SurfaceStyle;
using unigui::theme::SurfaceTokens;
using unigui::theme::SurfacePreset;
using unigui::theme::SurfaceStyleName;
using unigui::theme::AllSurfaceStyles;
using unigui::theme::ApplySurfaceStyle;

class SurfaceStyleTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& c = ImGui::GetStyle().Colors;
        // Opaque baseline palette so alpha multipliers are easy to verify.
        c[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
        c[ImGuiCol_ChildBg]  = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        c[ImGuiCol_PopupBg]  = ImVec4(0.14f, 0.14f, 0.16f, 1.0f);
        c[ImGuiCol_FrameBg]  = ImVec4(0.16f, 0.16f, 0.18f, 1.0f);
        c[ImGuiCol_Border]   = ImVec4(0.20f, 0.20f, 0.24f, 1.0f);
    }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(SurfaceStyleTest, GlassIsTranslucentAndKeepsRim) {
    ApplySurfaceStyle(ImGui::GetStyle(), SurfaceStyle::Glass);
    const auto& c = ImGui::GetStyle().Colors;
    // Surfaces become translucent.
    EXPECT_LT(c[ImGuiCol_WindowBg].w, 1.0f);
    EXPECT_LT(c[ImGuiCol_ChildBg].w, 1.0f);
    EXPECT_LT(c[ImGuiCol_FrameBg].w, 1.0f);
    // Frames are the most see-through layer in the glass preset.
    EXPECT_LT(c[ImGuiCol_FrameBg].w, c[ImGuiCol_WindowBg].w);
    // Border still visible (rim), not fully erased.
    EXPECT_GT(c[ImGuiCol_Border].w, 0.0f);
}

TEST_F(SurfaceStyleTest, SolidLeavesSurfacesOpaque) {
    ApplySurfaceStyle(ImGui::GetStyle(), SurfaceStyle::Solid);
    const auto& c = ImGui::GetStyle().Colors;
    EXPECT_FLOAT_EQ(c[ImGuiCol_WindowBg].w, 1.0f);
    EXPECT_FLOAT_EQ(c[ImGuiCol_ChildBg].w, 1.0f);
    EXPECT_FLOAT_EQ(c[ImGuiCol_PopupBg].w, 1.0f);
    EXPECT_FLOAT_EQ(c[ImGuiCol_FrameBg].w, 1.0f);
}

TEST_F(SurfaceStyleTest, MinimalIsBorderless) {
    ApplySurfaceStyle(ImGui::GetStyle(), SurfaceStyle::Minimal);
    auto& s = ImGui::GetStyle();
    EXPECT_FLOAT_EQ(s.WindowBorderSize, 0.0f);
    EXPECT_FLOAT_EQ(s.FrameBorderSize, 0.0f);
    EXPECT_FLOAT_EQ(s.Colors[ImGuiCol_Border].w, 0.0f);
}

TEST_F(SurfaceStyleTest, FrostedIsMoreTranslucentThanGlass) {
    EXPECT_LT(SurfacePreset(SurfaceStyle::Frosted).window_alpha,
              SurfacePreset(SurfaceStyle::Glass).window_alpha);
}

TEST_F(SurfaceStyleTest, AlphaMultiplierIsRelativeToExistingAlpha) {
    // Start from a half-transparent window; multiplier must compound, not replace.
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.5f;
    const float m = SurfacePreset(SurfaceStyle::Glass).window_alpha;
    ApplySurfaceStyle(ImGui::GetStyle(), SurfaceStyle::Glass);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w, 0.5f * m);
}

TEST_F(SurfaceStyleTest, AllStylesAreNamedAndDistinct) {
    const auto& all = AllSurfaceStyles();
    EXPECT_EQ(all.size(), 5u);
    for (auto st : all) {
        EXPECT_STRNE(SurfaceStyleName(st), "");
    }
    // Glass is the documented default name.
    EXPECT_STREQ(SurfaceStyleName(SurfaceStyle::Glass), "Glass");
}

TEST_F(SurfaceStyleTest, BackdropIsOpaque) {
    using unigui::theme::BackdropColor;
    ImVec4 wb(0.10f, 0.10f, 0.12f, 0.5f); // translucent input alpha must be ignored
    for (auto st : AllSurfaceStyles()) {
        EXPECT_FLOAT_EQ(BackdropColor(wb, st).w, 1.0f);
    }
}

TEST_F(SurfaceStyleTest, BackdropDarkensForGlassButNotSolid) {
    using unigui::theme::BackdropColor;
    ImVec4 wb(0.20f, 0.20f, 0.24f, 1.0f);
    // Glass backdrop is darker than the window background for contrast.
    EXPECT_LT(BackdropColor(wb, SurfaceStyle::Glass).x, wb.x);
    // Solid/Minimal keep the same tone (opaque materials need no contrast gap).
    EXPECT_FLOAT_EQ(BackdropColor(wb, SurfaceStyle::Solid).x, wb.x);
    EXPECT_FLOAT_EQ(BackdropColor(wb, SurfaceStyle::Minimal).x, wb.x);
}
