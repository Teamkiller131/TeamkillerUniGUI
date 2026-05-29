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

    /// Custom node renderer: called for each node WITHIN the TreeNode
    /// after the expand arrow and label.  int=nodeId, int=depth.
    void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode& node)> fn);

private:
    TreeNode root_;
    bool multiSelect_ = false;
    std::vector<int> selected_;
    int nodeCounter_ = 0;
    bool hideRoot_ = false;
    std::function<void(int,int,const TreeNode&)> nodeRenderer_;
    void RenderNode(TreeNode& node, int depth);
};
}
