#include <unigui/widgets/badge.h>

namespace unigui {

Badge::Badge(const std::string& label) : text_(label) {}
void Badge::SetText(const std::string& t)  { text_ = t; }
void Badge::SetVariant(Variant v)           { variant_ = v; }
void Badge::SetColor(ImU32 c)              { color_ = c; }
void Badge::SetCount(int n)                { count_ = n; variant_ = Count; }

void Badge::Render() {
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    float size;
    std::string display;

    switch (variant_) {
    case Dot:
        size = 8.f;
        dl->AddCircleFilled(ImVec2(cursor.x + 4, cursor.y + 4), 4.f, color_);
        ImGui::Dummy(ImVec2(8, 8));
        break;
    case Count:
        display = std::to_string(count_);
        size = 18.f;
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
        size = 16.f;
        {
            ImVec2 textSize = ImGui::CalcTextSize(display.c_str());
            float w = textSize.x + 8.f;
            ImVec2 p0(cursor.x, cursor.y);
            ImVec2 p1(p0.x + w, p0.y + size);
            dl->AddRectFilled(p0, p1, color_, size * 0.5f);
            dl->AddText(ImVec2(p0.x + 4.f, p0.y + 1.f),
                        IM_COL32_WHITE, display.c_str());
            ImGui::Dummy(ImVec2(w, size));
        }
        break;
    }
}

} // namespace unigui
