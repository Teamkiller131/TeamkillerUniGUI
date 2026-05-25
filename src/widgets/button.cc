#include <unigui/widgets/button.h>
#include <imgui.h>

namespace unigui {

Button::Button(std::string name, std::string label)
    : Widget(std::move(name)), label_(std::move(label)) {
}

void Button::Render() {
    if (!IsVisible()) return;
    ImGui::BeginDisabled(!enabled_);
    clicked_ = ImGui::Button(label_.c_str());
    ImGui::EndDisabled();
}

bool Button::WasClicked() const { return clicked_; }
void Button::SetEnabled(bool enabled) { enabled_ = enabled; }
bool Button::IsEnabled() const { return enabled_; }
const std::string& Button::GetLabel() const { return label_; }
void Button::SetLabel(std::string label) { label_ = std::move(label); }

} // namespace unigui
