#include <unigui/widgets/skeleton.h>
#include <cmath>

namespace unigui {

void SkeletonScreen::AddBlock(float w, float h, float x, float y) {
    elements_.push_back({Element::Block, x, y, w, h});
}

void SkeletonScreen::AddLine(float w, float x, float y) {
    elements_.push_back({Element::Line, x, y, w, 14.f});
}

void SkeletonScreen::AddCircle(float r, float x, float y) {
    elements_.push_back({Element::Circle, x, y, r, r});
}

SkeletonScreen SkeletonScreen::FromSize(float w, float h, int lineCount) {
    SkeletonScreen s;
    // Header block
    s.AddBlock(w * 0.4f, 20.f, 0, 0);
    // Lines
    for (int i = 0; i < lineCount; ++i) {
        float y = 28.f + (float)i * 20.f;
        float lw = (i == lineCount - 1) ? w * 0.6f : w;
        s.AddLine(lw, 0, y);
    }
    // Avatar + content row
    s.AddCircle(32.f, 0, 28.f + lineCount * 20.f);
    s.AddBlock(w * 0.3f, 14.f, 40.f, 28.f + lineCount * 20.f);
    s.AddBlock(w * 0.5f, 14.f, 40.f, 48.f + lineCount * 20.f);
    return s;
}

void SkeletonScreen::Render() {
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    float maxY = 0.f;
    for (auto& e : elements_) {
        ImVec2 p0(cursor.x + e.x, cursor.y + e.y);
        ImVec2 p1(p0.x + e.w, p0.y + e.h);

        if (e.kind == Element::Circle) {
            dl->AddCircleFilled(ImVec2(p0.x + e.w * 0.5f, p0.y + e.h * 0.5f), e.w, kColor);
        } else {
            float rr = (e.kind == Element::Line) ? e.h * 0.5f : kRounding;
            dl->AddRectFilled(p0, p1, kColor, rr);
        }
        maxY = std::max(maxY, e.y + e.h);
    }

    if (!elements_.empty()) ImGui::Dummy(ImVec2(0, maxY + 4));
}

} // namespace unigui
