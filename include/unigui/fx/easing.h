#pragma once
#include <cmath>
#include <string>
#include <string_view>

namespace unigui::fx {

/// 10 standard easing curves.  All return y in [0,1] for x in [0,1].
enum class EasingCurve {
    Linear,
    QuadIn, QuadOut, QuadInOut,
    CubicIn, CubicOut,
    ExpoIn, ExpoOut,
    ElasticOut,
    BounceOut,
    // --- aliases ---
    Ease        = CubicOut,   // CSS "ease"
    EaseIn      = QuadIn,
    EaseOut     = QuadOut,
    EaseInOut   = QuadInOut,
};

// ── Core math ──────────────────────────────────────────────────────────────

constexpr float pi    = 3.14159265358979323846f;
constexpr float c1    = 1.70158f;           // Back/elastic constant
constexpr float c2    = c1 * 1.525f;
constexpr float c3    = c1 + 1.f;
constexpr float c4    = (2.f * pi) / 3.f;
constexpr float c5    = (2.f * pi) / 4.5f;

inline float linear(float t) { return t; }

// ── Quad ───────────────────────────────────────────────────────────────────

inline float quadIn(float t)     { return t * t; }
inline float quadOut(float t)    { return 1.f - (1.f - t) * (1.f - t); }
inline float quadInOut(float t)  { return t < 0.5f ? 2.f * t * t
                                                   : 1.f - std::pow(-2.f * t + 2.f, 2.f) / 2.f; }

// ── Cubic ──────────────────────────────────────────────────────────────────

inline float cubicIn(float t)    { return t * t * t; }
inline float cubicOut(float t)   { return 1.f - std::pow(1.f - t, 3.f); }

// ── Expo ───────────────────────────────────────────────────────────────────

inline float expoIn(float t)     { return t == 0.f ? 0.f : std::pow(2.f, 10.f * t - 10.f); }
inline float expoOut(float t)    { return t == 1.f ? 1.f : 1.f - std::pow(2.f, -10.f * t); }

// ── Elastic ────────────────────────────────────────────────────────────────

inline float elasticOut(float t) {
    if (t == 0.f || t == 1.f) return t;
    return std::pow(2.f, -10.f * t) * std::sin((t * 10.f - 0.75f) * c4) + 1.f;
}

// ── Bounce ─────────────────────────────────────────────────────────────────

inline float bounceOut(float t) {
    constexpr float n1 = 7.5625f;
    constexpr float d1 = 2.75f;
    if (t < 1.f / d1)
        return n1 * t * t;
    if (t < 2.f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    }
    if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    }
    t -= 2.625f / d1;
    return n1 * t * t + 0.984375f;
}

// ── Dispatcher ─────────────────────────────────────────────────────────────

inline float Ease(float t, EasingCurve c = EasingCurve::Linear) {
    switch (c) {
    case EasingCurve::Linear:     return linear(t);
    case EasingCurve::QuadIn:     return quadIn(t);
    case EasingCurve::QuadOut:    return quadOut(t);
    case EasingCurve::QuadInOut:  return quadInOut(t);
    case EasingCurve::CubicIn:    return cubicIn(t);
    case EasingCurve::CubicOut:   return cubicOut(t);
    case EasingCurve::ExpoIn:     return expoIn(t);
    case EasingCurve::ExpoOut:    return expoOut(t);
    case EasingCurve::ElasticOut: return elasticOut(t);
    case EasingCurve::BounceOut:  return bounceOut(t);
    default:                      return t;
    }
}

/// Parse a string name to EasingCurve.
/// Accepted: "linear", "quadIn", "bounceOut", "ease", "ease-in-out", etc.
inline EasingCurve ParseEasing(std::string_view name) {
    if (name == "linear")       return EasingCurve::Linear;
    if (name == "quadIn")       return EasingCurve::QuadIn;
    if (name == "quadOut")      return EasingCurve::QuadOut;
    if (name == "quadInOut")    return EasingCurve::QuadInOut;
    if (name == "cubicIn")      return EasingCurve::CubicIn;
    if (name == "cubicOut")     return EasingCurve::CubicOut;
    if (name == "expoIn")       return EasingCurve::ExpoIn;
    if (name == "expoOut")      return EasingCurve::ExpoOut;
    if (name == "elasticOut")   return EasingCurve::ElasticOut;
    if (name == "bounceOut")    return EasingCurve::BounceOut;
    // CSS aliases
    if (name == "ease")         return EasingCurve::Ease;
    if (name == "ease-in")      return EasingCurve::EaseIn;
    if (name == "ease-out")     return EasingCurve::EaseOut;
    if (name == "ease-in-out")  return EasingCurve::EaseInOut;
    return EasingCurve::Linear;  // fallback
}

} // namespace unigui::fx
