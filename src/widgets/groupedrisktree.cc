#include <unigui/theme/color_tokens.h>
#include <unigui/widgets/groupedrisktree.h>
#include <unigui/widgets/pnltext.h> // GradedRole

#include <imgui.h>

#include <algorithm>
#include <cstdio>

namespace unigui {

GroupedRiskTree::GroupedRiskTree(std::string name)
        : FluentWidget<GroupedRiskTree>(std::move(name))
        , tree_(GetName() + "_tree") {}

double GroupedRiskTree::ComputeRatio(const RiskNode& n, Rollup r) {
    if (n.children.empty())
        return n.ratio;
    double worst = 0.0, sum = 0.0;
    int cnt = 0;
    for (const auto& c : n.children) {
        const double cr = ComputeRatio(c, r);
        worst = std::max(worst, cr);
        sum += cr;
        ++cnt;
    }
    switch (r) {
    case Rollup::Worst:
        return worst;
    case Rollup::Mean:
        return cnt > 0 ? sum / cnt : 0.0;
    case Rollup::Sum:
        return sum;
    }
    return worst;
}

TreeNode GroupedRiskTree::BuildNode(const RiskNode& rn) const {
    TreeNode tn;
    tn.label = rn.label;
    tn.expanded = true;

    const double ratio = ComputeRatio(rn, rollup_);
    const float clamped = static_cast<float>(ratio < 0.0 ? 0.0 : (ratio > 1.0 ? 1.0 : ratio));
    tn.progress = clamped;
    tn.progressColor =
        ImGui::GetColorU32(theme::GetSemanticColor(GradedRole(ratio, warn_, danger_)));

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.0f%%", ratio * 100.0);
    tn.suffix = buf;

    for (const auto& c : rn.children)
        tn.children.push_back(BuildNode(c));
    return tn;
}

void GroupedRiskTree::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());
    tree_.SetHideRoot(hideRoot_);
    tree_.SetRoot(BuildNode(root_));
    tree_.Render();
    ImGui::PopID();
}

} // namespace unigui
