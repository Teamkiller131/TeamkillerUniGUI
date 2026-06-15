#include <unigui/widgets/buttongroup.h>

#include <imgui.h>

#include <algorithm>
#include <vector>

namespace unigui {

ButtonGroup::ButtonGroup(std::string name)
        : FluentWidget<ButtonGroup>(std::move(name)) {}

void ButtonGroup::Render() {
    if (!IsVisible() || items_.empty())
        return;

    ImGui::PushID(GetName().c_str());

    const ImGuiStyle& style = ImGui::GetStyle();
    const int n = static_cast<int>(items_.size());
    const float avail = ImGui::GetContentRegionAvail().x;

    // Resolve per-button widths.
    std::vector<float> widths(static_cast<std::size_t>(n));
    float totalW = 0.f;
    if (align_ == Align::Fill) {
        const float each = std::max(1.f, (avail - spacing_ * (n - 1)) / static_cast<float>(n));
        for (auto& w : widths)
            w = each;
        totalW = avail;
    } else {
        for (int i = 0; i < n; ++i) {
            widths[static_cast<std::size_t>(i)] =
                buttonWidth_ > 0.f
                    ? buttonWidth_
                    : ImGui::CalcTextSize(items_[static_cast<std::size_t>(i)].label.c_str()).x +
                          style.FramePadding.x * 2.f;
            totalW += widths[static_cast<std::size_t>(i)];
        }
        totalW += spacing_ * (n - 1);
    }

    // Right-align: shift the start cursor to the right edge of the region.
    if (align_ == Align::Right && avail > totalW)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - totalW));

    for (int i = 0; i < n; ++i) {
        const Item& it = items_[static_cast<std::size_t>(i)];
        if (i > 0)
            ImGui::SameLine(0.f, spacing_);

        ImGui::PushID(i);
        int pushed = 0;
        if (it.tinted) {
            const ImVec4 base = theme::GetSemanticColor(it.color);
            ImGui::PushStyleColor(ImGuiCol_Button, base);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(base.x * 1.15f, base.y * 1.15f, base.z * 1.15f, base.w));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(base.x * 0.85f, base.y * 0.85f, base.z * 0.85f, base.w));
            pushed = 3;
        }
        if (!it.enabled)
            ImGui::BeginDisabled();
        const bool pressed = ImGui::Button(it.label.c_str(),
                                           ImVec2(widths[static_cast<std::size_t>(i)], 0.f));
        if (!it.enabled)
            ImGui::EndDisabled();
        if (pushed)
            ImGui::PopStyleColor(pushed);
        ImGui::PopID();

        if (pressed && it.enabled && it.onClick)
            it.onClick();
    }

    ImGui::PopID();
}

} // namespace unigui
