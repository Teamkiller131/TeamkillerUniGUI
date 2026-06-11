#pragma once
#include <imgui.h>

#include <string>
namespace unigui {
class Tooltip {
public:
    static void Show(std::string text) {
        if (!text.empty())
            ImGui::SetTooltip("%s", text.c_str());
    }
};
} // namespace unigui
