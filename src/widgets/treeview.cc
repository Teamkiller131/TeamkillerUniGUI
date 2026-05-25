#include <unigui/widgets/treeview.h>
#include <imgui.h>
#include <algorithm>
namespace unigui {
TreeView::TreeView(std::string name) : Widget(std::move(name)) {}
void TreeView::Render() { if (!IsVisible() || root_.label.empty()) return; nodeCounter_ = 0; selected_.clear(); RenderNode(root_, 0); }
void TreeView::SetRoot(TreeNode root) { root_ = std::move(root); }
const TreeNode& TreeView::GetRoot() const { return root_; }
void TreeView::SetMultiSelect(bool on) { multiSelect_ = on; }
std::vector<int> TreeView::GetSelectedNodes() const { return selected_; }
void TreeView::RenderNode(TreeNode& node, int) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (node.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    int id = nodeCounter_++;
    bool isSelected = std::find(selected_.begin(), selected_.end(), id) != selected_.end();
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;
    if (ImGui::TreeNodeEx(node.label.c_str(), flags)) {
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            if (multiSelect_) {
                auto it = std::find(selected_.begin(), selected_.end(), id);
                if (it != selected_.end()) selected_.erase(it); else selected_.push_back(id);
            } else { selected_ = {id}; }
        }
        for (auto& child : node.children) RenderNode(child, 0);
        ImGui::TreePop();
    }
}
}
