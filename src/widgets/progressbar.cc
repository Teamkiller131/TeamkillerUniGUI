#include <unigui/fx/animation.h>
#include <unigui/fx/effect_scope.h>
#include <unigui/widgets/progressbar.h>

#include <imgui.h>

#include <cmath>
namespace unigui {
ProgressBar::ProgressBar(std::string name, float fraction)
        : FluentWidget<ProgressBar>(std::move(name))
        , fraction_(fraction) {}
void ProgressBar::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    // Animated fill
    float target = fraction_;
    float displayFraction = fraction_;
    if (anim_.progress != target) {
        if (!anim_.IsPlaying())
            anim_.Play(0.3f, fx::EasingCurve::CubicOut);
        anim_.Update(ImGui::GetIO().DeltaTime);
        displayFraction = anim_.progress;
    }

    displayFraction = displayFraction < 0.f ? 0.f : (displayFraction > 1.f ? 1.f : displayFraction);
    ImVec2 size(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton((GetName() + "##progress").c_str(), size);
    const bool itemFocused = ImGui::IsItemFocused();
    ReportAccessible(a11y::Role::Progress, itemFocused,
                     std::to_string(static_cast<int>(std::lround(displayFraction * 100.f))) + "%");
    auto* dl = ImGui::GetWindowDrawList();
    ImU32 bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bg,
                      ImGui::GetStyle().FrameRounding);

    float fillW = size.x * displayFraction;
    if (fillW > 0.f) {
        ImU32 left = gradC1_, right = gradC3_;
        if (displayFraction <= gradT1_) {
            left = gradC1_;
            right = gradC1_;
        } else if (displayFraction <= gradT2_) {
            left = gradC1_;
            right = gradC2_;
        } else {
            left = gradC2_;
            right = gradC3_;
        }
        fx::GradientBrush::Horizontal(dl, pos, ImVec2(pos.x + fillW, pos.y + size.y), left, right);
    }

    if (!overlay_.empty()) {
        ImVec2 textSize = ImGui::CalcTextSize(overlay_.c_str());
        ImVec2 textPos(pos.x + (size.x - textSize.x) * 0.5f, pos.y + (size.y - textSize.y) * 0.5f);
        dl->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), overlay_.c_str());
    }

    // Reset animation when target reached
    if (anim_.progress >= target - 0.001f && anim_.progress <= target + 0.001f) {
        anim_.Stop();
        anim_.progress = target;
    }
    ImGui::PopID();
}
void ProgressBar::SetFraction(float f) {
    fraction_ = f;
}
float ProgressBar::GetFraction() const {
    return fraction_;
}
void ProgressBar::SetState(State s) {
    state_ = s;
}
void ProgressBar::SetOverlayText(std::string text) {
    overlay_ = std::move(text);
}
void ProgressBar::SetGradient(float t1, ImU32 c1, float t2, ImU32 c2, ImU32 c3) {
    gradT1_ = t1;
    gradC1_ = c1;
    gradT2_ = t2;
    gradC2_ = c2;
    gradC3_ = c3;
}
} // namespace unigui
