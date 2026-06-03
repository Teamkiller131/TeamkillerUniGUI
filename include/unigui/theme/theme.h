#pragma once

#include <string>
#include <string_view>
#include <imgui.h>
#include <unigui/theme/surface_style.h>
#include <unigui/theme/color_tokens.h>

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
    bool emoji_fallback = true;  // auto-load system emoji font
    // Surface material applied on top of the palette. Frosted glass / glassmorphism
    // (毛玻璃, the general aesthetic — not the specific `Frosted` enum value) is the
    // default; switch to Solid for fully opaque classic surfaces, or pick another
    // ready-made material (Frosted/Acrylic/Minimal). See theme/surface_style.h.
    // Kept last so positional aggregate initialisation of the older fields stays valid.
    theme::SurfaceStyle surface = theme::SurfaceStyle::Glass;
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
/// Deferred font atlas rebuild (call before ImGui::NewFrame, not mid-frame)
bool HasPendingFontRebuild();
void ApplyPendingFontRebuild();

/// Set global font scale. Wraps ImGui::GetIO().FontGlobalScale.
/// Call after Init(), before the render loop. Default is 1.0f.
inline void SetFontScale(float scale) { ImGui::GetIO().FontGlobalScale = scale; }
inline float GetFontScale() { return ImGui::GetIO().FontGlobalScale; }
/// Export/Import current ImGui theme colors to/from JSON.
std::string ExportThemeJSON();
bool ImportThemeJSON(const std::string& json);

/// Opaque framebuffer clear colour for the active theme + surface material.
/// Translucent surface materials (Glass/Frosted/Acrylic) reveal whatever is drawn
/// behind ImGui windows, so backends should clear to this tinted backdrop instead
/// of black for the glass effect to read correctly. Updated on every ApplyTheme();
/// defaults to the Dark window background before the first ApplyTheme() call.
ImVec4 GetBackdropColor();

/// Active accent & semantic colour tokens for the current theme (Step 3). Updated
/// on every ApplyTheme() and ThemeRegistry::Apply(). Widgets that need semantic
/// colours (success/warning/danger/info) should read them from here so they track
/// the active theme's accent. Thin wrappers over theme::ActiveColorTokens().
inline const theme::ColorTokens& GetColorTokens() { return theme::ActiveColorTokens(); }
inline ImVec4 GetSemanticColor(theme::Semantic role) { return theme::GetSemanticColor(role); }

} // namespace unigui
