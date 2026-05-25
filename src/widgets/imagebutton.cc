#include <unigui/widgets/imagebutton.h>
#include <imgui.h>

namespace unigui {

ImageButton::ImageButton(std::string name, std::string label)
    : Widget(std::move(name)), label_(std::move(label)) {}

void ImageButton::Render() {
    if (!IsVisible()) return;
    if (!enabled_) ImGui::BeginDisabled();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding_);
    clicked_ = ImGui::ImageButton(GetName().c_str(), texture_,
        ImVec2(imgW_, imgH_), ImVec2(0, 0), ImVec2(1, 1),
        ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
    ImGui::PopStyleVar();
    if (!label_.empty()) {
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label_.c_str());
    }
    if (!enabled_) ImGui::EndDisabled();
}

void ImageButton::SetImage(ImTextureID textureID, float width, float height) {
    texture_ = textureID;
    imgW_ = width;
    imgH_ = height;
}

void ImageButton::SetLabel(std::string label) { label_ = std::move(label); }
bool ImageButton::WasClicked() const { return clicked_; }
void ImageButton::SetEnabled(bool enabled) { enabled_ = enabled; }
void ImageButton::SetFramePadding(float x, float y) { framePadding_ = ImVec2(x, y); }

} // namespace unigui
