#include <unigui/widgets/progressbar.h>
#include <unigui/fx/animation.h>
#include <unigui/fx/effect_scope.h>
#include <imgui.h>
namespace unigui {
ProgressBar::ProgressBar(std::string name, float fraction)
    : Widget(std::move(name)), fraction_(fraction) {}
void ProgressBar::Render() {
    if (!IsVisible()) return;

    // Animated fill
    float target = fraction_;
    if (anim_.progress != target) {
        if (!anim_.IsPlaying()) anim_.Play(0.3f, fx::EasingCurve::CubicOut);
        anim_.Update(ImGui::GetIO().DeltaTime);
        float displayFraction = anim_.progress;
        ImGui::ProgressBar(displayFraction, ImVec2(-1, 0),
                          overlay_.empty() ? nullptr : overlay_.c_str());
    } else {
        ImGui::ProgressBar(fraction_, ImVec2(-1, 0),
                          overlay_.empty() ? nullptr : overlay_.c_str());
    }

    // Reset animation when target reached
    if (anim_.progress >= target - 0.001f && anim_.progress <= target + 0.001f) {
        anim_.Stop();
        anim_.progress = target;
    }
}
void ProgressBar::SetFraction(float f) { fraction_ = f; }
float ProgressBar::GetFraction() const { return fraction_; }
void ProgressBar::SetState(State s) { state_ = s; }
void ProgressBar::SetOverlayText(std::string text) { overlay_ = std::move(text); }
}
