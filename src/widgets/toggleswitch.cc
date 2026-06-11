#include <unigui/widgets/toggleswitch.h>

#include <imgui.h>
namespace unigui {
ToggleSwitch::ToggleSwitch(std::string n, std::string l, bool on)
        : ValueWidget<bool>(std::move(n), on)
        , label_(std::move(l)) {}
void ToggleSwitch::Render() {
    if (!IsVisible())
        return;
    bool prev = value_;

    // Animated transition
    float target = value_ ? 1.f : 0.f;
    if (anim_.progress != target && !anim_.IsPlaying())
        anim_.Play(0.2f, fx::EasingCurve::EaseOut);
    float t = anim_.Update(ImGui::GetIO().DeltaTime);

    // Push alpha for smooth on/off feel
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f + 0.3f * t);
    ImGui::PushID(GetName().c_str());
    ImGui::Checkbox(label_.c_str(), &value_);
    ImGui::PopID();
    ImGui::PopStyleVar();

    NotifyChange(prev);
}
} // namespace unigui
