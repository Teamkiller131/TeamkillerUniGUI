#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

class CascadingCombo : public Widget {
public:
    // Arrangement of the per-level combo boxes.
    enum class Layout {
        Vertical,  // stacked top-to-bottom (default)
        Horizontal // placed left-to-right on one line
    };

    struct Level {
        std::string label;
        std::vector<std::string> options;
        int selectedIndex = 0;
        float width = 0.f; // per-level width override in px (0 = use global item width)
    };

    CascadingCombo(std::string name, std::vector<Level> levels = {});
    void Render() override;
    void SetLevels(std::vector<Level> levels);
    void SetOptions(int level, std::vector<std::string> options);
    int GetSelectedIndex(int level) const;
    std::string GetSelectedText(int level) const;

    // ── Layout & width ─────────────────────────────────────────────────
    void SetLayout(Layout layout);
    Layout GetLayout() const { return layout_; }
    // Global default width (px) applied to every combo. <=0 means use the
    // ImGui default sizing. A per-level Level::width overrides this.
    void SetItemWidth(float width);
    float GetItemWidth() const { return itemWidth_; }
    // Per-level width override (px). <=0 clears the override.
    void SetItemWidth(int level, float width);
    // Gap (px) between combos in Horizontal layout. <0 uses ImGui default
    // item spacing.
    void SetSpacing(float spacing);
    float GetSpacing() const { return spacing_; }

    // Visible caption control. By default each level's `label` is NOT drawn as a
    // trailing text caption next to the combo (that is visually noisy and the
    // current selection already conveys the control's purpose). When hidden, the
    // label is instead surfaced as a hover tooltip. Call SetShowLabels(true) to
    // restore the classic trailing-caption behaviour.
    void SetShowLabels(bool on) { showLabels_ = on; }
    bool GetShowLabels() const { return showLabels_; }

    // ── Fluent configuration (chainable) ───────────────────────────────
    CascadingCombo& WithLayout(Layout layout) {
        SetLayout(layout);
        return *this;
    }
    CascadingCombo& WithItemWidth(float width) {
        SetItemWidth(width);
        return *this;
    }
    CascadingCombo& WithSpacing(float spacing) {
        SetSpacing(spacing);
        return *this;
    }
    CascadingCombo& WithShowLabels(bool on) {
        SetShowLabels(on);
        return *this;
    }

    using OnChanged = std::function<void(int level, int index)>;
    void SetOnChanged(OnChanged fn);
    /// Render levels horizontally (SameLine between combos) instead of stacked.
    /// Convenience wrapper around SetLayout(Layout::Horizontal|Vertical).
    void SetHorizontal(bool on) { SetLayout(on ? Layout::Horizontal : Layout::Vertical); }

private:
    std::vector<Level> levels_;
    OnChanged onChanged_;
    Layout layout_ = Layout::Vertical;
    float itemWidth_ = 0.f;
    float spacing_ = -1.f;
    bool showLabels_ = false;
};

} // namespace unigui
