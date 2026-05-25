#include <unigui/widgets/treeview.h>
#include <imgui.h>
namespace unigui {
TreeView::TreeView(std::string name) : Widget(std::move(name)) {}
void TreeView::Render() { if (!IsVisible() || root_.label.empty()) return; RenderNode(root_); }
void TreeView::SetRoot(TreeNode root) { root_ = std::move(root); }
const TreeNode& TreeView::GetRoot() const { return root_; }
void TreeView::RenderNode(TreeNode& node) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (node.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (ImGui::TreeNodeEx(node.label.c_str(), flags)) {
        for (auto& child : node.children) RenderNode(child);
        ImGui::TreePop();
    }
}
}
