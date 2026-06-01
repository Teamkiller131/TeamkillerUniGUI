#include <unigui/widgets/treeview.h>
#include <imgui.h>
#include <algorithm>
namespace unigui {
TreeView::TreeView(std::string name) : Widget(std::move(name)) {}
void TreeView::Render() {
    if (!IsVisible() || root_.label.empty()) return;
    ImGui::PushID(GetName().c_str());
    nodeCounter_ = 0; selected_.clear();
    if (hideRoot_) {
        for (auto& child : root_.children) RenderNode(child, 0);
    } else {
        RenderNode(root_, 0);
    }
    ImGui::PopID();
}
void TreeView::SetRoot(TreeNode root) { root_ = std::move(root); }
const TreeNode& TreeView::GetRoot() const { return root_; }
void TreeView::SetMultiSelect(bool on) { multiSelect_ = on; }
std::vector<int> TreeView::GetSelectedNodes() const { return selected_; }
void TreeView::SetNodeRenderer(std::function<void(int,int,const TreeNode&)> fn) { nodeRenderer_ = std::move(fn); }
void TreeView::SetRowRenderer(std::function<void(int,int,const TreeNode&,bool)> fn) { rowRenderer_ = std::move(fn); }
void TreeView::RenderNode(TreeNode& node, int depth) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (node.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    int id = nodeCounter_++;
    bool isSelected = std::find(selected_.begin(), selected_.end(), id) != selected_.end();
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

    // ── background fill (before TreeNodeEx) ───────────────────────────
    if (node.bgColor != 0) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float h = ImGui::GetTextLineHeightWithSpacing();
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + h),
            node.bgColor);
    }

    // ── selected highlight ────────────────────────────────────────────
    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
    }

    bool open = false;
    if (rowRenderer_) {
        // Use invisible label; row renderer draws everything
        open = ImGui::TreeNodeEx("##node", flags);
        if (open) rowRenderer_(id, depth, node, isSelected);
    } else {
        // Default rendering with icon/suffix/progress
        std::string displayLabel;
        if (!node.icon.empty()) displayLabel += node.icon + " ";
        displayLabel += node.label;

        // push label color if set
        if (node.labelColor != 0)
            ImGui::PushStyleColor(ImGuiCol_Text, node.labelColor);

        open = ImGui::TreeNodeEx(displayLabel.c_str(), flags);

        if (node.labelColor != 0)
            ImGui::PopStyleColor();

        if (open) {
            // suffix
            if (!node.suffix.empty()) {
                ImGui::SameLine();
                ImGui::TextUnformatted(node.suffix.c_str());
            }
            // progress bar
            if (node.progress >= 0.0f) {
                ImGui::SameLine();
                float barW = ImGui::GetContentRegionAvail().x * 0.3f;
                if (node.progressColor != 0)
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, node.progressColor);
                ImGui::ProgressBar(node.progress, ImVec2(barW, 0.0f), "");
                if (node.progressColor != 0)
                    ImGui::PopStyleColor();
            }
            // legacy node renderer
            if (nodeRenderer_) nodeRenderer_(id, depth, node);
        }
    }

    if (isSelected)
        ImGui::PopStyleColor();

    if (open) {
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            if (multiSelect_) {
                auto it = std::find(selected_.begin(), selected_.end(), id);
                if (it != selected_.end()) selected_.erase(it); else selected_.push_back(id);
            } else { selected_ = {id}; }
        }
        for (auto& child : node.children) RenderNode(child, depth + 1);
        ImGui::TreePop();
    }
}
}
