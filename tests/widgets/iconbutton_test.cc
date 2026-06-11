#include <unigui/unigui.h>
#include <unigui/widgets/iconbutton.h>

#include <imgui.h>

#include <gtest/gtest.h>
class IconButtonTest : public ::testing::Test {
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
TEST_F(IconButtonTest, Render_DoesNotCrash) {
    unigui::IconButton ib("ib", "★", "Star");
    ib.Render();
}
TEST_F(IconButtonTest, WasClicked_DefaultsFalse) {
    unigui::IconButton ib("ib", "★");
    EXPECT_FALSE(ib.WasClicked());
}
