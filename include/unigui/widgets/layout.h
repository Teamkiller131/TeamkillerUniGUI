#pragma once
#include <imgui.h>
#include <vector>
#include <functional>
#include <string>

namespace unigui {

/// Simple declarative layout helpers.
namespace Layout {

/// Horizontal box: render children side by side.
inline void HBox(std::initializer_list<std::function<void()>> children) {
    for (auto& c : children) { c(); if (&c != children.end() - 1) ImGui::SameLine(); }
}

/// Vertical box: render children stacked.
inline void VBox(std::initializer_list<std::function<void()>> children) {
    for (auto& c : children) { c(); }
}

/// Begin a horizontal group. Call EndHBox() after rendering children.
inline void BeginHBox() { ImGui::BeginGroup(); }
inline void EndHBox() { ImGui::EndGroup(); }

/// Begin a vertical split with ratio. ratio=0.5 means 50/50.
inline void BeginHSplit(float leftRatio = 0.5f) {
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::BeginChild("##left", ImVec2(avail * leftRatio, 0), ImGuiChildFlags_Borders);
}
inline void NextHSplit() {
    ImGui::EndChild(); ImGui::SameLine();
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::BeginChild("##right", ImVec2(avail, 0), ImGuiChildFlags_Borders);
}
inline void EndHSplit() { ImGui::EndChild(); }

} // namespace Layout
} // namespace unigui
