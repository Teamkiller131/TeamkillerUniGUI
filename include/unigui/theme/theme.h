#pragma once

#include <unigui/theme/color_tokens.h>
#include <unigui/theme/surface_style.h>

#include <imgui.h>

#include <string>
#include <string_view>

namespace unigui {

enum class ThemePreset {
    Dark,
    Light,
};

struct ThemeConfig {
    ThemePreset preset = ThemePreset::Light;
    float dpi_scale = 0.0f;          // 0 = auto-detect from system DPI
    float font_size = 13.0f;         // logical px at 96 DPI (FontScaleDpi re-rasterizes per monitor)
    const char* font_path = nullptr; // CJK glyph-fallback override; nullptr = auto
    bool emoji_fallback = true;      // auto-load system emoji font
    // Surface material applied on top of the palette. Frosted glass / glassmorphism
    // (毛玻璃, the general aesthetic — not the specific `Frosted` enum value) is the
    // default; switch to Solid for fully opaque classic surfaces, or pick another
    // ready-made material (Frosted/Acrylic/Minimal). See theme/surface_style.h.
    // Kept before the font-family fields so positional aggregate initialisation of
    // the older fields stays valid.
    theme::SurfaceStyle surface = theme::SurfaceStyle::Glass;
    // Font-family configuration (S0/D4–D5). nullptr/"" = role default:
    //   body    → embedded JetBrains Mono Nerd Font (always available)
    //   heading → body
    //   mono    → body (the embedded body font is itself monospace)
    // `font_path` above stays the CJK *fallback* override; these three select the
    // Latin families. Appended after `surface` for positional-init compatibility.
    const char* body_font_path = nullptr;
    const char* heading_font_path = nullptr;
    const char* mono_font_path = nullptr;
};

/// Font families the UI shell renders with. Empty strings mean "role default"
/// (body = the embedded JetBrains Mono Nerd Font; heading/mono fall back to the
/// body). `cjk` overrides the CJK glyph-fallback chain (empty = automatic:
/// UNIGUI_CJK_FONT → search paths → embedded OFL subset → system font).
struct FontChainConfig {
    std::string body;
    std::string heading;
    std::string mono;
    std::string cjk;
    bool emoji = true;
};

/// Detect system DPI scale factor. Returns 1.0 on failure.
/// On Windows: uses GetDpiForWindow or GetDeviceCaps.
float DetectDPIScale(void* native_window);

/// Load the default font chain at the given pixel size and make it active.
///
/// Chain: body font (embedded JetBrains Mono Nerd Font, or the configured
/// `body_font_path`) + CJK glyph fallback (explicit path → search paths →
/// embedded OFL subset → system font) + optional emoji fallback. Every probe
/// failure is logged explicitly and surfaced through GetFontFallbackReport() —
/// a missing CJK font is never silent (S0/D37).
///
/// `ttf_path` is the legacy explicit CJK fallback override (app.cc passes
/// ThemeConfig::font_path); nullptr keeps the chain configured by ApplyTheme().
/// Clears the atlas first: all previously obtained ImFont* are invalidated.
void LoadDefaultFont(float size_pixels, const char* ttf_path = nullptr);

/// Configure the font families for the next LoadDefaultFont()/DPI rebuild.
/// ApplyTheme() fills this from ThemeConfig on every call, so a DPI- or
/// font-size-driven rebuild keeps the configured chain.
void ConfigureFontChain(const FontChainConfig& chain);
const FontChainConfig& GetFontChain();

/// Primary font (body). nullptr before LoadDefaultFont().
ImFont* GetBodyFont();
/// Dedicated heading family when configured, otherwise the body font.
ImFont* GetHeadingFont();
/// Dedicated mono family when configured, otherwise the body font.
ImFont* GetMonoFont();
/// One-line health report of the active font chain (embedded assets, CJK
/// fallback source, explicit failure reasons). Never empty after
/// LoadDefaultFont(); the no-font-environment acceptance test reads this.
std::string GetFontFallbackReport();

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
inline void SetFontScale(float scale) {
    ImGui::GetIO().FontGlobalScale = scale;
}
inline float GetFontScale() {
    return ImGui::GetIO().FontGlobalScale;
}
/// Export/Import current ImGui theme colors to/from JSON.
std::string ExportThemeJSON();
bool ImportThemeJSON(const std::string& json);

/// Opaque framebuffer clear colour for the active theme + surface material.
/// Translucent surface materials (Glass/Frosted/Acrylic) reveal whatever is drawn
/// behind ImGui windows, so backends should clear to this tinted backdrop instead
/// of black for the glass effect to read correctly. Updated on every ApplyTheme();
/// defaults to the Light window background before the first ApplyTheme() call.
ImVec4 GetBackdropColor();

/// Active accent & semantic colour tokens for the current theme (Step 3). Updated
/// on every ApplyTheme() and ThemeRegistry::Apply(). Widgets that need semantic
/// colours (success/warning/danger/info) should read them from here so they track
/// the active theme's accent. Thin wrappers over theme::ActiveColorTokens().
inline const theme::ColorTokens& GetColorTokens() {
    return theme::ActiveColorTokens();
}
inline ImVec4 GetSemanticColor(theme::Semantic role) {
    return theme::GetSemanticColor(role);
}

} // namespace unigui
