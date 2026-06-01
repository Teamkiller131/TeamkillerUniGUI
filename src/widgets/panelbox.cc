#include <unigui/widgets/panelbox.h>
#include <imgui.h>

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
    ImVec2 textSize = ImGui::CalcTextSize(title_.c_str());
    float textX = cursor.x + 10.f; // left padding
    float textY = cursor.y + (titleBarH - fontSize) * 0.5f;
    dl->AddText(ImVec2(textX, textY), IM_COL32_WHITE, title_.c_str());

    // ── Content area ────────────────────────────────────────────────────
    float contentTop = cursor.y + titleBarH;
    float contentH = ImGui::GetContentRegionAvail().y - titleBarH;

    if (contentH < 0.f) contentH = 0.f;

    ImVec2 contentTopLeft = ImVec2(cursor.x, contentTop);
    ImVec2 contentBotRight = ImVec2(cursor.x + availW, contentTop + contentH);

    // Rounded rect border for content area
    dl->AddRect(contentTopLeft, contentBotRight,
                IM_COL32(80, 80, 90, 255), 4.f, 0, 1.5f);

    // Optional tint overlay
    if (tintColor_ != 0) {
        dl->AddRectFilled(contentTopLeft, contentBotRight, tintColor_);
    }

    // Position ImGui cursor for content callback
    ImGui::SetCursorScreenPos(ImVec2(contentTopLeft.x + 8.f, contentTopLeft.y + 8.f));

    // Call content callback if set
    if (contentCb_) {
        contentCb_();
    }

    // Advance cursor past content area (add spacing)
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, contentBotRight.y + 4.f));

    ImGui::PopID();
}

} // namespace unigui
