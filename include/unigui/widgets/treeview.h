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
private:
    TreeNode root_;
    bool multiSelect_ = false;
    std::vector<int> selected_;
    int nodeCounter_ = 0;
    void RenderNode(TreeNode& node, int depth);
};
}
