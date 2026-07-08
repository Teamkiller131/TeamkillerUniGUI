#pragma once
#include <unigui/fx/animation.h>
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <string>

namespace unigui {
class ProgressBar : public FluentWidget<ProgressBar> {
public:
    enum State { Normal, Warning, Error };
    ProgressBar(std::string name, float fraction = 0.0f);
    void Render() override;
    void SetFraction(float f);
    float GetFraction() const;
    void SetState(State s);
    void SetOverlayText(std::string text);
    void SetGradient(float t1, ImU32 c1, float t2, ImU32 c2, ImU32 c3);

    // ── Fluent (chainable) helpers — return ProgressBar& via CRTP base ──────────
    ProgressBar& WithFraction(float f) {
        SetFraction(f);
        return *this;
    }
    ProgressBar& WithState(State s) {
        SetState(s);
        return *this;
    }
    ProgressBar& WithOverlayText(std::string text) {
        SetOverlayText(std::move(text));
        return *this;
    }
    ProgressBar& WithGradient(float t1, ImU32 c1, float t2, ImU32 c2, ImU32 c3) {
        SetGradient(t1, c1, t2, c2, c3);
        return *this;
    }

private:
    float fraction_;
    State state_ = Normal;
    std::string overlay_;
    float gradT1_ = 0.f, gradT2_ = 1.f;
    ImU32 gradC1_ = IM_COL32(34, 197, 94, 255), gradC2_ = IM_COL32(250, 204, 21, 255),
          gradC3_ = IM_COL32(239, 68, 68, 255);
    fx::AnimationState anim_;
};
} // namespace unigui
