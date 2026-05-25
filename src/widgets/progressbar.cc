#include <unigui/widgets/progressbar.h>
#include <imgui.h>
namespace unigui {
ProgressBar::ProgressBar(std::string name, float fraction)
    : Widget(std::move(name)), fraction_(fraction) {}
void ProgressBar::Render() {
    if (!IsVisible()) return;
    ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram];
    if (state_ == Warning) color = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
    else if (state_ == Error) color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(fraction_, ImVec2(-1, 0), overlay_.empty() ? nullptr : overlay_.c_str());
    ImGui::PopStyleColor();
}
void ProgressBar::SetFraction(float f) { fraction_ = f; }
float ProgressBar::GetFraction() const { return fraction_; }
void ProgressBar::SetState(State s) { state_ = s; }
void ProgressBar::SetOverlayText(std::string text) { overlay_ = std::move(text); }
}
