#include <unigui/widgets/alertbar.h>

#include <imgui.h>

namespace unigui {

AlertBar::AlertBar(std::string name)
        : Widget(std::move(name)) {}

void AlertBar::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    float dt = ImGui::GetIO().DeltaTime;
    if (shown_)
        animTimer_ = std::min(animTimer_ + dt / 0.2f, 1.0f);
    else
        animTimer_ = std::max(animTimer_ - dt / 0.2f, 0.0f);
    float t = animTimer_ * animTimer_ * (3.0f - 2.0f * animTimer_); // smoothstep
    float animHeight = t * 48.0f;

    if (!shown_ && animHeight < 0.5f) {
        ImGui::PopID();
        return;
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    auto* dl = ImGui::GetWindowDrawList();

    // Top red accent bar
    dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + 3.0f), IM_COL32(0xe9, 0x45, 0x60, 0xff));

    // Background
    ImVec2 bgSize(availW, animHeight);
    dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + animHeight),
                      IM_COL32(0x7f, 0x1d, 0x1d, 0xff));

    // Icon (!!) + message
    float textY = pos.y + (animHeight - ImGui::GetTextLineHeightWithSpacing()) * 0.5f;
    dl->AddText(ImVec2(pos.x + 8.0f, textY), IM_COL32(0xfc, 0xa5, 0xa5, 0xff), "!!");
    dl->AddText(ImVec2(pos.x + 34.0f, textY), IM_COL32(0xfc, 0xa5, 0xa5, 0xff), message_.c_str());

    // Close button (right side)
    ImGui::SetCursorScreenPos(
        ImVec2(pos.x + availW - 40.0f, pos.y + (animHeight - ImGui::GetFrameHeight()) * 0.5f));
    if (ImGui::SmallButton("X"))
        Hide();

    ImGui::Dummy(ImVec2(availW, animHeight));
    ImGui::PopID();
}

void AlertBar::Show(std::string msg) {
    message_ = std::move(msg);
    shown_ = true;
}
void AlertBar::Hide() {
    shown_ = false;
}

} // namespace unigui
