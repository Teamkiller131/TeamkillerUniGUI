#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <vector>

namespace unigui {

/// MultiSplitter — N-panel resizable layout with drag handles.
/// Panels are added via AddPanel(ratio, content). Ratios sum to 1.0.
class MultiSplitter : public Widget {
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

private:
    Orientation ori_;
    struct Panel {
        float ratio;
        std::function<void()> content;
        float minPx = 0.f;
    };
    std::vector<Panel> panels_;
    int dragIndex_ = -1;
    std::vector<float> designRatios_;
};

} // namespace unigui
