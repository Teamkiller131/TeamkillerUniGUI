#pragma once
#include <imgui.h>
#include <unigui/theme/style_tokens.h>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// Surface style presets (UI beautification — Step 2)
//
// Step 1 unified the *geometry* tokens (rounding/spacing/borders). Step 2 layers
// a *surface material* on top of any colour palette: how translucent the window,
// child, popup and frame surfaces read, plus the subtle bright "rim" border that
// sells a frosted-glass look.
//
// The frosted-glass / glassmorphism aesthetic (毛玻璃 — the general look, of which
// both the `Glass` and `Frosted` enum values are variants) is the project default
// (`SurfaceStyle::Glass`), but every other material keeps a ready-made preset so a
// theme can opt into a flat, heavier-frost, acrylic, or minimal look without
// hand-tuning alphas. The preset is applied *after* a palette is set, multiplying
// the surface alphas and tweaking borders, so it composes with the Dark/Light
// themes and any registry preset.
//
// NOTE: a translucent surface reveals whatever is painted behind the ImGui window
// (the framebuffer clear colour or other windows). To get a true glass effect the
// backend should clear to a tinted/blurred backdrop; with an opaque clear colour
// the material still reads as a soft, layered surface.
// ─────────────────────────────────────────────────────────────────────────────

namespace unigui::theme {

/// Surface material applied on top of a colour palette.
enum class SurfaceStyle {
    Solid,    ///< Flat, fully opaque surfaces — classic look, no translucency.
    Glass,    ///< Frosted glass / glassmorphism — translucent surfaces + bright rim. (default)
    Frosted,  ///< Heavier translucency than Glass, stronger rim.
    Acrylic,  ///< Fluent-style acrylic — translucent with a firmer tint and border.
    Minimal,  ///< Near-opaque, borderless and quiet — minimal chrome.
};

/// Per-material tuning. Alpha fields are *multipliers* applied to the matching
/// colour's existing alpha (palette surfaces are usually fully opaque, so a value
/// of 0.80 yields 80% opacity). Border-size fields use a negative sentinel to mean
/// "leave the geometry token untouched".
struct SurfaceTokens {
    float window_alpha   = 1.0f; ///< multiplier for WindowBg / DockingEmptyBg
    float child_alpha    = 1.0f; ///< multiplier for ChildBg
    float popup_alpha    = 1.0f; ///< multiplier for PopupBg
    float frame_alpha    = 1.0f; ///< multiplier for FrameBg (inputs)
    float menubar_alpha  = 1.0f; ///< multiplier for MenuBarBg
    float title_alpha    = 1.0f; ///< multiplier for TitleBg / TitleBgActive
    float border_alpha   = 1.0f; ///< multiplier for Border alpha
    bool  rim_highlight  = false;///< brighten the border into a glass "rim"
    float rim_strength   = 0.0f; ///< 0..1 amount the rim is lightened toward white
    float window_border  = -1.0f;///< override WindowBorderSize (px); <0 = keep token
    float frame_border   = -1.0f;///< override FrameBorderSize (px);  <0 = keep token
    float popup_border   = -1.0f;///< override PopupBorderSize (px);  <0 = keep token
};

/// Look up the tuning for a given surface material.
inline SurfaceTokens SurfacePreset(SurfaceStyle style) {
    SurfaceTokens t;
    switch (style) {
        case SurfaceStyle::Solid:
            // Everything opaque; rely purely on the palette and geometry tokens.
            t.window_alpha = t.child_alpha = t.popup_alpha = 1.0f;
            t.frame_alpha  = t.menubar_alpha = t.title_alpha = 1.0f;
            t.border_alpha = 1.0f;
            t.rim_highlight = false;
            break;

        case SurfaceStyle::Glass:  // default
            t.window_alpha  = 0.86f;
            t.child_alpha   = 0.78f;
            t.popup_alpha   = 0.92f;
            t.frame_alpha   = 0.70f;
            t.menubar_alpha = 0.82f;
            t.title_alpha   = 0.88f;
            t.border_alpha  = 0.90f;
            t.rim_highlight = true;
            t.rim_strength  = 0.22f;
            t.window_border = 1.0f;
            t.frame_border  = 0.0f;
            t.popup_border  = 1.0f;
            break;

        case SurfaceStyle::Frosted:
            t.window_alpha  = 0.72f;
            t.child_alpha   = 0.64f;
            t.popup_alpha   = 0.85f;
            t.frame_alpha   = 0.58f;
            t.menubar_alpha = 0.70f;
            t.title_alpha   = 0.78f;
            t.border_alpha  = 0.85f;
            t.rim_highlight = true;
            t.rim_strength  = 0.32f;
            t.window_border = 1.0f;
            t.frame_border  = 0.0f;
            t.popup_border  = 1.0f;
            break;

        case SurfaceStyle::Acrylic:
            t.window_alpha  = 0.80f;
            t.child_alpha   = 0.74f;
            t.popup_alpha   = 0.90f;
            t.frame_alpha   = 0.66f;
            t.menubar_alpha = 0.78f;
            t.title_alpha   = 0.84f;
            t.border_alpha  = 1.0f;
            t.rim_highlight = true;
            t.rim_strength  = 0.14f;
            t.window_border = 1.0f;
            t.frame_border  = 0.0f;
            t.popup_border  = 1.0f;
            break;

        case SurfaceStyle::Minimal:
            t.window_alpha  = 0.97f;
            t.child_alpha   = 0.95f;
            t.popup_alpha   = 1.0f;
            t.frame_alpha   = 0.92f;
            t.menubar_alpha = 0.96f;
            t.title_alpha   = 1.0f;
            t.border_alpha  = 0.0f;   // borderless
            t.rim_highlight = false;
            t.window_border = 0.0f;
            t.frame_border  = 0.0f;
            t.popup_border  = 0.0f;
            break;
    }
    return t;
}

/// Stable, human-readable name for a surface material (handy for UI pickers/logs).
inline const char* SurfaceStyleName(SurfaceStyle style) {
    switch (style) {
        case SurfaceStyle::Solid:   return "Solid";
        case SurfaceStyle::Glass:   return "Glass";
        case SurfaceStyle::Frosted: return "Frosted";
        case SurfaceStyle::Acrylic: return "Acrylic";
        case SurfaceStyle::Minimal: return "Minimal";
    }
    return "Glass";
}

/// All built-in surface materials, in display order.
inline const std::array<SurfaceStyle, 5>& AllSurfaceStyles() {
    static const std::array<SurfaceStyle, 5> kAll = {
        SurfaceStyle::Solid, SurfaceStyle::Glass, SurfaceStyle::Frosted,
        SurfaceStyle::Acrylic, SurfaceStyle::Minimal};
    return kAll;
}

namespace detail {
inline void MulAlpha(ImVec4& c, float m) { c.w = Clamp01(c.w * m); }
/// Last-applied surface material (single instance across TUs). Defaults to the
/// project default so queries are valid before the first ApplySurfaceStyle().
inline SurfaceStyle g_active_surface = SurfaceStyle::Glass;
} // namespace detail

/// Apply a surface material's tuning on top of an already-populated palette.
/// Multiplies the relevant surface alphas, tweaks the border into a bright rim
/// when requested, and optionally overrides border-size geometry. Call this
/// AFTER setting colours and BEFORE ImGuiStyle::ScaleAllSizes so border-size
/// overrides get DPI-scaled along with everything else.
inline void ApplySurfaceStyle(ImGuiStyle& s, const SurfaceTokens& t) {
    auto& c = s.Colors;
    detail::MulAlpha(c[ImGuiCol_WindowBg],       t.window_alpha);
    detail::MulAlpha(c[ImGuiCol_DockingEmptyBg], t.window_alpha);
    detail::MulAlpha(c[ImGuiCol_ChildBg],        t.child_alpha);
    detail::MulAlpha(c[ImGuiCol_PopupBg],        t.popup_alpha);
    detail::MulAlpha(c[ImGuiCol_FrameBg],        t.frame_alpha);
    detail::MulAlpha(c[ImGuiCol_MenuBarBg],      t.menubar_alpha);
    detail::MulAlpha(c[ImGuiCol_TitleBg],        t.title_alpha);
    detail::MulAlpha(c[ImGuiCol_TitleBgActive],  t.title_alpha);

    // Border: optionally lighten into a glass "rim", then scale its alpha.
    ImVec4 border = c[ImGuiCol_Border];
    if (t.rim_highlight && t.rim_strength > 0.0f) {
        border = Lighten(border, t.rim_strength);
    }
    detail::MulAlpha(border, t.border_alpha);
    c[ImGuiCol_Border] = border;

    if (t.window_border >= 0.0f) s.WindowBorderSize = t.window_border;
    if (t.frame_border  >= 0.0f) s.FrameBorderSize  = t.frame_border;
    if (t.popup_border  >= 0.0f) s.PopupBorderSize  = t.popup_border;
}

/// Convenience overload: apply a named material's preset.
inline void ApplySurfaceStyle(ImGuiStyle& s, SurfaceStyle style) {
    ApplySurfaceStyle(s, SurfacePreset(style));
    detail::g_active_surface = style;
}

/// The surface material from the most recent `ApplySurfaceStyle(style)` call
/// (defaults to the project default, Glass). Lets elevation/effects (Step 4) and
/// theme pickers know which material is active without threading it everywhere.
inline SurfaceStyle ActiveSurfaceStyle() { return detail::g_active_surface; }

/// Derive an opaque "backdrop" colour for the framebuffer clear behind ImGui
/// windows. Translucent (glass/frosted/acrylic) surfaces reveal whatever is
/// painted behind them, so the backend should clear to a tinted backdrop rather
/// than black for the material to read correctly. The backdrop is a slightly
/// darkened, fully-opaque version of the window background so translucent windows
/// stay visually distinct from the empty area behind them. For opaque materials
/// (Solid/Minimal) it simply mirrors the window background.
/// @param windowBg  the palette's WindowBg colour (alpha ignored).
/// @param style     the active surface material.
inline ImVec4 BackdropColor(const ImVec4& windowBg, SurfaceStyle style) {
    // Translucent materials benefit from a darker backdrop for contrast; opaque
    // ones keep the same tone so the central docking area matches windows.
    float darken = 0.0f;
    switch (style) {
        case SurfaceStyle::Glass:   darken = 0.04f; break;
        case SurfaceStyle::Frosted: darken = 0.06f; break;
        case SurfaceStyle::Acrylic: darken = 0.05f; break;
        case SurfaceStyle::Solid:
        case SurfaceStyle::Minimal: darken = 0.0f;  break;
    }
    ImVec4 b = Darken(windowBg, darken);
    b.w = 1.0f; // backdrop is always opaque
    return b;
}

} // namespace unigui::theme
