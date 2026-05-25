#include <unigui/unigui.h>
#include <unigui/widgets/loadingindicator.h>
#include <imgui.h>
#include <gtest/gtest.h>
class LoadingIndicatorTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(LoadingIndicatorTest, Render_DoesNotCrash) { unigui::LoadingIndicator li("li"); li.Render(); }
TEST_F(LoadingIndicatorTest, SetActive_False_StopsRendering) { unigui::LoadingIndicator li("li"); li.SetActive(false); li.Render(); }
