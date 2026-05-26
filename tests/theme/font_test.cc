#include <unigui/unigui.h>
#include <unigui/theme/theme.h>
#include <imgui.h>
#include <gtest/gtest.h>

class FontTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
    }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(FontTest, EmbeddedFont_LoadsSuccessfully) {
    unigui::LoadDefaultFont(18.0f);
    EXPECT_GE(ImGui::GetIO().Fonts->Fonts.Size, 1);
}

TEST_F(FontTest, CJKMerge_Optional) {
    unigui::LoadDefaultFont(16.0f);
    // CJK merge may or may not succeed depending on system fonts
    // Just verify no crash
    auto* atlas = ImGui::GetIO().Fonts;
    EXPECT_GE(atlas->Fonts.Size, 1); // At least the primary font
}
