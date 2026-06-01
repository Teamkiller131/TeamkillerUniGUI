#include <unigui/widgets/statusbar.h>
#include <imgui.h>
namespace unigui {
StatusBar::StatusBar(std::string name, std::string text) : Widget(std::move(name)), text_(std::move(text)) {}
void StatusBar::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    ImGui::Separator();
    ImGui::TextUnformatted(text_.c_str());
    ImGui::PopID();
}
void StatusBar::SetText(std::string text) { text_ = std::move(text); }
const std::string& StatusBar::GetText() const { return text_; }
}
