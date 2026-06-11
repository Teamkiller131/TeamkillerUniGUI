#pragma once
#include <unigui/fx/animation.h>

#include <imgui.h>

namespace unigui::fx {

// ═══════════════════════════════════════════════════════════════════════════════
// Transition — header-only animation helpers for widgets
// ═══════════════════════════════════════════════════════════════════════════════

struct Transition {

    /// Fade a widget's alpha from 0→1 over duration.
    /// Usage: float a = Transition::Fade(state, 0.5f, EasingCurve::EaseOut, dt);
    ///         ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
    ///         // ... draw widget ...
    ///         ImGui::PopStyleVar();
    static float Fade(AnimationState& state, float duration,
                      EasingCurve curve = EasingCurve::EaseOut, float dt = 0.f) {
        if (dt > 0.f)
            state.Update(dt);
        return state.progress;
    }

    /// Slide a widget from an offset position. Returns offset to apply.
    /// Usage: float off = Transition::SlideIn(state, -50.f, 0.3f, dt);
    ///         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    static float SlideIn(AnimationState& state, float fromOffset, float duration = 0.3f,
                         EasingCurve curve = EasingCurve::EaseOut, float dt = 0.f) {
        if (!state.IsPlaying() && state.progress < 0.001f)
            state.Play(duration, curve);
        float v = state.Update(dt);
        return fromOffset * (1.f - v); // 0 when done
    }

    /// Scale from fromScale→toScale.
    /// Returns current scale factor.
    static float Scale(AnimationState& state, float fromScale, float toScale, float duration = 0.3f,
                       EasingCurve curve = EasingCurve::EaseOut, float dt = 0.f) {
        if (!state.IsPlaying() && state.progress < 0.001f)
            state.Play(duration, curve);
        float v = state.Update(dt);
        return fromScale + (toScale - fromScale) * v;
    }

    /// Cross-fade between two widgets: returns alpha for first widget.
    /// Second widget alpha = 1.0 - returned.
    static float CrossFade(AnimationState& state, float duration = 0.3f,
                           EasingCurve curve = EasingCurve::EaseInOut, float dt = 0.f) {
        if (!state.IsPlaying() && state.progress < 0.001f)
            state.Play(duration, curve);
        float v = state.Update(dt);
        return 1.f - v; // old widget fades out
    }

    /// Page-switch convenience: auto-play on trigger.
    /// Returns transition progress [0..1].
    static float PageSwitch(AnimationState& state, bool trigger, float duration = 0.25f,
                            EasingCurve curve = EasingCurve::EaseInOut, float dt = 0.f) {
        if (trigger && !state.IsPlaying())
            state.Play(duration, curve);
        return state.Update(dt);
    }

    /// Convenience: play an animation once, get alpha for the "entering" element.
    /// Returns alpha in [0..1].
    static float Appear(AnimationState& state, float duration = 0.3f,
                        EasingCurve curve = EasingCurve::CubicOut, float dt = 0.f) {
        if (!state.IsPlaying() && state.progress < 0.001f)
            state.Play(duration, curve);
        return state.Update(dt);
    }

    /// Convenience: shrink-then-remove animation for disappearing elements.
    /// Returns scale in [1..0].
    static float Disappear(AnimationState& state, float duration = 0.2f,
                           EasingCurve curve = EasingCurve::CubicIn, float dt = 0.f) {
        if (!state.IsPlaying() && state.progress < 0.001f)
            state.Play(duration, curve);
        float v = state.Update(dt);
        return 1.f - v * 0.5f; // shrinks to 50%
    }
};

} // namespace unigui::fx
