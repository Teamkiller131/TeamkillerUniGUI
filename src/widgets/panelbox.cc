#include <unigui/widgets/panelbox.h>
#include <imgui.h>
#include <algorithm>

namespace unigui {

PanelBox::PanelBox(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {
}

void PanelBox::Render() {
    if (!IsVisible()) return;

    ImGui::PushID(GetName().c_str());

    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;

    // ── Title bar (dark bg #1e1e24, 28px tall) ──────────────────────────
    const float titleBarH = 28.f;
    ImVec2 titleBarTopLeft = cursor;
    ImVec2 titleBarBotRight = ImVec2(cursor.x + availW, cursor.y + titleBarH);

    dl->AddRectFilled(titleBarTopLeft, titleBarBotRight,
                      IM_COL32(0x1e, 0x1e, 0x24, 255));

    // Bottom border line on title bar
    dl->AddLine(ImVec2(titleBarTopLeft.x, titleBarBotRight.y),
                ImVec2(titleBarBotRight.x, titleBarBotRight.y),
                IM_COL32(60, 60, 70, 255), 1.f);

    // Title text (bold white, centered vertically in title bar)
    float fontSize = ImGui::GetFontSize();
    float textX = cursor.x + 10.f; // left padding
    float textY = cursor.y + (titleBarH - fontSize) * 0.5f;
    dl->AddText(ImVec2(textX, textY), IM_COL32_WHITE, title_.c_str());

    // ── Content area ────────────────────────────────────────────────────
    float contentTop = cursor.y + titleBarH;
    float contentH = ImGui::GetContentRegionAvail().y - titleBarH;

    if (contentH < 0.f) contentH = 0.f;

    ImGui::SetCursorScreenPos(ImVec2(cursor.x, contentTop));

    const std::string bodyId = GetName() + "_body";
    ImGuiChildFlags childFlags = ImGuiChildFlags_Borders;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollWithMouse;
    if (shrinkWrapContent_) {
        childFlags |= ImGuiChildFlags_AutoResizeY;
        windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
        ImGui::SetNextWindowSizeConstraints(ImVec2(availW, 0.f), ImVec2(availW, contentH));
    }

    ImGui::BeginChild(bodyId.c_str(), ImVec2(availW, shrinkWrapContent_ ? 0.f : contentH),
                      childFlags, windowFlags);
    if (contentCb_) {
        contentCb_();
    }
    ImGui::EndChild();

    const float bodyH = shrinkWrapContent_
                            ? std::min(contentH, ImGui::GetItemRectSize().y)
                            : contentH;

    // Claim the splitter slot without painting a second manual border.
    ImGui::SetCursorScreenPos(cursor);
    ImGui::Dummy(ImVec2(availW, titleBarH + bodyH));

    ImGui::PopID();
}

} // namespace unigui
