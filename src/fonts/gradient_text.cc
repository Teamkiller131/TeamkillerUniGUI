#include <unigui/fonts/gradient_text.h>

#include <cmath>
#include <cstring>

namespace unigui {

namespace {
// UTF-8 前导字节 → 该码点占用的字节数（续字节/非法前导回退为 1，保证向前推进不死循环）。
// 关键：按“码点”而非“字节”切分——否则多字节汉字被拆成单字节喂给字体，逐字节都成缺字方块（乱码）。
inline int utf8_seq_len(unsigned char c) {
    if (c < 0x80) return 1;             // 0xxxxxxx  ASCII
    if ((c & 0xE0) == 0xC0) return 2;   // 110xxxxx
    if ((c & 0xF0) == 0xE0) return 3;   // 1110xxxx  （汉字多在此区）
    if ((c & 0xF8) == 0xF0) return 4;   // 11110xxx
    return 1;                            // 续字节/非法 → 前进 1
}
} // namespace

void GradientText::Render(const char* text, ImU32 leftColor, ImU32 rightColor) {
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x = pos.x;
    float totalWidth = ImGui::CalcTextSize(text).x;
    const char* end = text + strlen(text);

    // 先数出码点总数，供渐变按“字符位置”插值（中英文一致），而非按字节数。
    int count = 0;
    for (const char* p = text; p < end;) {
        p += utf8_seq_len((unsigned char) *p);
        ++count;
    }

    // Interpolate color channels
    int lr = (leftColor >> 0) & 0xFF, lg = (leftColor >> 8) & 0xFF,
        lb = (leftColor >> 16) & 0xFF, la = (leftColor >> 24) & 0xFF;
    int rr = (rightColor >> 0) & 0xFF, rg = (rightColor >> 8) & 0xFF,
        rb = (rightColor >> 16) & 0xFF, ra = (rightColor >> 24) & 0xFF;

    int idx = 0;
    for (const char* p = text; p < end; ++idx) {
        int n = utf8_seq_len((unsigned char) *p);
        const char* s = p;
        const char* e = (p + n <= end) ? p + n : end;   // 截断保护
        float charW = ImGui::CalcTextSize(s, e).x;
        float t = (count > 1) ? (float) idx / (float) (count - 1) : 0.5f;

        ImU32 col = IM_COL32((int) (lr + (rr - lr) * t), (int) (lg + (rg - lg) * t),
                             (int) (lb + (rb - lb) * t), (int) (la + (ra - la) * t));

        dl->AddText(ImVec2(x, pos.y), col, s, e);   // 传 begin/end：整颗码点一次绘制
        x += charW;
        p = e;
    }

    // Reserve space
    ImGui::Dummy(ImVec2(totalWidth, ImGui::GetTextLineHeight()));
}

void GradientText::RenderHex(const char* text, unsigned lr, unsigned lg, unsigned lb, unsigned rr,
                             unsigned rg, unsigned rb) {
    Render(text, IM_COL32(lr, lg, lb, 255), IM_COL32(rr, rg, rb, 255));
}

} // namespace unigui
