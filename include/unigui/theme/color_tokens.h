#pragma once
#include <unigui/theme/style_tokens.h>

#include <imgui.h>

#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// Accent & semantic colour tokens (UI beautification — Step 3)
//
// Step 1 unified the *geometry* tokens and Step 2 layered a *surface material*.
// Step 3 unifies the *interactive colour relationship*: every theme expresses its
// accent → hover → active progression from a single base accent (reusing the
// AccentHover/AccentActive helpers from style_tokens.h) and exposes a consistent
// set of semantic colours (success / warning / danger / info) tuned for the
// theme's luminance.
//
// Historically each theme hand-tuned the accent-driven slots (CheckMark,
// SliderGrabActive, SeparatorActive, ResizeGrip*, DragDropTarget, NavHighlight,
// DockingPreview, TextSelectedBg) independently, so the same logical "accent"
// produced subtly different hover/active relationships across presets. Deriving
// them from one base accent guarantees a single, consistent interactive language
// while every theme keeps its own accent *hue*.
//
// `ApplyColorTokens()` only rewrites the accent-semantic interaction slots; it
// never touches surface alphas (Step 2), geometry (Step 1), or a preset's
// independent Button/Header/Tab/Plot palette, so it composes with Dark/Light and
// every registry preset.
// ─────────────────────────────────────────────────────────────────────────────

namespace unigui::theme {

/// Logical semantic colour roles. `Accent`/`Info` follow the theme accent; the
/// rest carry canonical hues tuned for the active luminance.
enum class Semantic {
    Accent,  ///< Primary interactive accent.
    Success, ///< Positive / confirmation (green).
    Warning, ///< Caution (amber).
    Danger,  ///< Error / destructive (red).
    Info,    ///< Informational (follows the accent).
};

/// A theme's full interactive colour palette, derived from one base accent.
struct ColorTokens {
    ImVec4 accent;        ///< Base accent.
    ImVec4 accent_hover;  ///< Brighter accent for hover.
    ImVec4 accent_active; ///< Darker accent for active/pressed.
    ImVec4 success;       ///< Semantic success (green).
    ImVec4 warning;       ///< Semantic warning (amber).
    ImVec4 danger;        ///< Semantic danger (red).
    ImVec4 info;          ///< Semantic info (== accent).
    bool dark = true;     ///< Whether tuned for a dark background.
};

/// Derive the full token set from a single base accent. `dark` selects luminance-
/// appropriate semantic hues (brighter on dark backgrounds, deeper on light ones)
/// and whether the "active" accent is reached by darkening (dark) or by a stronger
/// darken (light), mirroring the historical Dark/Light hand-tuned behaviour.
inline ColorTokens DeriveColorTokens(const ImVec4& accent, bool dark) {
    ColorTokens t;
    t.accent = accent;
    t.accent_hover = AccentHover(accent);
    t.accent_active = AccentActive(accent);
    t.info = accent;
    t.dark = dark;
    if (dark) {
        // Brighter, slightly desaturated semantics read better on dark surfaces.
        t.success = ImVec4(0.30f, 0.78f, 0.45f, accent.w);
        t.warning = ImVec4(0.95f, 0.70f, 0.20f, accent.w);
        t.danger = ImVec4(0.92f, 0.34f, 0.34f, accent.w);
    } else {
        // Deeper semantics keep contrast against light surfaces.
        t.success = ImVec4(0.18f, 0.62f, 0.34f, accent.w);
        t.warning = ImVec4(0.80f, 0.55f, 0.10f, accent.w);
        t.danger = ImVec4(0.82f, 0.24f, 0.24f, accent.w);
    }
    return t;
}

/// Human-readable name for a semantic role (for pickers/logs).
inline const char* SemanticName(Semantic s) {
    switch (s) {
    case Semantic::Accent:
        return "Accent";
    case Semantic::Success:
        return "Success";
    case Semantic::Warning:
        return "Warning";
    case Semantic::Danger:
        return "Danger";
    case Semantic::Info:
        return "Info";
    }
    return "Accent";
}

/// Pick a semantic colour out of a token set.
inline ImVec4 SemanticColor(const ColorTokens& t, Semantic s) {
    switch (s) {
    case Semantic::Accent:
        return t.accent;
    case Semantic::Success:
        return t.success;
    case Semantic::Warning:
        return t.warning;
    case Semantic::Danger:
        return t.danger;
    case Semantic::Info:
        return t.info;
    }
    return t.accent;
}

namespace detail {
/// Last-applied tokens, so widgets can query the active semantic palette without
/// threading it through every call. `inline` gives a single instance across TUs.
inline ColorTokens g_active_color_tokens =
    DeriveColorTokens(ImVec4(0.40f, 0.58f, 0.93f, 1.00f), /*dark=*/true);
} // namespace detail

/// The colour tokens from the most recent ApplyColorTokens() call.
inline const ColorTokens& ActiveColorTokens() {
    return detail::g_active_color_tokens;
}

/// Active semantic colour by role (updated on every ApplyColorTokens()).
inline ImVec4 GetSemanticColor(Semantic s) {
    return SemanticColor(detail::g_active_color_tokens, s);
}

/// Apply the accent relationship to the accent-driven ImGui slots so every theme
/// shares a consistent accent → hover → active progression. Records the tokens as
/// the active semantic palette. Does NOT touch surface alphas, geometry, or a
/// preset's independent Button/Header/Tab/Plot palette.
inline void ApplyColorTokens(ImGuiStyle& s, const ColorTokens& t) {
    auto& c = s.Colors;
    c[ImGuiCol_CheckMark] = t.accent;
    c[ImGuiCol_SliderGrab] = t.accent;
    c[ImGuiCol_SliderGrabActive] = t.dark ? t.accent_hover : t.accent_active;
    c[ImGuiCol_SeparatorActive] = t.accent;
    c[ImGuiCol_ResizeGripHovered] = t.accent;
    c[ImGuiCol_ResizeGripActive] = t.accent;
    c[ImGuiCol_DragDropTarget] = t.accent;
    c[ImGuiCol_NavHighlight] = t.accent;
    c[ImGuiCol_DockingPreview] = WithAlpha(t.accent, 0.70f);
    c[ImGuiCol_TextSelectedBg] = WithAlpha(t.accent, 0.35f);
    detail::g_active_color_tokens = t;
}

/// Convenience: derive tokens from a base accent and apply them.
inline void ApplyColorTokens(ImGuiStyle& s, const ImVec4& accent, bool dark) {
    ApplyColorTokens(s, DeriveColorTokens(accent, dark));
}

/// Extract a base accent from an already-populated style. Uses CheckMark — the
/// slot every built-in theme sets to its accent colour — so registry presets that
/// don't expose an explicit accent value can still be unified.
inline ImVec4 AccentFromStyle(const ImGuiStyle& s) {
    return s.Colors[ImGuiCol_CheckMark];
}

/// Heuristic: is the style's window background dark? (luminance < 0.5)
inline bool StyleIsDark(const ImGuiStyle& s) {
    const ImVec4& bg = s.Colors[ImGuiCol_WindowBg];
    float lum = 0.299f * bg.x + 0.587f * bg.y + 0.114f * bg.z;
    return lum < 0.5f;
}

/// All semantic roles, in display order.
inline const std::array<Semantic, 5>& AllSemantics() {
    static const std::array<Semantic, 5> kAll = {
        Semantic::Accent, Semantic::Success, Semantic::Warning, Semantic::Danger, Semantic::Info};
    return kAll;
}

} // namespace unigui::theme
