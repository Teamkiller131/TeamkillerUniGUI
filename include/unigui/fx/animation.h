#pragma once
#include <unigui/fx/easing.h>
#include <functional>
#include <vector>
#include <string>
#include <chrono>

namespace unigui::fx {

// ═══════════════════════════════════════════════════════════════════════════════
// AnimationState — per-widget animation state
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
/// AnimationState — per-widget animation state.
///
/// **progress** is the target [0..1].  Play() starts interpolating toward 1.0
/// (or 0.0 for pingPong reverse).  Call Update(dt) each frame and use its
/// RETURN VALUE (already eased) for actual rendering.
///
///   AnimationState s;
///   s.Play(0.3f, EasingCurve::CubicOut);
///   float alpha = s.Update(dt);   // eased current value for rendering
///   if (!s.IsPlaying()) { /* done */ }
// ═══════════════════════════════════════════════════════════════════════════════

struct AnimationState {
    float      progress = 0.f;       // target [0..1] — Update() returns eased current
    EasingCurve curve   = EasingCurve::Ease;
    float      duration = 0.3f;      // seconds
    float      elapsed  = 0.f;       // seconds since Play()
    bool       playing  = false;
    bool       loop     = false;     // repeat from 0 after reaching 1
    bool       pingPong = false;     // reverse direction at end, then loop
    int        direction = 1;        // 1=forward, -1=reverse

    /// Start playing. Resets elapsed to 0, sets curve, returns eased progress via Update().
    void Play(float dur = -1.f, EasingCurve c = EasingCurve::Ease);

    /// Stop immediately (progress stays at current value).
    void Stop();

    /// Update by dt seconds. Returns current eased value.
    float Update(float dt);

    /// Backward-compatible convenience: update using elapsed=state_time_since_start.
    float UpdateFromStart(float timeSinceStart);

    /// Returns true if currently in an animation.
    bool IsPlaying() const { return playing; }

    /// Register callback fired when animation reaches end (or half-way in pingPong).
    std::function<void()> onComplete;
};

// ═══════════════════════════════════════════════════════════════════════════════
// AnimationManager — global animation registry (optional, for bulk updates)
// ═══════════════════════════════════════════════════════════════════════════════

class AnimationManager {
public:
    static AnimationManager& Instance();

    /// Register an animation state for bulk update.
    /// Returns a handle (index). Call Unregister to remove.
    int Register(AnimationState* state);

    /// Remove a registered animation.
    void Unregister(int handle);

    /// Update all registered animations by dt. Called once per frame.
    void UpdateAll(float dt);

    /// Pause / resume all.
    void SetPaused(bool paused) { paused_ = paused; }
    bool IsPaused() const { return paused_; }

private:
    AnimationManager() = default;
    std::vector<AnimationState*> states_;
    bool paused_ = false;
};

} // namespace unigui::fx
