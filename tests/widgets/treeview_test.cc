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
TEST_F(TreeViewTest, RowRenderer_DoesNotCrash) {
    unigui::TreeView tv("tv");
    tv.SetRoot({"Root", {{"A", {}}, {"B", {}}}});
    tv.SetRowRenderer([](int id, int depth, const unigui::TreeNode& node, bool sel) {
        ImGui::Text("%s", node.label.c_str());
    });
    tv.Render();
}
TEST_F(TreeViewTest, EnhancedFields_Render) {
    unigui::TreeView tv("tv");
    unigui::TreeNode root;
    root.label = "Root";
    root.icon = "\xf0\x9f\x93\x81";   // folder emoji
    root.suffix = "(3)";
    root.labelColor = IM_COL32(255, 0, 0, 255);
    root.bgColor = IM_COL32(40, 40, 40, 255);
    root.progress = 0.75f;
    root.progressColor = IM_COL32(0, 255, 0, 255);
    root.children.push_back({"Child", {}});
    tv.SetRoot(std::move(root));
    tv.Render();
}
