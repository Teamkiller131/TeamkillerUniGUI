#include <unigui/widgets/panel.h>
#include <imgui.h>

namespace unigui {

Panel::Panel(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {
}

void Panel::Render() {
    if (!IsVisible()) return;
    if (!ImGui::Begin(title_.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }
    if (content_callback_) {
        if (wrap_) ImGui::PushTextWrapPos(0.0f); // 0 = wrap at right edge
        content_callback_();
        if (wrap_) ImGui::PopTextWrapPos();
    }
    ImGui::End();
}

void Panel::SetTitle(std::string title) { title_ = std::move(title); }
const std::string& Panel::GetTitle() const { return title_; }
bool Panel::IsCollapsed() const { return collapsed_; }
void Panel::SetContentCallback(std::function<void()> callback) { content_callback_ = std::move(callback); }

} // namespace unigui
