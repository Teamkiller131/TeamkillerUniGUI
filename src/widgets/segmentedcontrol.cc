#include <unigui/widgets/segmentedcontrol.h>

#include <imgui.h>

#include <algorithm>

namespace unigui {

SegmentedControl::SegmentedControl(std::string name, std::vector<std::string> segments)
        : FluentWidget<SegmentedControl>(std::move(name))
        , segments_(std::move(segments)) {}

void SegmentedControl::SetSegments(std::vector<std::string> segments) {
    segments_ = std::move(segments);
    if (selected_ >= static_cast<int>(segments_.size()))
        selected_ = segments_.empty() ? 0 : static_cast<int>(segments_.size()) - 1;
}

void SegmentedControl::AddSegment(std::string label) {
    segments_.push_back(std::move(label));
}

void SegmentedControl::Clear() {
    segments_.clear();
    selected_ = 0;
}

void SegmentedControl::SetSelected(int index) {
    if (index >= 0 && index < static_cast<int>(segments_.size()))
        selected_ = index;
}

std::string SegmentedControl::GetSelectedLabel() const {
    if (selected_ >= 0 && selected_ < static_cast<int>(segments_.size()))
        return segments_[selected_];
    return "";
}

void SegmentedControl::Render() {
    if (!IsVisible())
        return;
    if (segments_.empty())
        return;

    ImGui::PushID(GetName().c_str());

    const ImGuiStyle& style = ImGui::GetStyle();
    const float h = ImGui::GetFrameHeight();
    const ImVec2 origin = ImGui::GetCursorScreenPos();

    // Resolve per-segment widths.
    const std::size_t n = segments_.size();
    std::vector<float> widths(n);
    float totalW = 0.f;
    if (fillWidth_) {
        const float avail = ImGui::GetContentRegionAvail().x;
        const float each = avail / static_cast<float>(n);
        for (auto& w : widths)
            w = each;
        totalW = avail;
    } else {
        for (std::size_t i = 0; i < n; ++i) {
            widths[i] = ImGui::CalcTextSize(segments_[i].c_str()).x + segPad_ * 2.f;
            totalW += widths[i];
        }
    }

    const float rounding = style.FrameRounding;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Outer frame background.
    dl->AddRectFilled(origin, ImVec2(origin.x + totalW, origin.y + h),
                      ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);

    const ImU32 accent = ImGui::GetColorU32(ImGuiCol_ButtonActive);
    const ImU32 textOn = ImGui::GetColorU32(ImGuiCol_Text);
    const ImU32 textOff = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    const ImU32 hoverBg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);

    const bool enabled = IsEnabled();
    float x = origin.x;
    int clicked = -1;
    for (std::size_t i = 0; i < n; ++i) {
        const float w = widths[i];
        const ImVec2 segMin(x, origin.y);
        const ImVec2 segMax(x + w, origin.y + h);

        ImGui::PushID(static_cast<int>(i));
        ImGui::SetCursorScreenPos(segMin);
        const bool pressed = ImGui::InvisibleButton("##seg", ImVec2(w, h)) && enabled;
        const bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();

        const bool isSel = (static_cast<int>(i) == selected_);
        // Per-segment rounding: only the outer corners of the first/last segment
        // are rounded so the group reads as one pill.
        ImDrawFlags corners = ImDrawFlags_RoundCornersNone;
        if (i == 0)
            corners |= ImDrawFlags_RoundCornersLeft;
        if (i == n - 1)
            corners |= ImDrawFlags_RoundCornersRight;

        if (isSel)
            dl->AddRectFilled(segMin, segMax, accent, rounding, corners);
        else if (hovered && enabled)
            dl->AddRectFilled(segMin, segMax, hoverBg, rounding, corners);

        const ImVec2 ts = ImGui::CalcTextSize(segments_[i].c_str());
        dl->AddText(ImVec2(x + (w - ts.x) * 0.5f, origin.y + (h - ts.y) * 0.5f),
                    isSel ? textOn : textOff, segments_[i].c_str());

        // Divider between unselected neighbours.
        if (i > 0 && !isSel && static_cast<int>(i) - 1 != selected_)
            dl->AddLine(ImVec2(x, origin.y + h * 0.2f), ImVec2(x, origin.y + h * 0.8f),
                        ImGui::GetColorU32(ImGuiCol_Separator), 1.f);

        if (pressed)
            clicked = static_cast<int>(i);
        x += w;
    }

    // Advance the ImGui cursor past the control so siblings flow normally.
    ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y));
    ImGui::Dummy(ImVec2(totalW, h));

    if (clicked >= 0 && clicked != selected_) {
        selected_ = clicked;
        if (onChange_)
            onChange_(selected_, segments_[selected_]);
    }

    RenderTooltip();
    ImGui::PopID();
}

} // namespace unigui
