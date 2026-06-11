#pragma once
#include <imgui.h>

#include <string>

namespace unigui {

/// Clipboard utilities wrapping ImGui clipboard.
namespace Clipboard {
inline void Copy(const std::string& text) {
    ImGui::SetClipboardText(text.c_str());
}
inline std::string Paste() {
    const char* s = ImGui::GetClipboardText();
    return s ? std::string(s) : std::string();
}
} // namespace Clipboard

} // namespace unigui
