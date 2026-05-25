#include <unigui/widgets/window.h>
#include <imgui.h>

namespace unigui {

Window::Window(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {
}

void Window::Render() {
    if (!IsVisible()) return;

    if (width_ > 0 || height_ > 0) {
        ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_FirstUseEver);
    }

    bool open = IsVisible();
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    if (menu_bar_enabled_) flags |= ImGuiWindowFlags_MenuBar;

    if (ImGui::Begin(title_.c_str(), &open, flags)) {
        for (auto& panel : panels_) {
            panel->Render();
        }
    }
    ImGui::End();

    if (!open) {
        Hide();
        if (on_close_) on_close_();
    }
}

void Window::AddPanel(std::shared_ptr<Panel> panel) {
    panels_.push_back(std::move(panel));
}

void Window::RemovePanel(const std::string& panel_name) {
    panels_.erase(std::remove_if(panels_.begin(), panels_.end(),
        [&](auto& p) { return p->GetName() == panel_name; }), panels_.end());
}

void Window::SetSize(float width, float height) { width_ = width; height_ = height; }
void Window::SetMenuBarEnabled(bool enabled) { menu_bar_enabled_ = enabled; }
bool Window::HasMenuBar() const { return menu_bar_enabled_; }
void Window::SetOnClose(std::function<void()> callback) { on_close_ = std::move(callback); }

} // namespace unigui
