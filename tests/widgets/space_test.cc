#include <unigui/unigui.h>
#include <unigui/widgets/space.h>
#include <imgui.h>
#include <gtest/gtest.h>
class DockSpaceTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(DockSpaceTest, Render_DoesNotCrash) { unigui::DockSpace ds("ds"); ds.Render(); }
