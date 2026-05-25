#include <unigui/widgets/checkbox.h>
#include <imgui.h>
namespace unigui {
CheckBox::CheckBox(std::string name, std::string label, bool checked)
    : Widget(std::move(name)), label_(std::move(label)), checked_(checked) {}
void CheckBox::Render() {
    if (!IsVisible()) return;
    bool prev = checked_;
    ImGui::Checkbox(label_.c_str(), &checked_);
    if (checked_ != prev && on_change_) on_change_(checked_);
}
bool CheckBox::IsChecked() const { return checked_; }
void CheckBox::SetChecked(bool checked) { checked_ = checked; }
const std::string& CheckBox::GetLabel() const { return label_; }
void CheckBox::SetOnChange(std::function<void(bool)> callback) { on_change_ = std::move(callback); }
}
