#include <unigui/widgets/taglist.h>

#include <imgui.h>

namespace unigui {

void TagList(const std::vector<TagItem>& tags, float wrapWidth) {
    if (tags.empty())
        return;

    const ImVec2 pad(6.f, 2.f);   // inner chip padding
    const float gap = 6.f;        // gap between chips
    const float rounding = 4.f;

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float avail = wrapWidth > 0.f ? wrapWidth : ImGui::GetContentRegionAvail().x;
    const float right = origin.x + (avail > 0.f ? avail : 1e6f);
    const float lineH = ImGui::GetTextLineHeight() + pad.y * 2.f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float x = origin.x;
    float y = origin.y;
    float maxX = origin.x;

    for (const auto& t : tags) {
        const ImVec2 ts = ImGui::CalcTextSize(t.text.c_str());
        const float chipW = ts.x + pad.x * 2.f;

        // Wrap to a new line if this chip would overflow (but always place at
        // least one chip per line).
        if (x > origin.x && x + chipW > right) {
            x = origin.x;
            y += lineH + 2.f;
        }

        const ImU32 base = t.color != 0 ? t.color
                                        : ImGui::GetColorU32(theme::GetSemanticColor(t.role));
        ImVec4 bgv = ImGui::ColorConvertU32ToFloat4(base);
        bgv.w = 0.18f; // translucent fill
        const ImU32 bg = ImGui::ColorConvertFloat4ToU32(bgv);

        const ImVec2 p0(x, y);
        const ImVec2 p1(x + chipW, y + lineH);
        dl->AddRectFilled(p0, p1, bg, rounding);
        dl->AddText(ImVec2(x + pad.x, y + pad.y), base, t.text.c_str());

        x += chipW + gap;
        if (x > maxX)
            maxX = x;
    }

    // Reserve the laid-out bounds so following widgets flow correctly.
    const float totalW = maxX - origin.x;
    const float totalH = (y + lineH) - origin.y;
    ImGui::Dummy(ImVec2(totalW, totalH));
}

} // namespace unigui
