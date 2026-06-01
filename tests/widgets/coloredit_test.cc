#include <unigui/unigui.h>
#include <unigui/widgets/coloredit.h>
#include <imgui.h>
#include <gtest/gtest.h>

class ColorEditTest : public ::testing::Test {
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

TEST_F(ColorEditTest, Defaults) {
    unigui::ColorEdit ce("ce", "Color");
    ImVec4 c = ce.GetColor();
    EXPECT_FLOAT_EQ(c.x, 1.0f); // r
    EXPECT_FLOAT_EQ(c.y, 1.0f); // g
    EXPECT_FLOAT_EQ(c.z, 1.0f); // b
    EXPECT_FLOAT_EQ(c.w, 1.0f); // a
    EXPECT_FALSE(ce.WasChanged());
    EXPECT_EQ(ce.GetLabel(), "Color");
}

TEST_F(ColorEditTest, SetGet) {
    unigui::ColorEdit ce("ce", "Color", 0.2f, 0.4f, 0.6f, 0.8f);
    ImVec4 c = ce.GetColor();
    EXPECT_FLOAT_EQ(c.x, 0.2f);
    EXPECT_FLOAT_EQ(c.y, 0.4f);
    EXPECT_FLOAT_EQ(c.z, 0.6f);
    EXPECT_FLOAT_EQ(c.w, 0.8f);

    ce.SetColor(0.1f, 0.3f, 0.5f, 0.7f);
    c = ce.GetColor();
    EXPECT_FLOAT_EQ(c.x, 0.1f);
    EXPECT_FLOAT_EQ(c.y, 0.3f);
    EXPECT_FLOAT_EQ(c.z, 0.5f);
    EXPECT_FLOAT_EQ(c.w, 0.7f);

    // default alpha = 1.0
    ce.SetColor(1.0f, 0.0f, 0.0f);
    c = ce.GetColor();
    EXPECT_FLOAT_EQ(c.w, 1.0f);
}

TEST_F(ColorEditTest, Render_DoesNotCrash) {
    unigui::ColorEdit ce("ce", "Color", 1.0f, 0.5f, 0.0f, 1.0f);
    ce.Render();
}

TEST_F(ColorEditTest, Render_RespectsVisibility) {
    unigui::ColorEdit ce("ce", "Color", 0.5f, 0.5f, 0.5f, 1.0f);
    ce.Hide();
    ce.Render();
    EXPECT_FALSE(ce.WasChanged());
}

TEST_F(ColorEditTest, WasChanged_DefaultsToFalse) {
    unigui::ColorEdit ce("ce", "Color");
    EXPECT_FALSE(ce.WasChanged());
}
