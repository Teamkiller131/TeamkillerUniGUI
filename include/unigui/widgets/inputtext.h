#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>
#include <vector>
#include <imgui.h>

namespace unigui {

/// InputText — wrapper for ImGui::InputText with hint, password, multiline, read-only support.
class InputText : public ValueWidget<std::string> {
public:
    InputText(std::string name, std::string label, std::string value = "", ImGuiInputTextFlags flags = 0);

    void Render() override;

    void SetHint(std::string hint);
    void SetPassword(bool on);
    void SetMultiline(bool on);
    void SetReadOnly(bool on);

private:
    std::string label_;
    std::string hint_;
    ImGuiInputTextFlags flags_ = 0;
    bool multiline_ = false;
    std::vector<char> buffer_;
};

} // namespace unigui
