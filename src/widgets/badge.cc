#include <unigui/widgets/badge.h>

namespace unigui {

Badge::Badge(const std::string& label)
        : text_(label) {}
void Badge::SetText(const std::string& t) {
    text_ = t;
}
void Badge::SetVariant(Variant v) {
    variant_ = v;
}
void Badge::SetColor(ImU32 c) {
    color_ = c;
}
void Badge::SetCount(int n) {
    count_ = n;
    variant_ = Count;
}

void Badge::Render() {
    ImGui::PushID(this);
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    float size;
    std::string display;

    switch (variant_) {
    case Dot:
        // W6-type: dot diameter tracks the font so it stays proportional at 26 px.
        size = std::max(8.f, ImGui::GetTextLineHeight() * 0.3f);
        dl->AddCircleFilled(ImVec2(cursor.x + size * 0.5f, cursor.y + size * 0.5f),
                            size * 0.5f, color_);
        ImGui::Dummy(ImVec2(size, size));
        break;
    case Count:
        display = std::to_string(count_);
        size = std::max(18.f, ImGui::GetTextLineHeight() + 6.f);
        {
            ImVec2 textSize = ImGui::CalcTextSize(display.c_str());
            float w = std::max(size, textSize.x + 8.f);
            ImVec2 p0(cursor.x, cursor.y);
            ImVec2 p1(p0.x + w, p0.y + size);
            dl->AddRectFilled(p0, p1, color_, size * 0.5f);
            dl->AddText(ImVec2(p0.x + (w - textSize.x) * 0.5f, p0.y + (size - textSize.y) * 0.5f),
                        IM_COL32_WHITE, display.c_str());
            ImGui::Dummy(ImVec2(w, size));
        }
        break;
    case Label:
        display = text_;
        size = std::max(16.f, ImGui::GetTextLineHeight() + 4.f);
        {
            ImVec2 textSize = ImGui::CalcTextSize(display.c_str());
            float w = textSize.x + 8.f;
            ImVec2 p0(cursor.x, cursor.y);
            ImVec2 p1(p0.x + w, p0.y + size);
            dl->AddRectFilled(p0, p1, color_, size * 0.5f);
            dl->AddText(ImVec2(p0.x + 4.f, p0.y + 1.f), IM_COL32_WHITE, display.c_str());
            ImGui::Dummy(ImVec2(w, size));
        }
        break;
    }
    ImGui::PopID();
}

} // namespace unigui
