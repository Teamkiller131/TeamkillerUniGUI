#include <unigui/unigui.h>
#include <unigui/widgets/treeview.h>
#include <imgui.h>
#include <gtest/gtest.h>
class TreeViewTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(TreeViewTest, Render_DoesNotCrash) { unigui::TreeView tv("tv"); tv.Render(); }
TEST_F(TreeViewTest, SetRoot_Works) {
    unigui::TreeView tv("tv");
    tv.SetRoot({"Root", {{"Child1", {}}, {"Child2", {}}}});
    tv.Render();
}
