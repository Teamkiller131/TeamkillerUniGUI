#include <unigui/unigui.h>
#include <unigui/widgets/groupbox.h>
#include <imgui.h>
#include <gtest/gtest.h>
class GroupBoxTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize = ImVec2(800, 600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(GroupBoxTest, Render_DoesNotCrash) {
    unigui::GroupBox gb("gb", "Settings");
    gb.Render();
}
TEST_F(GroupBoxTest, ContentCallback_IsCalled) {
    unigui::GroupBox gb("gb", "Group");
    bool called = false;
    gb.SetContentCallback([&]() { called = true; });
    gb.Render();
    EXPECT_TRUE(called);
}
TEST_F(GroupBoxTest, SetTitle_Works) {
    unigui::GroupBox gb("gb", "Old");
    gb.SetTitle("New");
    gb.Render(); // Just verify no crash
}
