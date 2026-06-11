#include <unigui/widgets/inputtext.h>

#include <imgui.h>

#include <algorithm>
#include <cstring>

namespace unigui {

InputText::InputText(std::string name, std::string label, std::string value,
                     ImGuiInputTextFlags flags)
        : ValueWidget<std::string>(std::move(name), std::move(value))
        , label_(std::move(label))
        , flags_(flags) {
    buffer_.resize(std::max<size_t>(4096, value_.size() + 1), '\0');
}

void InputText::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    if (buffer_.size() <= value_.size()) {
        buffer_.resize(value_.size() + 1, '\0');
    }
    std::fill(buffer_.begin(), buffer_.end(), '\0');
    std::memcpy(buffer_.data(), value_.data(), value_.size());

    std::string oldVal = value_;
    bool changed = false;
    const bool disabled = !IsEnabled();
    if (disabled)
        ImGui::BeginDisabled();
    if (multiline_) {
        changed = ImGui::InputTextMultiline(label_.c_str(), buffer_.data(), buffer_.size(),
                                            ImVec2(-1, 80), flags_);
    } else if (!hint_.empty()) {
        changed = ImGui::InputTextWithHint(label_.c_str(), hint_.c_str(), buffer_.data(),
                                           buffer_.size(), flags_);
    } else {
        changed = ImGui::InputText(label_.c_str(), buffer_.data(), buffer_.size(), flags_);
    }

    if (disabled)
        ImGui::EndDisabled();
    if (changed) {
        value_ = buffer_.data();
        NotifyChange(oldVal);
    }
    ImGui::PopID();
}

void InputText::SetHint(std::string hint) {
    hint_ = std::move(hint);
}
void InputText::SetPassword(bool on) {
    if (on)
        flags_ |= ImGuiInputTextFlags_Password;
    else
        flags_ &= ~ImGuiInputTextFlags_Password;
}
void InputText::SetMultiline(bool on) {
    multiline_ = on;
}
void InputText::SetReadOnly(bool on) {
    if (on)
        flags_ |= ImGuiInputTextFlags_ReadOnly;
    else
        flags_ &= ~ImGuiInputTextFlags_ReadOnly;
}

} // namespace unigui
