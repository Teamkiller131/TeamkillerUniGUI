#include <unigui/core/log.h>
#include <unigui/widgets/window.h>

#include <imgui.h>

#include <algorithm>
#include <string>
#include <vector>

namespace unigui {

Window::Window(std::string name, std::string title)
        : FluentWidget<Window>(std::move(name))
        , title_(std::move(title)) {}

void Window::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    UNIGUI_LOG_TRACE("Window::Render '{}': {} panels", title_, panels_.size());

    if (pos_x_ >= 0)
        ImGui::SetNextWindowPos(ImVec2(pos_x_, pos_y_), ImGuiCond_FirstUseEver);
    if (width_ > 0 || height_ > 0)
        ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_FirstUseEver);

    bool open = IsVisible();
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    if (menu_bar_enabled_)
        flags |= ImGuiWindowFlags_MenuBar;

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
                const char* data = (const char*) payload->Data;
                int size = payload->DataSize;
                std::string current;
                for (int i = 0; i < size; i++) {
                    if (data[i] == '\0') {
                        if (!current.empty()) {
                            files.push_back(current);
                            current.clear();
                        }
                    } else {
                        current += data[i];
                    }
                }
                if (!files.empty())
                    onDrop_(files);
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
        if (!closeToTray_ && on_close_)
            on_close_();
    }
    ImGui::PopID();
}

void Window::AddPanel(std::shared_ptr<Panel> panel) {
    panels_.push_back(std::move(panel));
}

void Window::RemovePanel(const std::string& panel_name) {
    panels_.erase(std::remove_if(panels_.begin(), panels_.end(),
                                 [&](auto& p) { return p->GetName() == panel_name; }),
                  panels_.end());
}

void Window::SetSize(float width, float height) {
    width_ = width;
    height_ = height;
}
void Window::SetMenuBarEnabled(bool enabled) {
    menu_bar_enabled_ = enabled;
}
bool Window::HasMenuBar() const {
    return menu_bar_enabled_;
}
void Window::SetOnClose(std::function<void()> callback) {
    on_close_ = std::move(callback);
}
void Window::SetPosition(float x, float y) {
    pos_x_ = x;
    pos_y_ = y;
}

std::string Window::SaveLayout() const {
    char buf[128];
    snprintf(buf, sizeof(buf), R"({"x":%.0f,"y":%.0f,"w":%.0f,"h":%.0f})", pos_x_, pos_y_, width_,
             height_);
    return buf;
}

void Window::RestoreLayout(const std::string& json) {
    float x = 0, y = 0, w = 0, h = 0;
    if (sscanf(json.c_str(), R"({"x":%f,"y":%f,"w":%f,"h":%f})", &x, &y, &w, &h) >= 4) {
        pos_x_ = x;
        pos_y_ = y;
        width_ = w;
        height_ = h;
    }
}

} // namespace unigui
