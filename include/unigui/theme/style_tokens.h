#pragma once
#include <imgui.h>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Unified design-language *size tokens* shared by every built-in theme preset
// and by the base Dark/Light themes in theme.cc.
//
// Historically each preset hard-coded its own rounding/spacing/border values,
// which produced subtly inconsistent geometry across themes (e.g. one theme used
// 12px window rounding, another 4px). Centralising the geometry here guarantees a
// single, consistent visual language while leaving each theme free to own its
// *colour* palette.
//
// Colour helpers (Lighten/Darken/WithAlpha and the Accent* derivations) make it
// easy to express the "accent + hover + active" relationship from a single base
// accent colour, instead of hand-tuning three near-identical colours per theme.
// ─────────────────────────────────────────────────────────────────────────────

namespace unigui::theme {

/// Geometry/spacing tokens. Colours are intentionally excluded — those belong to
/// each individual theme palette.
struct StyleTokens {
    // Corner rounding
    float window_rounding    = 6.0f;
    float frame_rounding     = 4.0f;
    float grab_rounding      = 4.0f;
    float tab_rounding       = 4.0f;
    float child_rounding     = 4.0f;
    float popup_rounding     = 4.0f;
    float scrollbar_rounding = 9.0f;

    // Borders (subtle layering: window + popup keep a hairline border, frames are
    // flat by default so inputs read as filled surfaces rather than outlined boxes)
    float window_border = 1.0f;
    float frame_border  = 0.0f;
    float popup_border  = 1.0f;
    float child_border  = 1.0f;
    float tab_border    = 0.0f;

    // Spacing / padding
    ImVec2 frame_padding      = ImVec2(8.0f, 6.0f);
    ImVec2 item_spacing       = ImVec2(8.0f, 6.0f);
    ImVec2 item_inner_spacing = ImVec2(6.0f, 6.0f);
    ImVec2 window_padding     = ImVec2(12.0f, 12.0f);
    float  scrollbar_size     = 14.0f;
    float  grab_min_size      = 10.0f;
};

/// Apply the unified geometry tokens to an ImGui style. Colours are left
/// untouched, so a preset can call this either before or after setting its
/// palette without clobbering colours.
inline void ApplyStyleTokens(ImGuiStyle& s, const StyleTokens& t = StyleTokens{}) {
    s.WindowRounding    = t.window_rounding;
    s.FrameRounding     = t.frame_rounding;
    s.GrabRounding      = t.grab_rounding;
    s.TabRounding       = t.tab_rounding;
    s.ChildRounding     = t.child_rounding;
    s.PopupRounding     = t.popup_rounding;
    s.ScrollbarRounding = t.scrollbar_rounding;

    s.WindowBorderSize = t.window_border;
    s.FrameBorderSize  = t.frame_border;
    s.PopupBorderSize  = t.popup_border;
    s.ChildBorderSize  = t.child_border;
    s.TabBorderSize    = t.tab_border;

    s.FramePadding     = t.frame_padding;
    s.ItemSpacing      = t.item_spacing;
    s.ItemInnerSpacing = t.item_inner_spacing;
    s.WindowPadding    = t.window_padding;
    s.ScrollbarSize    = t.scrollbar_size;
    s.GrabMinSize      = t.grab_min_size;

    // Keep the window collapse/menu button out of the title bar for a cleaner look.
    s.WindowMenuButtonPosition = ImGuiDir_None;
}

// ── Colour helpers ───────────────────────────────────────────────────────────

inline float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

/// Lighten an RGB colour by `amt` (0..1), preserving alpha.
inline ImVec4 Lighten(const ImVec4& c, float amt) {
    return ImVec4(Clamp01(c.x + amt), Clamp01(c.y + amt), Clamp01(c.z + amt), c.w);
}

/// Darken an RGB colour by `amt` (0..1), preserving alpha.
inline ImVec4 Darken(const ImVec4& c, float amt) {
    return ImVec4(Clamp01(c.x - amt), Clamp01(c.y - amt), Clamp01(c.z - amt), c.w);
}

/// Copy a colour with a new alpha value.
inline ImVec4 WithAlpha(const ImVec4& c, float a) {
    return ImVec4(c.x, c.y, c.z, Clamp01(a));
}

/// Derive the "hovered" variant of an accent colour (slightly brighter).
inline ImVec4 AccentHover(const ImVec4& accent, float amt = 0.10f) {
    return Lighten(accent, amt);
}

/// Derive the "active/pressed" variant of an accent colour (slightly darker).
inline ImVec4 AccentActive(const ImVec4& accent, float amt = 0.08f) {
    return Darken(accent, amt);
}

} // namespace unigui::theme
