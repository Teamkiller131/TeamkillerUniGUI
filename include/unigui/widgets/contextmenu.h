#pragma once
#include <imgui.h>

#include <functional>
#include <string>
#include <vector>
namespace unigui {
struct ContextMenuItem {
    std::string label;
    std::function<void()> action;
    bool separator = false;
};
class ContextMenu {
public:
    static void Show(const char* id, std::vector<ContextMenuItem> items) {
        if (ImGui::BeginPopupContextItem(id)) {
            for (auto& item : items) {
                if (item.separator)
                    ImGui::Separator();
                else if (ImGui::MenuItem(item.label.c_str()) && item.action)
                    item.action();
            }
            ImGui::EndPopup();
        }
    }
    static void ShowWindow(const char* id, std::vector<ContextMenuItem> items) {
        if (ImGui::BeginPopupContextWindow(id, ImGuiPopupFlags_MouseButtonRight |
                                                   ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            for (auto& item : items) {
                if (item.separator)
                    ImGui::Separator();
                else if (ImGui::MenuItem(item.label.c_str()) && item.action)
                    item.action();
            }
            ImGui::EndPopup();
        }
    }
};
} // namespace unigui
