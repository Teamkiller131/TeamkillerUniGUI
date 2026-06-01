#include <unigui/widgets/inputtext.h>
#include <imgui.h>
#include <cstring>

namespace unigui {

InputText::InputText(std::string name, std::string label, std::string value, ImGuiInputTextFlags flags)
    : ValueWidget<std::string>(std::move(name), std::move(value)), label_(std::move(label)), flags_(flags) {
    buf_[0] = '\0';
}

void InputText::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    std::strncpy(buf_, value_.c_str(), sizeof(buf_) - 1);
    buf_[sizeof(buf_) - 1] = '\0';

    std::string oldVal = value_;
    bool changed = false;
    if (!hint_.empty())
        changed = ImGui::InputTextWithHint(label_.c_str(), hint_.c_str(), buf_, sizeof(buf_), flags_);
    else
        changed = ImGui::InputText(label_.c_str(), buf_, sizeof(buf_), flags_);

    if (changed) {
        value_ = buf_;
        NotifyChange(oldVal);
    }
    ImGui::PopID();
}

void InputText::SetHint(std::string hint) { hint_ = std::move(hint); }
void InputText::SetPassword(bool on) {
    if (on) flags_ |= ImGuiInputTextFlags_Password;
    else    flags_ &= ~ImGuiInputTextFlags_Password;
}
void InputText::SetMultiline(bool) { /* imgui 1.92: use separate InputTextMultiline widget */ }
void InputText::SetReadOnly(bool on) {
    if (on) flags_ |= ImGuiInputTextFlags_ReadOnly;
    else    flags_ &= ~ImGuiInputTextFlags_ReadOnly;
}

} // namespace unigui
