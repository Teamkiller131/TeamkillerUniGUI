#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace unigui::theme {

/// Theme preset descriptor
struct ThemePreset {
    std::string name;
    std::string description;
    std::function<void(ImGuiStyle&)> apply;
};

/// Central registry for all theme presets
class ThemeRegistry {
public:
    static ThemeRegistry& Instance();

    void Register(ThemePreset preset);
    const ThemePreset* Get(const std::string& name) const;
    std::vector<std::string> List() const;
    bool Apply(const std::string& name);

    /// Set callback fired when theme changes.  Receives the new theme name.
    void SetOnChange(std::function<void(const std::string&)> cb) { onChange_ = std::move(cb); }

private:
    ThemeRegistry() = default;
    std::unordered_map<std::string, ThemePreset> presets_;
    std::string current_;
    std::function<void(const std::string&)> onChange_;
};

// ── Convenience: register all built-in themes ────────────────────────────────
void RegisterAllThemes();

} // namespace unigui::theme
