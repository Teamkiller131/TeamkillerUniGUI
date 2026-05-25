#include <unigui/widgets/label.h>
#include <imgui.h>

namespace unigui {

Label::Label(std::string name, std::string text)
    : Widget(std::move(name)), text_(std::move(text)) {
}

void Label::Render() {
    if (!IsVisible()) return;
    ImGui::TextUnformatted(text_.c_str());
}

void Label::SetText(std::string text) { text_ = std::move(text); }
const std::string& Label::GetText() const { return text_; }

} // namespace unigui
