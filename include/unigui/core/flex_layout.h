#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// flex_layout.h — a CSS-flexbox-style main-axis solver (unigui::layout).
//
// Given a container length and a list of flex items (basis + grow/shrink +
// min/max clamps), SolveFlex resolves each item's main-axis size and position so
// UIs can reflow without hand-computed widths. This is the pure computational
// core of the layout system: NO ImGui, NO allocation beyond the result vector, so
// it is fully unit-testable headlessly. A widget-facing flex container builds on
// top of it.
//
//     using namespace unigui::layout;
//     auto spans = SolveFlex({{.basis = 100, .grow = 1}, {.basis = 100, .grow = 2}},
//                            {.containerSize = 400});
//     // spans[0] = {offset 0,   size 200}   (100 + 1/3 of the 300px free)
//     // spans[1] = {offset 200, size 200}   (100 + 2/3 of the 300px free)
//
// Cross-axis alignment (align-items) and wrapping are layered on in later passes.
// ─────────────────────────────────────────────────────────────────────────────

#include <algorithm>
#include <cfloat>
#include <vector>

namespace unigui::layout {

/// One participant in a flex line. All sizes are pixels along the main axis.
struct FlexItem {
    float basis = 0.0f;      // preferred main-axis size (flex-basis)
    float grow = 0.0f;       // share of leftover space to absorb (flex-grow)
    float shrink = 1.0f;     // share of overflow to give up (flex-shrink)
    float minSize = 0.0f;    // lower clamp on the resolved size
    float maxSize = FLT_MAX; // upper clamp on the resolved size
};

/// Distribution of leftover main-axis space (CSS justify-content).
enum class FlexJustify { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };

struct FlexParams {
    float containerSize = 0.0f;               // available main-axis length
    float gap = 0.0f;                         // fixed gap between adjacent items
    FlexJustify justify = FlexJustify::Start; // leftover-space distribution
};

/// Resolved geometry for one item: main-axis offset + size, both in pixels.
struct FlexSpan {
    float offset = 0.0f;
    float size = 0.0f;
};

namespace detail {
inline float ClampSize(float v, const FlexItem& it) {
    return std::min(std::max(v, it.minSize), it.maxSize);
}
} // namespace detail

/// Resolve a single flex line along the main axis. Returns one FlexSpan per item
/// (in input order). Grow distributes leftover space by grow weight; shrink
/// distributes overflow by the scaled shrink factor (shrink × basis); both honor
/// the per-item min/max clamps via a freeze-and-redistribute pass. justify-content
/// then positions whatever main-axis space remains.
inline std::vector<FlexSpan> SolveFlex(const std::vector<FlexItem>& items,
                                       const FlexParams& params) {
    const int n = static_cast<int>(items.size());
    std::vector<FlexSpan> out(n);
    if (n == 0)
        return out;

    // Seed each item at its (clamped) basis.
    std::vector<float> size(n);
    std::vector<char> frozen(n, 0);
    for (int i = 0; i < n; ++i)
        size[i] = detail::ClampSize(items[i].basis, items[i]);

    const float totalGap = params.gap * static_cast<float>(n - 1);
    auto usedMain = [&] {
        float s = totalGap;
        for (int i = 0; i < n; ++i)
            s += size[i];
        return s;
    };

    float free = params.containerSize - usedMain();

    if (free > 0.0f) {
        // GROW — hand out leftover space by grow weight, freezing items at maxSize
        // and redistributing the remainder among those still able to grow.
        for (int guard = 0; guard <= n && free > 1e-4f; ++guard) {
            float totalGrow = 0.0f;
            for (int i = 0; i < n; ++i)
                if (!frozen[i])
                    totalGrow += items[i].grow;
            if (totalGrow <= 0.0f)
                break;
            float distributed = 0.0f;
            bool froze = false;
            for (int i = 0; i < n; ++i) {
                if (frozen[i] || items[i].grow <= 0.0f)
                    continue;
                const float add = free * (items[i].grow / totalGrow);
                const float target = size[i] + add;
                if (target >= items[i].maxSize) {
                    distributed += items[i].maxSize - size[i];
                    size[i] = items[i].maxSize;
                    frozen[i] = 1;
                    froze = true;
                } else {
                    size[i] = target;
                    distributed += add;
                }
            }
            free -= distributed;
            if (!froze)
                break; // free space fully absorbed by the unfrozen items
        }
    } else if (free < 0.0f) {
        // SHRINK — give up overflow weighted by shrink × basis, freezing at minSize
        // and redistributing the remainder among those still able to shrink.
        float overflow = -free;
        for (int guard = 0; guard <= n && overflow > 1e-4f; ++guard) {
            float totalScaled = 0.0f;
            for (int i = 0; i < n; ++i)
                if (!frozen[i])
                    totalScaled += items[i].shrink * items[i].basis;
            if (totalScaled <= 0.0f)
                break;
            float removed = 0.0f;
            bool froze = false;
            for (int i = 0; i < n; ++i) {
                if (frozen[i])
                    continue;
                const float scaled = items[i].shrink * items[i].basis;
                if (scaled <= 0.0f)
                    continue;
                const float sub = overflow * (scaled / totalScaled);
                const float target = size[i] - sub;
                if (target <= items[i].minSize) {
                    removed += size[i] - items[i].minSize;
                    size[i] = items[i].minSize;
                    frozen[i] = 1;
                    froze = true;
                } else {
                    size[i] = target;
                    removed += sub;
                }
            }
            overflow -= removed;
            if (!froze)
                break;
        }
    }

    // Position items per justify-content over whatever main-axis space is left.
    float leftover = params.containerSize - usedMain();
    if (leftover < 0.0f)
        leftover = 0.0f;
    float cursor = 0.0f;
    float between = params.gap;
    switch (params.justify) {
    case FlexJustify::Start:
        break;
    case FlexJustify::End:
        cursor = leftover;
        break;
    case FlexJustify::Center:
        cursor = leftover * 0.5f;
        break;
    case FlexJustify::SpaceBetween:
        if (n > 1)
            between = params.gap + leftover / static_cast<float>(n - 1);
        break;
    case FlexJustify::SpaceAround:
        between = params.gap + leftover / static_cast<float>(n);
        cursor = leftover / static_cast<float>(n) * 0.5f;
        break;
    case FlexJustify::SpaceEvenly:
        between = params.gap + leftover / static_cast<float>(n + 1);
        cursor = leftover / static_cast<float>(n + 1);
        break;
    }
    for (int i = 0; i < n; ++i) {
        out[i].offset = cursor;
        out[i].size = size[i];
        cursor += size[i] + between;
    }
    return out;
}

} // namespace unigui::layout
