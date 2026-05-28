#include <unigui/widgets/button.h>
#include <unigui/fx/animation.h>
#include <imgui.h>

namespace unigui {

Button::Button(std::string name, std::string label)
    : Widget(std::move(name)), label_(std::move(label)) {
}

void Button::Render() {
    if (!IsVisible()) return;
    ImGui::BeginDisabled(!enabled_);

    // ── Determine base color ──────────────────────────────────────────────
    ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Button];
    switch (variant_) {
    case Primary: color = ImVec4(0.16f, 0.47f, 0.80f, 1.0f); break;
    case Danger:  color = ImVec4(0.80f, 0.16f, 0.20f, 1.0f); break;
    case Success: color = ImVec4(0.18f, 0.60f, 0.28f, 1.0f); break;
    default: break;
    }

    // ── Animated hover — push interpolated colors ─────────────────────────
    bool wantHover = ImGui::IsItemHovered() || ImGui::IsItemActive();
    float target = wantHover ? 1.f : 0.f;

    if (anim_.IsPlaying() || wantHover || anim_.progress > 0.01f) {
        if (wantHover && !anim_.IsPlaying()) anim_.Play(0.15f);
        if (!wantHover && anim_.progress > 0.99f) anim_.Stop();
        anim_.Update(ImGui::GetIO().DeltaTime);
    }

    float t = anim_.progress;  // 0..1 hover interpolation
    ImVec4 hoverCol(color.x + (1.f - color.x) * t * 0.25f,
                    color.y + (1.f - color.y) * t * 0.25f,
                    color.z + (1.f - color.z) * t * 0.20f, 1.f);
    ImVec4 activeCol(color.x * (1.f - t * 0.15f),
                     color.y * (1.f - t * 0.15f),
                     color.z * (1.f - t * 0.15f), 1.f);

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeCol);

    ImVec2 size(0,0);
    if (sz_ == Small) size = ImVec2(80, 24);
    else if (sz_ == Large) size = ImVec2(180, 36);
    ImGui::PushID(GetName().c_str());
    if (size.x > 0) clicked_ = ImGui::Button(label_.c_str(), size);
    else clicked_ = ImGui::Button(label_.c_str());
    ImGui::PopID();

    ImGui::PopStyleColor(3);
    ImGui::EndDisabled();
}

bool Button::WasClicked() const { return clicked_; }
void Button::SetEnabled(bool enabled) { enabled_ = enabled; }
bool Button::IsEnabled() const { return enabled_; }
const std::string& Button::GetLabel() const { return label_; }
void Button::SetLabel(std::string label) { label_ = std::move(label); }
void Button::SetColorVariant(ColorVariant v) { variant_ = v; }
void Button::SetSize(Size s) { sz_ = s; }

} // namespace unigui
