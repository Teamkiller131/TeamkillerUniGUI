#include <unigui/widgets/window.h>
#include <unigui/core/log.h>
#include <imgui.h>
#include <algorithm>
#include <vector>
#include <string>

namespace unigui {

Window::Window(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {
}

void Window::Render() {
    if (!IsVisible()) return;
    UNIGUI_LOG_TRACE("Window::Render '{}': {} panels", title_, panels_.size());

    if (pos_x_ >= 0)
        ImGui::SetNextWindowPos(ImVec2(pos_x_, pos_y_), ImGuiCond_FirstUseEver);
    if (width_ > 0 || height_ > 0)
        ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_FirstUseEver);

    bool open = IsVisible();
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    if (menu_bar_enabled_) flags |= ImGuiWindowFlags_MenuBar;

    // Respect popup priority: don't steal input when menus are open
    if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (ImGui::Begin(title_.c_str(), &open, flags)) {
        // v1.6: file drag-drop support
        if (onDrop_ && ImGui::BeginDragDropTarget()) {
            if (auto* payload = ImGui::AcceptDragDropPayload("FILES")) {
                std::vector<std::string> files;
                // ImGui doesn't parse file paths from drag-drop natively
                // Windows: files come as null-separated list
                const char* data = (const char*)payload->Data;
                int size = payload->DataSize;
                std::string current;
                for (int i = 0; i < size; i++) {
                    if (data[i] == '\0') {
                        if (!current.empty()) { files.push_back(current); current.clear(); }
                    } else {
                        current += data[i];
                    }
                }
                if (!files.empty()) onDrop_(files);
            }
            ImGui::EndDragDropTarget();
        }
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
void Window::SetPosition(float x, float y) { pos_x_ = x; pos_y_ = y; }

} // namespace unigui
