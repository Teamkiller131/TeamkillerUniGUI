#pragma once

#include <string>
#include <string_view>
#include <imgui.h>

namespace unigui {

/// Theme presets.
enum class ThemePreset {
    Dark,   ///< Discord/Linear-inspired dark theme
    Light,  ///< Light theme (white/gray backgrounds, dark text)
};

/// Configuration for applying a theme.
struct ThemeConfig {
    ThemePreset preset = ThemePreset::Dark;
    float dpi_scale = 1.0f;
    float font_size = 16.0f;
};

/// Applies a complete theme (all 53 ImGui colors + style variables).
/// Must be called after ImGui context is created.
void ApplyTheme(const ThemeConfig& config);

/// Export the current ImGui theme colors to a JSON string.
std::string ExportThemeJSON();
/// Import theme colors from a JSON string. Returns true on success.
bool ImportThemeJSON(const std::string& json);

} // namespace unigui
