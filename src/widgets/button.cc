#include <unigui/widgets/button.h>
#include <imgui.h>

namespace unigui {

Button::Button(std::string name, std::string label)
    : Widget(std::move(name)), label_(std::move(label)) {
}

void Button::Render() {
    if (!IsVisible()) return;
    ImGui::BeginDisabled(!enabled_);
    if (variant_ != Default || sz_ != Medium) {
        ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Button];
        switch (variant_) {
        case Primary: color = ImVec4(0.15f,0.40f,0.80f,1.0f); break;
        case Danger:  color = ImVec4(0.80f,0.15f,0.15f,1.0f); break;
        case Success: color = ImVec4(0.15f,0.60f,0.25f,1.0f); break;
        default: break;
        }
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x*1.1f,color.y*1.1f,color.z*1.1f,1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x*0.9f,color.y*0.9f,color.z*0.9f,1));
    }
    ImVec2 size(0,0);
    if (sz_ == Small) size = ImVec2(80, 24);
    else if (sz_ == Large) size = ImVec2(180, 36);
    if (size.x > 0) clicked_ = ImGui::Button(label_.c_str(), size);
    else clicked_ = ImGui::Button(label_.c_str());
    if (variant_ != Default || sz_ != Medium) ImGui::PopStyleColor(3);
    ImGui::EndDisabled();
}

bool Button::WasClicked() const { return clicked_; }
void Button::SetEnabled(bool enabled) { enabled_ = enabled; }
bool Button::IsEnabled() const { return enabled_; }
const std::string& Button::GetLabel() const { return label_; }
void Button::SetLabel(std::string label) { label_ = std::move(label); }
void Button::SetColorVariant(ColorVariant v) { variant_ = v; }
void Button::SetSize(Size s) { sz_ = s; }

} // namespace unigui
