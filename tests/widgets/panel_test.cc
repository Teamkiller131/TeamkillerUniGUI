#include <unigui/unigui.h>
#include <unigui/widgets/panel.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>

class PanelTest : public ::testing::Test {
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
    }
};

TEST_F(PanelTest, Render_DoesNotCrash) {
    unigui::Panel panel("pnl", "My Panel");
    panel.Render();
}

TEST_F(PanelTest, GetTitle_ReturnsGivenTitle) {
    unigui::Panel panel("pnl", "Settings");
    EXPECT_EQ(panel.GetTitle(), "Settings");
}

TEST_F(PanelTest, SetTitle_UpdatesTitle) {
    unigui::Panel panel("pnl", "Settings");
    panel.SetTitle("Advanced");
    EXPECT_EQ(panel.GetTitle(), "Advanced");
}

TEST_F(PanelTest, ContentCallback_IsCalled) {
    unigui::Panel panel("pnl", "Panel");
    bool called = false;
    panel.SetContentCallback([&called]() { called = true; });
    panel.Render();
    EXPECT_TRUE(called);
}

TEST_F(PanelTest, Hidden_DoesNotRender) {
    unigui::Panel panel("pnl", "Hidden");
    panel.Hide();
    panel.Render(); // Should not crash
}

TEST_F(PanelTest, IsVisible_DefaultsToTrue) {
    unigui::Panel panel("pnl", "Visible");
    EXPECT_TRUE(panel.IsVisible());
}
