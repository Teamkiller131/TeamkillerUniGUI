#include <unigui/widgets/checkbox.h>
#include <imgui.h>
namespace unigui {
CheckBox::CheckBox(std::string name, std::string label, bool checked)
    : ValueWidget<bool>(std::move(name), checked), label_(std::move(label)) {}
void CheckBox::Render() {
    if (!IsVisible()) return;
    bool prev = value_;
    ImGui::PushID(GetName().c_str());
    ImGui::Checkbox(label_.c_str(), &value_);
    ImGui::PopID();
    NotifyChange(prev);
}
const std::string& CheckBox::GetLabel() const { return label_; }
}
