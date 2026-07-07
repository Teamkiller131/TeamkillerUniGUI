// Internal helper: the UniGUI combo dropdown indicator.
//
// ImGui's default combo arrow is a bulky full-frame-height square button that
// reads as clunky in dense UIs (trading tables especially) and wastes a square
// of frame-height width. Every UniGUI combo — im::Combo / im::BeginCombo, the
// retained ComboBox / MultiCombo / CascadingCombo, and the combo rows in Form,
// PropertyGrid, SettingsPage and OrderTicket — suppresses it
// (ImGuiComboFlags_NoArrowButton) and paints this slim ˅ chevron in the right
// padding instead: tiny, dim at rest, brighter on hover/open.
//
// Usage:
//   const auto frame = detail::CaptureComboFrame();   // BEFORE BeginCombo
//   const bool open  = ImGui::BeginCombo(..., flags | ImGuiComboFlags_NoArrowButton);
//   detail::DrawComboChevron(frame, open || ImGui::IsItemHovered());
//
// The geometry and draw list MUST be captured before BeginCombo: when the
// popup opens, BeginCombo switches the current window to the popup — using the
// captured list keeps the chevron on the preview frame regardless.

#pragma once

#include <imgui.h>

#include <algorithm>

namespace unigui::detail {

struct ComboFrame {
    ImDrawList* draw_list;
    ImVec2 pos;
    float width;
    float height;
};

/// Capture the preview-frame geometry. Call after SetNextItemWidth (if any),
/// immediately before BeginCombo — CalcItemWidth honours the pending width.
inline ComboFrame CaptureComboFrame() {
    return {ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImGui::CalcItemWidth(),
            ImGui::GetFrameHeight()};
}

/// Paint the slim ˅ chevron on the captured frame. `active` brightens it
/// (pass hovered-or-open). Single polyline stroke keeps the vertex joint clean
/// (two AddLine segments leave a notch there at >1px thickness). Theme-aware,
/// DPI-scaled.
inline void DrawComboChevron(const ComboFrame& frame, bool active) {
    const float fs = ImGui::GetFontSize();
    const float halfW = fs * 0.20f;
    const float halfH = fs * 0.11f;
    const float rightPad = ImGui::GetStyle().FramePadding.x + halfW + 1.0f;
    const float cx = frame.pos.x + frame.width - rightPad;
    const float cy = frame.pos.y + frame.height * 0.5f;
    const ImU32 col = ImGui::GetColorU32(ImGuiCol_Text, active ? 0.85f : 0.45f);
    const float thick = std::max(1.0f, fs * 0.075f);
    const ImVec2 pts[3] = {ImVec2(cx - halfW, cy - halfH), ImVec2(cx, cy + halfH),
                           ImVec2(cx + halfW, cy - halfH)};
    frame.draw_list->AddPolyline(pts, 3, col, ImDrawFlags_None, thick);
}

} // namespace unigui::detail
