#pragma once
#include <imgui.h>
#include <vector>
#include <unigui/fx/animation.h>

namespace unigui {

/// SkeletonScreen — static loading placeholder (blocks + circles + lines)
/// Use with Shimmer for animated loading UIs.
class SkeletonScreen {
public:
    SkeletonScreen() = default;

    /// Add a rectangular block (e.g., card, image placeholder)
    void AddBlock(float width, float height, float x = 0.f, float y = 0.f);

    /// Add a text line placeholder
    void AddLine(float width, float x = 0.f, float y = 0.f);

    /// Add a circle placeholder (e.g., avatar)
    void AddCircle(float radius, float x = 0.f, float y = 0.f);

    /// Auto-generate skeleton from a child region size
    static SkeletonScreen FromSize(float w, float h, int lineCount = 4);

    /// Render all skeleton elements
    void Render();

private:
    struct Element {
        enum Kind { Block, Line, Circle } kind = Block;
        float x = 0, y = 0, w = 0, h = 0;
    };
    std::vector<Element> elements_;
    static constexpr ImU32 kColor = IM_COL32(60, 60, 70, 120);
    static constexpr float kRounding = 4.f;
};

} // namespace unigui
