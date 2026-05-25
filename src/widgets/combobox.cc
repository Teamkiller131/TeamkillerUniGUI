#include <unigui/widgets/combobox.h>
#include <imgui.h>
namespace unigui {
ComboBox::ComboBox(std::string name, std::string label, std::vector<std::string> items, int selected)
    : Widget(std::move(name)), label_(std::move(label)), items_(std::move(items)), selected_(selected) {}
void ComboBox::Render() {
    if (!IsVisible()) return;
    if (ImGui::BeginCombo(label_.c_str(), selected_ < (int)items_.size() ? items_[selected_].c_str() : "")) {
        for (int i = 0; i < (int)items_.size(); i++) {
            bool is_sel = (i == selected_);
            if (ImGui::Selectable(items_[i].c_str(), is_sel)) {
                selected_ = i;
                if (on_change_) on_change_(i);
            }
            if (is_sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}
int ComboBox::GetSelectedIndex() const { return selected_; }
void ComboBox::SetSelectedIndex(int idx) { selected_ = idx; }
const std::string& ComboBox::GetSelectedValue() const { static std::string empty; return selected_ < (int)items_.size() ? items_[selected_] : empty; }
const std::vector<std::string>& ComboBox::GetItems() const { return items_; }
void ComboBox::SetItems(std::vector<std::string> items) { items_ = std::move(items); }
void ComboBox::SetOnChange(std::function<void(int)> callback) { on_change_ = std::move(callback); }
}
