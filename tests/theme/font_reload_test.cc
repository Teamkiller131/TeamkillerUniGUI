#include <unigui/theme/theme.h>
#include <imgui.h>
#include <gtest/gtest.h>
TEST(ThemeFontReload, QueuesRebuildOnChange) {
    ImGui::CreateContext();
    EXPECT_FALSE(unigui::HasPendingFontRebuild());
    unigui::ThemeConfig tc;tc.font_size=16.f;unigui::ApplyTheme(tc);
    EXPECT_FALSE(unigui::HasPendingFontRebuild());
    tc.font_size=24.f;unigui::ApplyTheme(tc);
    EXPECT_TRUE(unigui::HasPendingFontRebuild());
    ImGui::DestroyContext();
}
