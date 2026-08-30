#pragma once
#include <unigui/fx/effect_scope.h>
#include <unigui/theme/surface_style.h>

#include <imgui.h>

#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Theme-driven elevation (UI beautification — Step 4)
//
// Step 4 ties the low-level visual effects (ShadowEffect/GlowEffect from
// effect_scope.h) to a small, semantic "elevation" scale and to the active
// surface material (Step 2). Instead of hand-picking a shadow radius/offset/alpha
// per widget, callers pick an `Elevation` level and the tokens for the current
// material are derived for them:
//
//   • Solid   → firmer, darker drop shadows, no rim glow.
//   • Glass / Frosted / Acrylic → softer, lower-alpha shadows plus a subtle bright
//     rim glow, so translucent surfaces read with depth consistent with the glass.
//   • Minimal → very quiet shadow, no glow.
//
// Higher elevation = larger radius, bigger downward offset, stronger shadow. The
// `MakeElevation*` factories return ready-to-use effect objects; `WithElevation`
// on widgets (widget_base.h) fills a widget's ShadowConfig from these tokens.
// ─────────────────────────────────────────────────────────────────────────────

namespace unigui::fx {

/// Semantic depth scale. Higher = the surface floats further above the page.
enum class Elevation {
    None,   ///< Flush with the page — no shadow.
    Low,    ///< Cards, list rows.
    Medium, ///< Panels, popovers.
    High,   ///< Dialogs, menus, floating windows.
};

/// Tuning derived from an elevation level + surface material.
struct ElevationTokens {
    float shadow_radius = 0.0f;   ///< blur spread (px)
    float shadow_offset_x = 0.0f; ///< horizontal offset (px)
    float shadow_offset_y = 0.0f; ///< vertical offset (px, positive = down)
    int shadow_alpha = 0;         ///< shadow opacity (0..255)
    int shadow_samples = 3;       ///< blur passes (1..4)
    bool rim_glow = false;        ///< add a bright rim glow (glass materials)
    float glow_radius = 0.0f;     ///< rim glow spread (px)
    int glow_alpha = 0;           ///< rim glow opacity (0..255)
};

namespace detail {

/// Material-neutral base tuning per elevation level.
inline ElevationTokens ElevationBase(Elevation level) {
    ElevationTokens t;
    switch (level) {
    case Elevation::None:
        t.shadow_radius = 0.f;
        t.shadow_offset_y = 0.f;
        t.shadow_alpha = 0;
        t.shadow_samples = 1;
        break;
    case Elevation::Low:
        t.shadow_radius = 4.f;
        t.shadow_offset_y = 2.f;
        t.shadow_alpha = 60;
        t.shadow_samples = 3;
        break;
    case Elevation::Medium:
        t.shadow_radius = 10.f;
        t.shadow_offset_y = 4.f;
        t.shadow_alpha = 80;
        t.shadow_samples = 3;
        break;
    case Elevation::High:
        t.shadow_radius = 18.f;
        t.shadow_offset_y = 8.f;
        t.shadow_alpha = 100;
        t.shadow_samples = 4;
        break;
    }
    return t;
}

inline int ClampAlpha(float a) {
    int v = static_cast<int>(std::lround(a));
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}

} // namespace detail

/// Derive elevation tuning for a level under a given surface material.
inline ElevationTokens ElevationPreset(Elevation level, theme::SurfaceStyle surface) {
    ElevationTokens t = detail::ElevationBase(level);
    if (level == Elevation::None)
        return t; // nothing to modulate

    switch (surface) {
    case theme::SurfaceStyle::Glass:
    case theme::SurfaceStyle::Frosted:
    case theme::SurfaceStyle::Acrylic:
        // Softer, more diffuse shadow + a bright rim so glass reads with depth.
        t.shadow_radius *= 1.15f;
        t.shadow_alpha = detail::ClampAlpha(t.shadow_alpha * 0.70f);
        t.rim_glow = true;
        t.glow_radius = t.shadow_radius * 0.6f;
        t.glow_alpha = detail::ClampAlpha(t.shadow_alpha * 0.9f) / 2 + 24;
        break;
    case theme::SurfaceStyle::Solid:
        // Firmer, slightly darker drop shadow; no rim.
        t.shadow_alpha = detail::ClampAlpha(t.shadow_alpha * 1.10f);
        t.rim_glow = false;
        break;
    case theme::SurfaceStyle::Minimal:
        // Very quiet — minimal chrome.
        t.shadow_radius *= 0.8f;
        t.shadow_alpha = detail::ClampAlpha(t.shadow_alpha * 0.40f);
        t.rim_glow = false;
        break;
    }
    return t;
}

/// Derive elevation tuning using the currently-active surface material.
inline ElevationTokens ElevationPreset(Elevation level) {
    return ElevationPreset(level, theme::ActiveSurfaceStyle());
}

/// Build a ready ShadowEffect for an elevation + material.
inline ShadowEffect MakeElevationShadow(Elevation level, theme::SurfaceStyle surface) {
    ElevationTokens t = ElevationPreset(level, surface);
    return ShadowEffect(t.shadow_radius, t.shadow_offset_x, t.shadow_offset_y,
                        IM_COL32(0, 0, 0, t.shadow_alpha), t.shadow_samples);
}

/// Build a ShadowEffect for an elevation using the active surface material.
inline ShadowEffect MakeElevationShadow(Elevation level) {
    return MakeElevationShadow(level, theme::ActiveSurfaceStyle());
}

/// Build the bright rim GlowEffect for an elevation + material. Returns a
/// zero-strength glow when the material doesn't call for a rim.
inline GlowEffect MakeElevationGlow(Elevation level, theme::SurfaceStyle surface,
                                    ImU32 color = IM_COL32(255, 255, 255, 255)) {
    ElevationTokens t = ElevationPreset(level, surface);
    int a = t.rim_glow ? t.glow_alpha : 0;
    ImU32 c = (color & 0x00FFFFFFu) | (static_cast<ImU32>(a) << 24);
    return GlowEffect(t.glow_radius, c, /*layers=*/4);
}

/// Build the rim GlowEffect for an elevation using the active surface material.
inline GlowEffect MakeElevationGlow(Elevation level, ImU32 color = IM_COL32(255, 255, 255, 255)) {
    return MakeElevationGlow(level, theme::ActiveSurfaceStyle(), color);
}

/// Stable, human-readable name for an elevation level.
inline const char* ElevationName(Elevation level) {
    switch (level) {
    case Elevation::None:
        return "None";
    case Elevation::Low:
        return "Low";
    case Elevation::Medium:
        return "Medium";
    case Elevation::High:
        return "High";
    }
    return "None";
}

} // namespace unigui::fx
