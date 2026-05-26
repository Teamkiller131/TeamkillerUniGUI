#include <unigui/widgets/shimmer.h>
#include <unigui/fx/effect_scope.h>
#include <cmath>

namespace unigui {

Shimmer::Shimmer() {
    anim_.loop = true;
    anim_.duration = 1.2f / speed_;
}

void Shimmer::AddBlock(float w, float h, float x, float y) {
    elements_.push_back({Element::Block, x, y, w, h});
}

void Shimmer::AddCircle(float r, float x, float y) {
    elements_.push_back({Element::Circle, x, y, r, r});
}

void Shimmer::Start() {
    if (!playing_) anim_.Play(1.2f / speed_, fx::EasingCurve::Linear);
    playing_ = true;
}

void Shimmer::Stop() {
    anim_.Stop();
    playing_ = false;
}

bool Shimmer::IsPlaying() const { return playing_; }

void Shimmer::SetSpeed(float s) {
    speed_ = s;
    anim_.duration = 1.2f / s;
}

void Shimmer::Render() {
    if (!playing_) return;

    float t = anim_.Update(ImGui::GetIO().DeltaTime);
    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    // Sweep gradient position: [−0.3 .. 1.3]
    float sweep = -0.3f + t * 1.6f;

    for (auto& e : elements_) {
        ImVec2 p0(cursor.x + e.x, cursor.y + e.y);
        ImVec2 p1(p0.x + e.w, p0.y + e.h);

        // Base placeholder (dim grey)
        ImU32 baseCol = IM_COL32(60, 60, 70, 100);
        ImU32 highlightCol = IM_COL32(140, 140, 160, 60);
        float rr = std::min(e.w, e.h) * 0.15f;

        if (e.kind == Element::Block) {
            dl->AddRectFilled(p0, p1, baseCol, rr);

            // Gradient sweep band
            float bandLeft  = p0.x + e.w * (sweep - 0.15f);
            float bandRight = p0.x + e.w * (sweep + 0.15f);
            bandLeft  = std::max(p0.x, std::min(p1.x, bandLeft));
            bandRight = std::max(p0.x, std::min(p1.x, bandRight));

            if (bandRight > bandLeft) {
                dl->AddRectFilledMultiColor(
                    ImVec2(bandLeft, p0.y), ImVec2(bandRight, p1.y),
                    highlightCol, highlightCol,
                    IM_COL32(60, 60, 70, 100), IM_COL32(60, 60, 70, 100));
            }
        } else {
            // Circle
            dl->AddCircleFilled(
                ImVec2(p0.x + e.w * 0.5f, p0.y + e.h * 0.5f),
                e.w, baseCol);

            // Highlight arc
            float cx = p0.x + e.w * 0.5f;
            float sweepX = cx + e.w * (sweep - 0.5f) * 0.8f;
            dl->AddCircleFilled(ImVec2(sweepX, p0.y + e.h * 0.5f),
                               e.w * 0.6f, highlightCol);
        }
    }

    // Reserve space for rendered elements
    if (!elements_.empty()) {
        float maxY = 0.f;
        for (auto& e : elements_) maxY = std::max(maxY, e.y + e.h);
        ImGui::Dummy(ImVec2(0, maxY));
    }
}

} // namespace unigui
