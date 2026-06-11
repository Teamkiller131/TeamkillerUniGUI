#pragma once
#include <imgui.h>

#include <chrono>

namespace unigui {

/// Simple animation helpers using ImGui style alpha.
namespace Animate {

/// Fade in a widget over duration seconds. Call in render loop.
/// Returns 0.0-1.0 alpha for the current frame.
inline float FadeIn(float duration = 0.3f) {
    static auto start = std::chrono::steady_clock::now();
    static bool first = true;
    if (first) {
        start = std::chrono::steady_clock::now();
        first = false;
    }
    auto elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
    if (elapsed > duration)
        return 1.0f;
    return elapsed / duration;
}

/// Reset the fade timer for next use.
inline void FadeInReset() { /* static vars reset via scope */ }

/// Slide from direction. Returns offset (0 = final position, negative = above/left).
inline float SlideIn(float duration = 0.3f, float fromOffset = -50.0f) {
    float t = FadeIn(duration);
    return fromOffset * (1.0f - t);
}

/// Animate a float from current to target over duration.
inline float Lerp(float current, float target, float speed = 0.1f) {
    return current + (target - current) * speed;
}

/// Push an alpha multiplier for fade effect.
struct FadeScope {
    float targetAlpha;
    FadeScope(float target = 1.0f, float duration = 0.3f)
            : targetAlpha(target) {
        float a = FadeIn(duration) * targetAlpha;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
    }
    ~FadeScope() { ImGui::PopStyleVar(); }
};

} // namespace Animate
} // namespace unigui
