#pragma once
#include <unigui/fx/animation.h>

#include <imgui.h>

#include <vector>

namespace unigui {

/// SkeletonScreen — loading placeholder (blocks + circles + lines).
/// Optional built-in shimmer animation via SetShimmer(true).
class SkeletonScreen {
public:
    SkeletonScreen() = default;

    void AddBlock(float width, float height, float x = 0.f, float y = 0.f);
    void AddLine(float width, float x = 0.f, float y = 0.f);
    void AddCircle(float radius, float x = 0.f, float y = 0.f);

    /// Enable built-in shimmer animation (gradient sweep over skeleton).
    void SetShimmer(bool enable, float speed = 1.2f);

    static SkeletonScreen FromSize(float w, float h, int lineCount = 4);

    void Render();

private:
    struct Element {
        enum Kind { Block, Line, Circle } kind = Block;
        float x = 0, y = 0, w = 0, h = 0;
    };
    std::vector<Element> elements_;
    static constexpr ImU32 kColor = IM_COL32(60, 60, 70, 120);
    static constexpr float kRounding = 4.f;

    bool shimmer_ = false;
    fx::AnimationState shimmerAnim_;
    float shimmerSpeed_ = 1.2f;
};

} // namespace unigui
