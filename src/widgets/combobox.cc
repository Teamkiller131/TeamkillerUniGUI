#include <unigui/widgets/combobox.h>

#include <imgui.h>

#include <algorithm>

#include "detail/combo_chevron.h"

namespace unigui {

namespace {

float CalcComboWidth(const std::vector<std::string>& items, const char* preview) {
    float maxTextWidth = ImGui::CalcTextSize(preview ? preview : "").x;
    for (const auto& item : items)
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(item.c_str()).x);
    const ImGuiStyle& style = ImGui::GetStyle();
    return maxTextWidth + style.FramePadding.x * 2.0f + ImGui::GetFrameHeight() +
           style.ItemInnerSpacing.x + 4.0f;
}

} // namespace

ComboBox::ComboBox(std::string name, std::string label, std::vector<std::string> items,
                   int selected)
        : Widget(std::move(name))
        , label_(std::move(label))
        , items_(std::move(items))
        , selected_(selected) {}
void ComboBox::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    const bool disabled = !IsEnabled();
    if (disabled)
        ImGui::BeginDisabled();
    const bool hasSel = selected_ >= 0 && selected_ < (int) items_.size();
    const char* preview = hasSel ? items_[selected_].c_str() : placeholder_.c_str();
    if (searchable_) {
        ImGui::InputText("##search", search_buf_, sizeof(search_buf_));
        ImGui::SameLine();
    }
    if (fillWidth_)
        ImGui::SetNextItemWidth(-FLT_MIN);
    else if (fixedWidth_ > 0.0f)
        ImGui::SetNextItemWidth(fixedWidth_);
    else
        ImGui::SetNextItemWidth(CalcComboWidth(items_, preview));
    // NoArrowButton：去掉右侧那个突兀的下拉三角，整个预览框仍可点击展开；
    // 统一的 UniGUI 下拉指示 = 右侧内边距里的细 ˅（见 detail/combo_chevron.h）。
    const auto comboFrame = detail::CaptureComboFrame();
    const bool comboOpen = ImGui::BeginCombo(label_.c_str(), preview, ImGuiComboFlags_NoArrowButton);
    const bool comboFocused = ImGui::IsItemFocused(); // capture before dropdown items steal it
    // [2026-08-25] Mouse-wheel quick-select — same behavior as im::Combo.
    // Use IsMouseHoveringRect (direct geometric check) instead of IsItemHovered()
    // to work inside ScrollY tables, child windows, and any hover-flag-blocked context.
    const bool comboHovered =
            ImGui::IsMouseHoveringRect(comboFrame.pos,
                                       ImVec2(comboFrame.pos.x + comboFrame.width,
                                              comboFrame.pos.y + comboFrame.height));
    detail::DrawComboChevron(comboFrame, comboOpen || comboHovered);
    // Wheel on closed combo = cycle selection; SetItemKeyOwner stops outer scroll region.
    if (!comboOpen && comboHovered && items_.size() > 1) {
        ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            int dir = wheel > 0.0f ? -1 : 1;
            int next = selected_ + dir;
            // Skip past the placeholder row when allowEmpty_
            const int lo = allowEmpty_ ? -1 : 0;
            if (next < lo) next = lo;
            if (next >= (int) items_.size()) next = (int) items_.size() - 1;
            if (next != selected_) {
                selected_ = next;
                if (on_change_) on_change_(selected_);
            }
        }
    }
    if (comboOpen) {
        if (allowEmpty_) {
            const char* none = placeholder_.empty() ? "(none)" : placeholder_.c_str();
            if (ImGui::Selectable(none, !hasSel)) {
                selected_ = -1;
                if (on_change_)
                    on_change_(-1);
            }
        }
        for (int i = 0; i < (int) items_.size(); i++) {
            if (searchable_ && search_buf_[0]) {
                if (items_[i].find(search_buf_) == std::string::npos)
                    continue;
            }
            bool is_sel = (i == selected_);
            // Render icon if set
            if (i < (int) icons_.size() && icons_[i]) {
                ImGui::Image(icons_[i], ImVec2(16, 16));
                ImGui::SameLine();
            }
            if (ImGui::Selectable(items_[i].c_str(), is_sel)) {
                selected_ = i;
                if (on_change_)
                    on_change_(i);
            }
            if (is_sel)
                ImGui::SetItemDefaultFocus();
        }
        if (editable_) {
            ImGui::Separator();
            static char newItem[256] = {};
            ImGui::InputText("##new", newItem, sizeof(newItem));
            if (ImGui::Button("Add") && newItem[0]) {
                items_.push_back(newItem);
                selected_ = (int) items_.size() - 1;
                if (on_change_)
                    on_change_(selected_);
                newItem[0] = 0;
            }
        }
        ImGui::EndCombo();
    }
    if (disabled)
        ImGui::EndDisabled();
    // Re-check bounds: selected_ may have changed to -1 ("(none)") inside the open dropdown
    // this frame, so the captured hasSel is stale — using it would read items_[-1].
    ReportAccessible(a11y::Role::Combo, comboFocused,
                     (selected_ >= 0 && selected_ < (int) items_.size()) ? items_[selected_]
                                                                         : std::string());
    ImGui::PopID();
}
int ComboBox::GetSelectedIndex() const {
    return selected_;
}
void ComboBox::SetSelectedIndex(int idx) {
    selected_ = idx;
}
const std::string& ComboBox::GetSelectedValue() const {
    static std::string empty;
    return (selected_ >= 0 && selected_ < (int) items_.size()) ? items_[selected_] : empty;
}
const std::vector<std::string>& ComboBox::GetItems() const {
    return items_;
}
void ComboBox::SetItems(std::vector<std::string> items) {
    items_ = std::move(items);
}
void ComboBox::SetOnChange(std::function<void(int)> callback) {
    on_change_ = std::move(callback);
}
void ComboBox::SetEditable(bool on) {
    editable_ = on;
}
void ComboBox::SetSearchable(bool on) {
    searchable_ = on;
}
void ComboBox::SetItemIcon(int index, ImTextureID textureID) {
    if (index >= (int) icons_.size())
        icons_.resize(index + 1, (ImTextureID) 0);
    icons_[index] = textureID;
}
ImTextureID ComboBox::GetItemIcon(int index) const {
    return index < (int) icons_.size() ? icons_[index] : (ImTextureID) 0;
}
} // namespace unigui
