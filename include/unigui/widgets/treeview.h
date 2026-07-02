#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
/// A single colored run of text inside a node label. Lets callers color
/// individual segments (e.g. only the direction "多"/"空") instead of tinting
/// the whole label via labelColor.
struct TextSpan {
    std::string text;
    ImU32 color = 0; // 0 = default theme text color
};
struct TreeNode {
    std::string label;
    std::vector<TreeNode> children;
    bool expanded = false;

    // ── Row rendering fields ──────────────────────────────────────────
    std::string icon;        // Unicode/Nerd Font icon char before label
    std::string suffix;      // text after label (e.g. percentage)
    ImU32 labelColor = 0;    // label color (0 = default theme)
    ImU32 bgColor = 0;       // row background (0 = none)
    float progress = -1.0f;  // progress bar (0~1, <0 = no bar)
    ImU32 progressColor = 0; // progress bar color

    // When non-empty (and no rowRenderer is set), the node label is drawn as a
    // sequence of individually-colored spans instead of the single `label`.
    // The expand/select target stays the full row; `label` may be left as a
    // plain fallback. Spans are rendered inline, left to right.
    std::vector<TextSpan> spans;
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

    /// Legacy: custom node renderer called INSIDE the TreeNode (after the
    /// expand arrow + label). Prefer SetRowRenderer for full row control.
    void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode& node)> fn);

    /// Custom row renderer: renders the ENTIRE row content (inside TreeNodeEx).
    /// TreeView manages expand/collapse/select. If NOT set, default rendering
    /// uses icon/suffix/progress/color fields on TreeNode.
    void SetRowRenderer(
        std::function<void(int id, int depth, const TreeNode& node, bool selected)> fn);

private:
    TreeNode root_;
    bool multiSelect_ = false;
    std::vector<int> selected_;
    int nodeCounter_ = 0;
    bool hideRoot_ = false;
    // Label of the first selected node this frame, captured during RenderNode's walk —
    // the a11y container value (what a screen reader should say the tree is "at").
    std::string a11ySelectedLabel_;
    std::function<void(int, int, const TreeNode&)> nodeRenderer_;
    std::function<void(int, int, const TreeNode&, bool)> rowRenderer_;
    void RenderNode(TreeNode& node, int depth);
};
} // namespace unigui
