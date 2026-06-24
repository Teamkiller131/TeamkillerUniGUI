#pragma once

#include <algorithm>
#include <cmath>

namespace unigui::dpi {

// ─────────────────────────────────────────────────────────────────────────────
// HiDPI content-scale helpers (pure, header-only).
//
// Dear ImGui ≥1.92 re-rasterises fonts on the fly from `ImGuiStyle::FontScaleDpi`
// (see App::SetContentScale). Platforms, however, report DPI in inconsistent and
// often fractional forms — GLFW's `glfwGetWindowContentScale` can hand back
// 1.4583 on a "150%" monitor — and feeding that straight in gives slightly soft,
// unpredictable glyphs. These helpers turn a raw platform value into a crisp,
// bounded, snapped content scale. Pure math: no ImGui, no platform calls, trivial
// to unit-test.
// ─────────────────────────────────────────────────────────────────────────────

/// Convert a raw monitor DPI to a content scale relative to the 96-DPI baseline
/// (the Windows / CSS reference): 96 → 1.0, 120 → 1.25, 144 → 1.5, 192 → 2.0.
/// A non-positive `dpi` or `baseDpi` falls back to 1.0.
inline float DpiToScale(float dpi, float baseDpi = 96.0f) {
    if (!(dpi > 0.0f) || !(baseDpi > 0.0f))
        return 1.0f;
    return dpi / baseDpi;
}

/// Normalise a raw content scale into a crisp, bounded value: clamp to
/// `[minScale, maxScale]`, then snap to the nearest multiple of `step`. Snapping
/// keeps fractional platform scales (e.g. 1.4583) on predictable boundaries
/// (→ 1.5) so glyph rasterisation stays sharp. A `step <= 0` disables snapping
/// (clamp only). A non-positive / NaN `raw` is treated as 1.0.
inline float NormalizeContentScale(float raw, float minScale = 1.0f, float maxScale = 4.0f,
                                   float step = 0.25f) {
    if (!(raw > 0.0f)) // catches NaN and non-positive
        raw = 1.0f;
    if (maxScale < minScale)
        std::swap(minScale, maxScale);
    float s = std::clamp(raw, minScale, maxScale);
    if (step > 0.0f) {
        s = std::round(s / step) * step;
        s = std::clamp(s, minScale, maxScale); // re-clamp in case snapping overshot a bound
    }
    return s;
}

} // namespace unigui::dpi
