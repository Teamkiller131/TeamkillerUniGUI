#pragma once

#include <string>
#include <string_view>
#include <imgui.h>

namespace unigui {

enum class ThemePreset {
    Dark,
    Light,
};

struct ThemeConfig {
    ThemePreset preset = ThemePreset::Dark;
    float dpi_scale = 0.0f;  // 0 = auto-detect from system DPI
    float font_size = 16.0f; // logical px at 96 DPI (scaled by auto-DPI)
    const char* font_path = nullptr; // nullptr = auto-detect system CJK font
};

/// Detect system DPI scale factor. Returns 1.0 on failure.
/// On Windows: uses GetDpiForWindow or GetDeviceCaps.
float DetectDPIScale(void* native_window);

/// Load the default font with CJK support at the given pixel size.
/// Calls ImGui::GetIO().Fonts->AddFontFromFileTTF with CJK glyph ranges.
/// Falls back to built-in font if TTF file not found.
void LoadDefaultFont(float size_pixels, const char* ttf_path = nullptr);

/// Enable text wrapping for content areas. Call before rendering panels.
/// Pairs: PushTextWrapPos (begin) / PopTextWrapPos (end).
void BeginTextWrap(float width = 0.0f);
void EndTextWrap();

/// Applies a complete theme (colors + style + DPI scaling + font).
/// Must be called after ImGui context is created.
void ApplyTheme(const ThemeConfig& config);

/// Export/Import current ImGui theme colors to/from JSON.
std::string ExportThemeJSON();
bool ImportThemeJSON(const std::string& json);

} // namespace unigui
