#include <unigui/widgets/cascadingcombo.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstdio>

#include "detail/combo_chevron.h"

namespace unigui {

namespace {

float CalcComboWidth(const std::vector<std::string>& options, const char* preview) {
    float maxTextWidth = ImGui::CalcTextSize(preview ? preview : "").x;
    for (const auto& option : options)
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(option.c_str()).x);
    const ImGuiStyle& style = ImGui::GetStyle();
    return maxTextWidth + style.FramePadding.x * 2.0f + ImGui::GetFrameHeight() +
           style.ItemInnerSpacing.x + 4.0f;
}

} // namespace

CascadingCombo::CascadingCombo(std::string name, std::vector<Level> levels)
        : FluentWidget<CascadingCombo>(std::move(name))
        , levels_(std::move(levels)) {}

void CascadingCombo::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    for (int lvl = 0; lvl < (int) levels_.size(); ++lvl) {
        auto& level = levels_[lvl];
        if (layout_ == Layout::Horizontal && lvl > 0)
            ImGui::SameLine(0.0f, spacing_);
        float width = level.width > 0.f ? level.width : itemWidth_;
        int prev = level.selectedIndex;
        char lbl[80];
        // Hide the visible caption by default: a text label trailing every combo
        // is noisy. Keep a stable, unique ImGui ID via the "##" suffix; expose the
        // label as a hover tooltip instead (unless SetShowLabels(true)).
        if (showLabels_ && !level.label.empty())
            snprintf(lbl, sizeof(lbl), "%s##lvl%d", level.label.c_str(), lvl);
        else
            snprintf(lbl, sizeof(lbl), "##lvl%d", lvl);
        const char* preview =
            level.options.empty() ? "" : level.options[level.selectedIndex].c_str();
        ImGui::SetNextItemWidth(width > 0.f ? width : CalcComboWidth(level.options, preview));
        const auto comboFrame = detail::CaptureComboFrame();
        const bool comboOpen = ImGui::BeginCombo(lbl, preview, ImGuiComboFlags_NoArrowButton);
        const bool comboFocused = ImGui::IsItemFocused(); // capture before tooltip/items steal it
        detail::DrawComboChevron(comboFrame, comboOpen || ImGui::IsItemHovered());
        if (comboOpen) {
            for (int i = 0; i < (int) level.options.size(); ++i) {
                bool isSel = (i == level.selectedIndex);
                if (ImGui::Selectable(level.options[i].c_str(), isSel))
                    level.selectedIndex = i;
                if (isSel)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        // [2026-10-14] Mouse-wheel quick-select — bring this widget to parity with
        // im::Combo / ComboBox: hover the closed combo and scroll to cycle the
        // selection in place (trader workflow; AA 比价表级联下拉此前无此能力).
        // Hover = geometric rect check (works inside ScrollY tables / child windows,
        // where IsItemHovered's hover flag can be blocked). Disabled guard mirrors
        // im.cc v2 [2026-09-17]: read the item-flags stack top, NOT DisabledStackSize —
        // the latter misreads BeginDisabled(false) wrappers as disabled and kills
        // wheel-select on enabled combos.
        // [WHEEL-OVERLAY-20261029] 几何命中 + 真悬停窗口，避免被浮窗遮住时仍吃滚轮。
        const bool comboHovered =
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
                ImGui::IsMouseHoveringRect(
                        comboFrame.pos,
                        ImVec2(comboFrame.pos.x + comboFrame.width,
                               comboFrame.pos.y + comboFrame.height));
        if (!comboOpen && comboHovered && level.options.size() > 1 && IsEnabled()) {
            const bool itemDisabled =
                    (ImGui::GetCurrentContext()->ItemFlagsStack.back() & ImGuiItemFlags_Disabled) != 0;
            if (!itemDisabled) {
                ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
                const float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) {
                    int next = level.selectedIndex + (wheel > 0.0f ? -1 : 1);
                    next = next < 0 ? 0
                                    : (next >= (int) level.options.size()
                                           ? (int) level.options.size() - 1 : next);
                    if (next != level.selectedIndex)
                        level.selectedIndex = next;
                }
            }
        }
        if (!showLabels_ && !level.label.empty() && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", level.label.c_str());
        if (level.selectedIndex != prev && onChanged_)
            onChanged_(lvl, level.selectedIndex);
        ReportAccessible(a11y::Role::Combo, comboFocused,
                         preview ? std::string(preview) : std::string());
    }
    ImGui::PopID();
}

void CascadingCombo::SetLevels(std::vector<Level> levels) {
    levels_ = std::move(levels);
}
void CascadingCombo::SetOptions(int level, std::vector<std::string> options) {
    if (level >= 0 && level < (int) levels_.size()) {
        levels_[level].options = std::move(options);
        if (levels_[level].selectedIndex >= (int) levels_[level].options.size())
            levels_[level].selectedIndex = 0;
    }
}
int CascadingCombo::GetSelectedIndex(int level) const {
    return (level >= 0 && level < (int) levels_.size()) ? levels_[level].selectedIndex : -1;
}
std::string CascadingCombo::GetSelectedText(int level) const {
    if (level < 0 || level >= (int) levels_.size())
        return {};
    auto& l = levels_[level];
    return (l.selectedIndex >= 0 && l.selectedIndex < (int) l.options.size())
               ? l.options[l.selectedIndex]
               : std::string{};
}
void CascadingCombo::SetLayout(Layout layout) {
    layout_ = layout;
}
void CascadingCombo::SetItemWidth(float width) {
    itemWidth_ = width;
}
void CascadingCombo::SetItemWidth(int level, float width) {
    if (level >= 0 && level < (int) levels_.size())
        levels_[level].width = width > 0.f ? width : 0.f;
}
void CascadingCombo::SetSpacing(float spacing) {
    spacing_ = spacing;
}
void CascadingCombo::SetOnChanged(OnChanged fn) {
    onChanged_ = std::move(fn);
}

} // namespace unigui
