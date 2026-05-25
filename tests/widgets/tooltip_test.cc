#include <unigui/unigui.h>
#include <unigui/widgets/tooltip.h>
#include <imgui.h>
#include <gtest/gtest.h>
class TooltipTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(TooltipTest, Show_DoesNotCrash) { unigui::Tooltip::Show("Help text"); }
TEST_F(TooltipTest, Show_EmptyString_DoesNotCrash) { unigui::Tooltip::Show(""); }
