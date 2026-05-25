#include <unigui/unigui.h>
#include <unigui/widgets/statusbar.h>
#include <imgui.h>
#include <gtest/gtest.h>
class StatusBarTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(StatusBarTest, Render_DoesNotCrash) { unigui::StatusBar sb("sb","Ready"); sb.Render(); }
TEST_F(StatusBarTest, SetText_Works) { unigui::StatusBar sb("sb"); sb.SetText("Done"); EXPECT_EQ(sb.GetText(), "Done"); }
