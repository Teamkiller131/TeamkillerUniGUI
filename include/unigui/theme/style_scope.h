#pragma once

#include <imgui.h>

namespace unigui {

/// RAII scope for temporarily modifying ImGui style colors and variables.
/// Automatically pops all pushed changes when destroyed.
class StyleScope {
public:
    StyleScope() = default;
    ~StyleScope();

    // Move-only
    StyleScope(StyleScope&& other) noexcept;
    StyleScope& operator=(StyleScope&& other) noexcept;
    StyleScope(const StyleScope&) = delete;
    StyleScope& operator=(const StyleScope&) = delete;

    void PushColor(ImGuiCol idx, const ImVec4& color);
    void PushVar(ImGuiStyleVar idx, float val);
    void PushVar(ImGuiStyleVar idx, const ImVec2& val);

    /// Returns true if this scope is active (not moved-from).
    bool active() const { return push_count_ > 0 || !moved_from_; }

private:
    int color_push_count_ = 0;
    int var_push_count_ = 0;
    int push_count_ = 0;
    bool moved_from_ = false;
};

} // namespace unigui
