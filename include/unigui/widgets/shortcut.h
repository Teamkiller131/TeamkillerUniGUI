#pragma once
#include <imgui.h>

#include <functional>
#include <string>
#include <vector>
namespace unigui {
struct ShortcutBinding {
    ImGuiKey key;
    bool ctrl = false;
    std::function<void()> action;
    std::string description;
};
class ShortcutManager {
public:
    void Register(ImGuiKey key, bool ctrl, std::function<void()> action, std::string desc = "") {
        bindings_.push_back({key, ctrl, std::move(action), std::move(desc)});
    }
    void Process() {
        for (auto& b : bindings_) {
            bool pressed = ImGui::IsKeyPressed(b.key);
            bool modOk = b.ctrl ? ImGui::GetIO().KeyCtrl : true;
            if (pressed && modOk && b.action)
                b.action();
        }
    }

private:
    std::vector<ShortcutBinding> bindings_;
};
} // namespace unigui
