#include <unigui/widgets/combobox.h>
#include <imgui.h>
#include <algorithm>

namespace unigui {

namespace {

float CalcComboWidth(const std::vector<std::string>& items, const char* preview) {
    float maxTextWidth = ImGui::CalcTextSize(preview ? preview : "").x;
    for (const auto& item : items)
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(item.c_str()).x);
    const ImGuiStyle& style = ImGui::GetStyle();
    return maxTextWidth + style.FramePadding.x * 2.0f + ImGui::GetFrameHeight() + style.ItemInnerSpacing.x + 4.0f;
}

} // namespace

ComboBox::ComboBox(std::string name, std::string label, std::vector<std::string> items, int selected)
    : Widget(std::move(name)), label_(std::move(label)), items_(std::move(items)), selected_(selected) {}
void ComboBox::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    const char* preview = selected_ < (int)items_.size() ? items_[selected_].c_str() : "";
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
    if (ImGui::BeginCombo(label_.c_str(), preview)) {
        for (int i = 0; i < (int)items_.size(); i++) {
            if (searchable_ && search_buf_[0]) {
                if (items_[i].find(search_buf_) == std::string::npos) continue;
            }
            bool is_sel = (i == selected_);
            // Render icon if set
            if (i < (int)icons_.size() && icons_[i]) {
                ImGui::Image(icons_[i], ImVec2(16, 16));
                ImGui::SameLine();
            }
            if (ImGui::Selectable(items_[i].c_str(), is_sel)) {
                selected_ = i; if (on_change_) on_change_(i);
            }
            if (is_sel) ImGui::SetItemDefaultFocus();
        }
        if (editable_) {
            ImGui::Separator();
            static char newItem[256] = {};
            ImGui::InputText("##new", newItem, sizeof(newItem));
            if (ImGui::Button("Add") && newItem[0]) {
                items_.push_back(newItem); selected_ = (int)items_.size() - 1;
                if (on_change_) on_change_(selected_); newItem[0] = 0;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
}
int ComboBox::GetSelectedIndex() const { return selected_; }
void ComboBox::SetSelectedIndex(int idx) { selected_ = idx; }
const std::string& ComboBox::GetSelectedValue() const { static std::string empty; return selected_ < (int)items_.size() ? items_[selected_] : empty; }
const std::vector<std::string>& ComboBox::GetItems() const { return items_; }
void ComboBox::SetItems(std::vector<std::string> items) { items_ = std::move(items); }
void ComboBox::SetOnChange(std::function<void(int)> callback) { on_change_ = std::move(callback); }
void ComboBox::SetEditable(bool on) { editable_ = on; }
void ComboBox::SetSearchable(bool on) { searchable_ = on; }
void ComboBox::SetItemIcon(int index, ImTextureID textureID) {
    if (index >= (int)icons_.size()) icons_.resize(index + 1, (ImTextureID)0);
    icons_[index] = textureID;
}
ImTextureID ComboBox::GetItemIcon(int index) const {
    return index < (int)icons_.size() ? icons_[index] : (ImTextureID)0;
}
}
