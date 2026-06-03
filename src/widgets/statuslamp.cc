#include <unigui/widgets/statuslamp.h>
#include <imgui.h>
#include <cmath>

namespace unigui {

StatusLamp::StatusLamp(std::string name, State state)
    : Widget(std::move(name)), state_(state) {
}

namespace {

// Scale an RGB color's brightness, keeping alpha intact.
ImU32 ScaleBrightness(ImU32 c, float factor) {
    ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
    auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };
    v.x = clamp01(v.x * factor);
    v.y = clamp01(v.y * factor);
    v.z = clamp01(v.z * factor);
    return ImGui::ColorConvertFloat4ToU32(v);
}

ImU32 WithAlpha(ImU32 c, float a) {
    ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
    v.w = a < 0.f ? 0.f : (a > 1.f ? 1.f : a);
    return ImGui::ColorConvertFloat4ToU32(v);
}

} // namespace

void StatusLamp::Render() {
    if (!IsVisible()) return;

    ImGui::PushID(GetName().c_str());

    // Horizontal centering within the available cell width
    if (centerInCell_) {
        const float diameter = radius_ * 2.0f;
        const float pad = glow_ ? radius_ * 0.8f : 0.0f;
        const float totalW = diameter + pad * 2.0f;
        const float avail = ImGui::GetContentRegionAvail().x;
        if (avail > totalW) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - totalW) * 0.5f);
        }
    }

    // -- Base color + animation per state --------------------------------
    ImU32 baseColor;
    float coreAlpha = 1.0f;   // opacity of the lit core
    float glowAlpha = 0.0f;   // opacity of the outer halo
    bool  lit = true;         // whether the lamp emits light (drives glow)

    switch (state_) {
    case Running:
        baseColor = IM_COL32(0x2e, 0xd1, 0x5e, 0xFF); // emerald green
        break;
    case Draft: {
        baseColor = IM_COL32(0xf0, 0xa8, 0x36, 0xFF); // amber
        blinkTimer_ += ImGui::GetIO().DeltaTime;
        float raw = std::sin(blinkTimer_ * 2.0f * 3.14159265f / 1.2f);
        coreAlpha = 0.35f + 0.65f * (raw + 1.0f) * 0.5f; // pulse 0.35..1
        break;
    }
    case Error: {
        baseColor = IM_COL32(0xe5, 0x3e, 0x3e, 0xFF); // red
        blinkTimer_ += ImGui::GetIO().DeltaTime;
        float raw = std::sin(blinkTimer_ * 2.0f * 3.14159265f / 0.7f);
        coreAlpha = 0.40f + 0.60f * (raw + 1.0f) * 0.5f; // faster pulse
        break;
    }
    case Warning:
        baseColor = IM_COL32(0xf5, 0xc2, 0x2c, 0xFF); // yellow
        break;
    case Paused:
        baseColor = IM_COL32(0x4d, 0x8c, 0xff, 0xFF); // blue
        break;
    case Off:
    default:
        baseColor = IM_COL32(0x4a, 0x4a, 0x52, 0xFF); // dim gray
        lit = false;
        break;
    }

    if (customColor_ != 0) baseColor = customColor_;

    // Gentle "breathing" glow for steady lit states.
    if (lit) {
        if (state_ == Running || state_ == Warning || state_ == Paused)
            blinkTimer_ += ImGui::GetIO().DeltaTime;
        float breathe = (std::sin(blinkTimer_ * 2.0f) + 1.0f) * 0.5f; // 0..1
        glowAlpha = (0.25f + 0.20f * breathe) * coreAlpha;
    }

    const float r = radius_;
    const float diameter = r * 2.0f;
    const float pad = glow_ ? r * 0.8f : 0.0f; // room for halo

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 center(origin.x + pad + r, origin.y + pad + r);

    // Reserve layout box (include halo padding on both axes so neighbors
    // don't overlap and the glow isn't clipped).
    ImGui::Dummy(ImVec2(diameter + pad * 2.0f, diameter + pad * 2.0f));

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // -- Outer glow halo (layered translucent circles) -------------------
    if (glow_ && glowAlpha > 0.001f) {
        dl->AddCircleFilled(center, r * 1.9f, WithAlpha(baseColor, glowAlpha * 0.35f), 32);
        dl->AddCircleFilled(center, r * 1.5f, WithAlpha(baseColor, glowAlpha * 0.55f), 32);
        dl->AddCircleFilled(center, r * 1.2f, WithAlpha(baseColor, glowAlpha * 0.85f), 32);
    }

    // -- Lamp body: dark rim -> lit core -> top gradient -> highlight -----
    ImU32 rim    = ScaleBrightness(baseColor, lit ? 0.45f : 0.7f);
    ImU32 core   = WithAlpha(baseColor, lit ? coreAlpha : 1.0f);
    ImU32 topLit = ScaleBrightness(baseColor, 1.35f);

    dl->AddCircleFilled(center, r, rim, 32);                    // rim/base
    dl->AddCircleFilled(center, r * 0.86f, core, 32);           // body
    // Upper gradient cap (fake radial light from the top).
    dl->AddCircleFilled(ImVec2(center.x, center.y - r * 0.18f),
                        r * 0.55f, WithAlpha(topLit, (lit ? coreAlpha : 1.0f) * 0.55f), 24);
    // Specular highlight dot near top-left.
    dl->AddCircleFilled(ImVec2(center.x - r * 0.28f, center.y - r * 0.32f),
                        r * 0.22f, WithAlpha(IM_COL32_WHITE, lit ? 0.75f : 0.35f), 16);
    // Crisp outline.
    dl->AddCircle(center, r, ScaleBrightness(baseColor, lit ? 0.6f : 0.85f), 32, 1.0f);

    // Tooltip on hover.
    if (!tooltip_.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip_.c_str());
    }

    ImGui::PopID();
}

void StatusLamp::SetState(State s) {
    state_ = s;
    if (s == Off) {
        blinkTimer_ = 0.0f; // reset animation when turned off
    }
}

void StatusLamp::SetTooltip(std::string text) {
    tooltip_ = std::move(text);
}

} // namespace unigui
