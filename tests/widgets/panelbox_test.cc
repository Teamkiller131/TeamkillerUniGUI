#include <unigui/unigui.h>
#include <unigui/widgets/panelbox.h>
#include <imgui.h>
#include <gtest/gtest.h>

class PanelBoxTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};

TEST_F(PanelBoxTest, GetTitle_ReturnsTitle) {
    unigui::PanelBox pb("pb", "My Panel");
    EXPECT_EQ(pb.GetTitle(), "My Panel");
}

TEST_F(PanelBoxTest, SetTitle_Updates) {
    unigui::PanelBox pb("pb", "Old");
    pb.SetTitle("New");
    EXPECT_EQ(pb.GetTitle(), "New");
}

TEST_F(PanelBoxTest, Visibility_Toggle) {
    unigui::PanelBox pb("pb", "Panel");
    EXPECT_TRUE(pb.IsVisible());
    pb.Hide();
    EXPECT_FALSE(pb.IsVisible());
}
