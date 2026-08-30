#include <unigui/fx/animation.h>

#include "../detail/context_registry.h"

#include <algorithm>

namespace unigui::fx {

// ═══════════════════════════════════════════════════════════════════════════════
// AnimationState
// ═══════════════════════════════════════════════════════════════════════════════

void AnimationState::Play(float dur, EasingCurve c) {
    if (dur > 0.f)
        duration = dur;
    curve = c;
    elapsed = 0.f;
    playing = true;
    direction = 1;
}

void AnimationState::Stop() {
    playing = false;
}

float AnimationState::Update(float dt) {
    if (!playing)
        return Ease(progress, curve);
    elapsed += dt;
    float t = (duration > 0.f) ? (elapsed / duration) : 1.f;
    if (t >= 1.f) {
        if (loop) {
            elapsed = 0.f;
            t = 0.f;
        } else if (pingPong) {
            direction *= -1;
            elapsed = 0.f;
            t = 0.f;
            if (onComplete)
                onComplete();
        } else {
            t = 1.f;
            progress = 1.f;
            playing = false;
            if (onComplete)
                onComplete();
            return Ease(1.f, curve);
        }
    }
    // For pingPong reverse: map t → [0..1..0]
    float easedT = (direction < 0) ? (1.f - t) : t;
    progress = easedT;
    return Ease(easedT, curve);
}

float AnimationState::UpdateFromStart(float timeSinceStart) {
    if (duration <= 0.f)
        return Ease(1.f, curve);
    float t = timeSinceStart / duration;
    t = (t < 0.f) ? 0.f : ((t > 1.f) ? 1.f : t);
    progress = t;
    if (t >= 0.999f) {
        playing = false;
        if (onComplete)
            onComplete();
    }
    return Ease(t, curve);
}

// ═══════════════════════════════════════════════════════════════════════════════
// AnimationManager
// ═══════════════════════════════════════════════════════════════════════════════

AnimationManager& AnimationManager::Instance() {
    return detail::ContextRegistry<AnimationManager>::Instance();
}

int AnimationManager::Register(AnimationState* state) {
    states_.push_back(state);
    return (int) states_.size() - 1;
}

void AnimationManager::Unregister(int handle) {
    if (handle >= 0 && handle < (int) states_.size())
        states_[handle] = nullptr; // lazy cleanup
}

void AnimationManager::UpdateAll(float dt) {
    if (paused_)
        return;
    for (auto* s : states_) {
        if (s)
            s->Update(dt);
    }
}

} // namespace unigui::fx
