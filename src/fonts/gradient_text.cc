#include <unigui/fonts/gradient_text.h>
#include <cmath>

namespace unigui {

void GradientText::Render(const char* text, ImU32 leftColor, ImU32 rightColor) {
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x = pos.x;
    float totalWidth = ImGui::CalcTextSize(text).x;
    int len = (int)strlen(text);

    for (int i = 0; i < len; ++i) {
        char ch[2] = {text[i], 0};
        float charW = ImGui::CalcTextSize(ch).x;
        float t = (len > 1) ? (float)i / (float)(len - 1) : 0.5f;

        // Interpolate color channels
        int lr = (leftColor >> 0) & 0xFF, lg = (leftColor >> 8) & 0xFF, lb = (leftColor >> 16) & 0xFF, la = (leftColor >> 24) & 0xFF;
        int rr = (rightColor >> 0) & 0xFF, rg = (rightColor >> 8) & 0xFF, rb = (rightColor >> 16) & 0xFF, ra = (rightColor >> 24) & 0xFF;

        ImU32 col = IM_COL32(
            (int)(lr + (rr - lr) * t),
            (int)(lg + (rg - lg) * t),
            (int)(lb + (rb - lb) * t),
            (int)(la + (ra - la) * t));

        dl->AddText(ImVec2(x, pos.y), col, ch);
        x += charW;
    }

    // Reserve space
    ImGui::Dummy(ImVec2(totalWidth, ImGui::GetTextLineHeight()));
}

void GradientText::RenderHex(const char* text,
                              unsigned lr, unsigned lg, unsigned lb,
                              unsigned rr, unsigned rg, unsigned rb) {
    Render(text, IM_COL32(lr, lg, lb, 255), IM_COL32(rr, rg, rb, 255));
}

} // namespace unigui
