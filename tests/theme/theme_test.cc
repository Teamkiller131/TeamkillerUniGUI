#include <unigui/unigui.h>
#include <unigui/theme/theme.h>
#include <unigui/theme/style_scope.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>

class ThemeTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }
    void TearDown() override {
        ImGui::DestroyContext();
    }
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
