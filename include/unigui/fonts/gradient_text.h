#pragma once
#include <imgui.h>
#include <imgui_internal.h>

namespace unigui {

/// GradientText — renders text with horizontal colour gradient
struct GradientText {
    /// Render text with left-to-right gradient.
    /// Uses ImFont::RenderChar for per-character colour interpolation.
    static void Render(const char* text, ImU32 leftColor, ImU32 rightColor);

    /// Convenience: gradient with hex colours
    static void RenderHex(const char* text, unsigned lr, unsigned lg, unsigned lb,
                          unsigned rr, unsigned rg, unsigned rb);
};

} // namespace unigui
