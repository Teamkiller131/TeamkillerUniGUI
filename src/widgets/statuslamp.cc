#include <unigui/widgets/statuslamp.h>
#include <imgui.h>
#include <cmath>

namespace unigui {

StatusLamp::StatusLamp(std::string name, State state)
    : Widget(std::move(name)), state_(state) {
}

void StatusLamp::Render() {
    if (!IsVisible()) return;

    ImGui::PushID(GetName().c_str());

    // Determine color and opacity based on state
    ImU32 color;
    float alpha = 1.0f;

    switch (state_) {
    case Running:
        color = IM_COL32(0x28, 0xa7, 0x45, 0xFF); // green #28a745
        break;
    case Draft: {
        // Blink animation: sin wave, period 1.2s
        color = IM_COL32(0xf0, 0xa0, 0x40, 0xFF); // orange #f0a040
        blinkTimer_ += ImGui::GetIO().DeltaTime;
        // Map sin to 0..1 (sin ranges -1..1, shift and scale to 0..1)
        float raw = std::sin(blinkTimer_ * 2.0f * 3.14159265f / 1.2f);
        alpha = (raw + 1.0f) * 0.5f; // 0..1
        // Clamp to avoid out-of-range
        if (alpha < 0.05f) alpha = 0.05f;
        if (alpha > 1.0f) alpha = 1.0f;
        break;
    }
    case Off:
    default:
        color = IM_COL32(0x55, 0x55, 0x55, 0xFF); // gray #555555
        break;
    }

    // Apply alpha to color
    ImU32 finalColor = IM_COL32(
        (int)((color >> IM_COL32_R_SHIFT) & 0xFF),
        (int)((color >> IM_COL32_G_SHIFT) & 0xFF),
        (int)((color >> IM_COL32_B_SHIFT) & 0xFF),
        (int)(alpha * 255.0f)
    );

    float diameter = radius_ * 2.0f;
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 center(cursor.x + radius_, cursor.y + radius_);

    // Use Dummy for layout
    ImGui::Dummy(ImVec2(diameter, diameter));

    // Draw the filled circle
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddCircleFilled(center, radius_, finalColor);

    // Tooltip on hover
    if (!tooltip_.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip_.c_str());
    }

    ImGui::PopID();
}

void StatusLamp::SetState(State s) {
    state_ = s;
    if (s != Draft) {
        blinkTimer_ = 0.0f; // reset blink timer when leaving draft
    }
}

void StatusLamp::SetTooltip(std::string text) {
    tooltip_ = std::move(text);
}

} // namespace unigui
