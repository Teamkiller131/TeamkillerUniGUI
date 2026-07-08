#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// MultiSplitter — N-panel resizable layout with drag handles.
/// Panels are added via AddPanel(ratio, content). Ratios sum to 1.0.
class MultiSplitter : public FluentWidget<MultiSplitter> {
public:
    enum Orientation { Horizontal, Vertical };

    MultiSplitter(std::string name, Orientation ori = Horizontal);

    /// A panel definition for the idempotent Configure() call.
    struct PanelDef {
        float ratio;
        std::function<void()> content;
        float minPx = 0.f; ///< minimum size along the split axis (0 = none)
    };

    void Render() override;
    void AddPanel(float ratio, std::function<void()> content);
    /// AddPanel with a minimum pixel size along the split axis.
    void AddPanel(float ratio, std::function<void()> content, float minPx);
    /// Idempotently populate panels: rebuilds (and records design ratios) only on
    /// the first call or when the panel count changes, so callers no longer need a
    /// `static bool` first-frame guard around AddPanel/SetDesignRatios.
    void Configure(std::vector<PanelDef> defs);
    /// True once panels have been added/configured.
    bool IsConfigured() const { return !panels_.empty(); }
    std::vector<float> GetRatios() const;
    void SetRatios(const std::vector<float>& ratios);

    /// Store the "design" (intended) ratios for this splitter.
    /// These are NOT applied immediately — call ResetToDesign() for that.
    void SetDesignRatios(const std::vector<float>& ratios) { designRatios_ = ratios; }
    const std::vector<float>& GetDesignRatios() const { return designRatios_; }
    /// Reset current runtime ratios back to the stored design ratios.
    void ResetToDesign();

    /// Serialize the current normalized ratios to a compact string (e.g.
    /// "0.30,0.44,0.26") for layout persistence — pair with a LayoutStore.
    std::string SerializeLayout() const;
    /// Restore ratios from a string produced by SerializeLayout(). Applies only
    /// when the value count matches the current panel count (so a stale/short
    /// layout for a different splitter is ignored). Returns true if applied.
    /// Non-throwing on malformed input.
    bool RestoreLayout(const std::string& s);

    // ── Fluent (chainable) helpers — return MultiSplitter& via CRTP base ──────────
    MultiSplitter& WithRatios(const std::vector<float>& ratios) {
        SetRatios(ratios);
        return *this;
    }
    MultiSplitter& WithDesignRatios(const std::vector<float>& ratios) {
        SetDesignRatios(ratios);
        return *this;
    }

private:
    Orientation ori_;
    struct Panel {
        float ratio;
        std::function<void()> content;
        float minPx = 0.f;
    };
    std::vector<Panel> panels_;
    std::vector<float> designRatios_;
};

} // namespace unigui
