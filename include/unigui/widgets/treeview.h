#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {
struct TreeNode {
    std::string label;
    std::vector<TreeNode> children;
    bool expanded = false;

    // ── Row rendering fields ──────────────────────────────────────────
    std::string icon;             // Unicode/Nerd Font icon char before label
    std::string suffix;           // text after label (e.g. percentage)
    ImU32 labelColor = 0;         // label color (0 = default theme)
    ImU32 bgColor = 0;            // row background (0 = none)
    float progress = -1.0f;       // progress bar (0~1, <0 = no bar)
    ImU32 progressColor = 0;      // progress bar color
};
class TreeView : public Widget {
public:
    TreeView(std::string name);
    void Render() override;
    void SetRoot(TreeNode root);
    const TreeNode& GetRoot() const;
    void SetMultiSelect(bool on);
    std::vector<int> GetSelectedNodes() const;
    void SetHideRoot(bool on) { hideRoot_ = on; }
    void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode& node)> fn);


    /// Legacy: custom node renderer called INSIDE TreeNode (after expand arrow + label).\n    /// Prefer SetRowRenderer for full row control.\n    void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode& node)> fn);\n\n    /// Custom row renderer: renders the ENTIRE row content (inside TreeNodeEx).
    /// TreeView manages expand/collapse/select. If NOT set, default rendering
    /// uses icon/suffix/progress/color fields on TreeNode.
    void SetRowRenderer(std::function<void(int id, int depth, const TreeNode& node, bool selected)> fn);

private:
    TreeNode root_;
    bool multiSelect_ = false;
    std::vector<int> selected_;
    int nodeCounter_ = 0;
    bool hideRoot_ = false;
    std::function<void(int,int,const TreeNode&)> nodeRenderer_;
    std::function<void(int,int,const TreeNode&,bool)> rowRenderer_;
    void RenderNode(TreeNode& node, int depth);
};
}
