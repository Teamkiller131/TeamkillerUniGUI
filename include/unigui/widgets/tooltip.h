#pragma once
#include <string>
#include <imgui.h>
namespace unigui {
class Tooltip {
public:
    static void Show(std::string text) { if (!text.empty()) ImGui::SetTooltip("%s", text.c_str()); }
};
}
