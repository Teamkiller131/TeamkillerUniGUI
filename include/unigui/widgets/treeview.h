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
private:
    TreeNode root_;
    void RenderNode(TreeNode& node);
};
}
