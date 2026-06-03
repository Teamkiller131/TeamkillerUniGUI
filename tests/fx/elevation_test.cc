#include <unigui/fx/elevation.h>
#include <imgui.h>
#include <gtest/gtest.h>

using unigui::fx::Elevation;
using unigui::fx::ElevationTokens;
using unigui::fx::ElevationPreset;
using unigui::fx::ElevationName;
using unigui::theme::SurfaceStyle;

TEST(ElevationTest, HigherLevelMeansBiggerShadow) {
    auto low  = ElevationPreset(Elevation::Low,    SurfaceStyle::Solid);
    auto med  = ElevationPreset(Elevation::Medium, SurfaceStyle::Solid);
    auto high = ElevationPreset(Elevation::High,   SurfaceStyle::Solid);
    EXPECT_LT(low.shadow_radius,  med.shadow_radius);
    EXPECT_LT(med.shadow_radius,  high.shadow_radius);
    EXPECT_LT(low.shadow_offset_y, high.shadow_offset_y);
    EXPECT_LE(low.shadow_alpha,    high.shadow_alpha);
}

TEST(ElevationTest, NoneHasNoShadowOrGlow) {
    auto t = ElevationPreset(Elevation::None, SurfaceStyle::Glass);
    EXPECT_FLOAT_EQ(t.shadow_radius, 0.0f);
    EXPECT_EQ(t.shadow_alpha, 0);
    EXPECT_FALSE(t.rim_glow);
}

TEST(ElevationTest, GlassIsSofterAndHasRim) {
    auto glass = ElevationPreset(Elevation::Medium, SurfaceStyle::Glass);
    auto solid = ElevationPreset(Elevation::Medium, SurfaceStyle::Solid);
    // Glass shadow is softer (lower alpha) but wider radius, and adds a rim glow.
    EXPECT_LT(glass.shadow_alpha, solid.shadow_alpha);
    EXPECT_GT(glass.shadow_radius, solid.shadow_radius);
    EXPECT_TRUE(glass.rim_glow);
    EXPECT_GT(glass.glow_alpha, 0);
    EXPECT_FALSE(solid.rim_glow);
}

TEST(ElevationTest, FrostedAndAcrylicAlsoGetRim) {
    EXPECT_TRUE(ElevationPreset(Elevation::Low, SurfaceStyle::Frosted).rim_glow);
    EXPECT_TRUE(ElevationPreset(Elevation::Low, SurfaceStyle::Acrylic).rim_glow);
}

TEST(ElevationTest, MinimalIsQuietest) {
    auto minimal = ElevationPreset(Elevation::High, SurfaceStyle::Minimal);
    auto solid   = ElevationPreset(Elevation::High, SurfaceStyle::Solid);
    EXPECT_LT(minimal.shadow_alpha, solid.shadow_alpha);
    EXPECT_FALSE(minimal.rim_glow);
}

TEST(ElevationTest, AlphaStaysInByteRange) {
    for (auto lvl : {Elevation::None, Elevation::Low, Elevation::Medium, Elevation::High}) {
        for (auto surf : {SurfaceStyle::Solid, SurfaceStyle::Glass, SurfaceStyle::Frosted,
                          SurfaceStyle::Acrylic, SurfaceStyle::Minimal}) {
            auto t = ElevationPreset(lvl, surf);
            EXPECT_GE(t.shadow_alpha, 0);
            EXPECT_LE(t.shadow_alpha, 255);
            EXPECT_GE(t.glow_alpha, 0);
            EXPECT_LE(t.glow_alpha, 255);
        }
    }
}

TEST(ElevationTest, ActiveSurfaceOverloadFollowsTheme) {
    // The active surface defaults to Glass, so the level-only overload should
    // match the explicit Glass preset.
    ImGui::CreateContext();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    unigui::theme::ApplySurfaceStyle(ImGui::GetStyle(), SurfaceStyle::Glass);
    auto active = ElevationPreset(Elevation::Medium);
    auto glass  = ElevationPreset(Elevation::Medium, SurfaceStyle::Glass);
    EXPECT_FLOAT_EQ(active.shadow_radius, glass.shadow_radius);
    EXPECT_EQ(active.rim_glow, glass.rim_glow);

    unigui::theme::ApplySurfaceStyle(ImGui::GetStyle(), SurfaceStyle::Solid);
    auto activeSolid = ElevationPreset(Elevation::Medium);
    EXPECT_FALSE(activeSolid.rim_glow);
    ImGui::DestroyContext();
}

TEST(ElevationTest, NamesAreStable) {
    EXPECT_STREQ(ElevationName(Elevation::None),   "None");
    EXPECT_STREQ(ElevationName(Elevation::Low),    "Low");
    EXPECT_STREQ(ElevationName(Elevation::Medium), "Medium");
    EXPECT_STREQ(ElevationName(Elevation::High),   "High");
}

TEST(ElevationTest, MakeShadowConstructs) {
    // Constructing the effect object must not require a draw list.
    auto sh = unigui::fx::MakeElevationShadow(Elevation::High, SurfaceStyle::Solid);
    (void)sh;
    auto gl = unigui::fx::MakeElevationGlow(Elevation::High, SurfaceStyle::Glass);
    (void)gl;
    SUCCEED();
}
