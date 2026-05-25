#include <unigui/unigui.h>
#include <unigui/widgets/imagebutton.h>
#include <imgui.h>
#include <gtest/gtest.h>
class ImageButtonTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(ImageButtonTest, Render_DoesNotCrash) { unigui::ImageButton ib("ib"); ib.Render(); }
TEST_F(ImageButtonTest, WasClicked_DefaultsFalse) { unigui::ImageButton ib("ib", "Click"); EXPECT_FALSE(ib.WasClicked()); ib.Render(); }
