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

void SkeletonScreen::SetShimmer(bool enable, float speed) {
    shimmer_ = enable;
    if (enable) {
        shimmerSpeed_ = speed;
        shimmerAnim_.loop = true;
        shimmerAnim_.duration = 1.2f / speed;
        shimmerAnim_.Play(1.2f / speed, fx::EasingCurve::Linear);
    }
}

SkeletonScreen SkeletonScreen::FromSize(float w, [[maybe_unused]] float h, int lineCount) {
    SkeletonScreen s;
    s.AddBlock(w * 0.4f, 20.f, 0, 0);
    for (int i = 0; i < lineCount; ++i) {
        float lw = (i == lineCount - 1) ? w * 0.6f : w;
        s.AddLine(lw, 0, 28.f + (float) i * 20.f);
    }
    s.AddCircle(32.f, 0, 28.f + lineCount * 20.f);
    s.AddBlock(w * 0.3f, 14.f, 40.f, 28.f + lineCount * 20.f);
    s.AddBlock(w * 0.5f, 14.f, 40.f, 48.f + lineCount * 20.f);
    return s;
}

void SkeletonScreen::Render() {
    ImGui::PushID(this);
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    float t = shimmer_ ? shimmerAnim_.Update(ImGui::GetIO().DeltaTime) : 0.f;
    ImU32 baseCol = kColor;
    ImU32 hiCol = IM_COL32(140, 140, 160, 60);

    float maxY = 0.f;
    for (auto& e : elements_) {
        ImVec2 p0(cursor.x + e.x, cursor.y + e.y);
        ImVec2 p1(p0.x + e.w, p0.y + e.h);

        if (e.kind == Element::Circle) {
            dl->AddCircleFilled(ImVec2(p0.x + e.w * 0.5f, p0.y + e.h * 0.5f), e.w, baseCol);
            if (shimmer_) {
                // Highlight arc sweeps across circle
                float sweepX = p0.x + e.w * (t * 2.f - 0.7f);
                sweepX = std::max(p0.x, std::min(p1.x, sweepX));
                dl->AddCircleFilled(ImVec2(sweepX, p0.y + e.h * 0.5f), e.w * 0.5f, hiCol);
            }
        } else {
            float rr = (e.kind == Element::Line) ? e.h * 0.5f : kRounding;
            dl->AddRectFilled(p0, p1, baseCol, rr);
            if (shimmer_) {
                // Gradient highlight band sweeps left→right
                float bandW = e.w * 0.3f;
                float bandX = p0.x + e.w * (t * 1.6f - 0.3f);
                bandX = std::max(p0.x, std::min(p1.x - bandW, bandX));
                dl->AddRectFilled(ImVec2(bandX, p0.y), ImVec2(bandX + bandW, p1.y), hiCol, rr);
            }
        }
        maxY = std::max(maxY, e.y + e.h);
    }

    if (!elements_.empty())
        ImGui::Dummy(ImVec2(0, maxY + 4));
    ImGui::PopID();
}

} // namespace unigui
