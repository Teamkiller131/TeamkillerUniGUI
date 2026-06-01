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

// ── RAII horizontal layout ───────────────────────────────────────────────
/// Auto SameLine between children + restores spacing.
/// Usage: { HBox h; Button...; Button...; }
class HBox {
public:
    HBox(float spacing = -1.0f) : spacing_(spacing) {
        if (spacing_ >= 0)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing_, 0));
    }
    ~HBox() {
        if (spacing_ >= 0)
            ImGui::PopStyleVar();
    }
    static void VSeparator() { ImGui::SameLine(); ImGui::TextUnformatted("|"); }
private:
private:
    float spacing_ = -1.0f;
};

// ── RAII vertical layout ──────────────────────────────────────────────────
/// Auto-stacks children vertically with optional spacing override.
/// Usage: { VBox v; Button...; Button...; }
class VBox {
public:
    VBox(float spacing = -1.0f) : spacing_(spacing) {
        if (spacing_ >= 0)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, spacing_));
    }
    ~VBox() {
        if (spacing_ >= 0)
            ImGui::PopStyleVar();
    }
private:
    float spacing_ = -1.0f;
};

} // namespace unigui
