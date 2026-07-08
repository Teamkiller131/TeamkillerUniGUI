#pragma once

#include <unigui/widgets/treeview.h>
#include <unigui/widgets/widget_base.h>

#include <string>
#include <vector>

namespace unigui {

/// GroupedRiskTree — a hierarchical risk view (account group → account → leg)
/// that shows each node's utilisation ratio as a threshold-coloured bar and
/// rolls child ratios up into the parent row (worst / mean / sum). Built on the
/// existing `TreeView` (whose nodes already carry a `progress` bar), so it adds
/// the two things strategy tabs hand-roll: the **parent rollup** and the
/// **warn/danger colouring**. Caller supplies pre-computed leaf ratios and
/// labels; no unit/scaling assumptions are baked in (presentation-only).
class GroupedRiskTree : public FluentWidget<GroupedRiskTree> {
public:
    enum class Rollup { Worst, Mean, Sum };

    /// A node in the risk hierarchy. Leaves carry `ratio`; parents' ratios are
    /// derived from their children via the active Rollup.
    struct RiskNode {
        std::string label;
        double ratio = 0.0; // 0..1+ (1.0 == at limit)
        std::vector<RiskNode> children;
    };

    explicit GroupedRiskTree(std::string name);

    void Render() override;

    void SetData(RiskNode root) { root_ = std::move(root); }
    void SetRollup(Rollup r) { rollup_ = r; }
    Rollup GetRollup() const { return rollup_; }
    /// Ratio thresholds: < warn → success(green), < danger → warning, else danger.
    void SetThresholds(double warn, double danger) {
        warn_ = warn;
        danger_ = danger;
    }
    void SetHideRoot(bool on) { hideRoot_ = on; }

    /// Pure: the effective ratio of `n` under rollup `r` (leaf → its own ratio,
    /// parent → worst/mean/sum of children, recursively). Unit-testable.
    static double ComputeRatio(const RiskNode& n, Rollup r);

    // ── Fluent (chainable) helpers — return GroupedRiskTree& via CRTP base ──────────
    GroupedRiskTree& WithData(RiskNode root) {
        SetData(std::move(root));
        return *this;
    }
    GroupedRiskTree& WithRollup(Rollup r) {
        SetRollup(r);
        return *this;
    }
    GroupedRiskTree& WithThresholds(double warn, double danger) {
        SetThresholds(warn, danger);
        return *this;
    }
    GroupedRiskTree& WithHideRoot(bool on) {
        SetHideRoot(on);
        return *this;
    }

private:
    TreeNode BuildNode(const RiskNode& rn) const;

    RiskNode root_;
    Rollup rollup_ = Rollup::Worst;
    double warn_ = 0.7;
    double danger_ = 0.85;
    bool hideRoot_ = false;
    TreeView tree_;
};

} // namespace unigui
