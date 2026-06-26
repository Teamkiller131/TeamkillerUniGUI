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
// Cross-axis placement (align-items) is supported via FlexParams::align +
// FlexItem::crossSize; line wrapping (flex-wrap) is layered on by SolveFlexWrap,
// which greedily breaks items into lines and stacks them on the cross axis.
// ─────────────────────────────────────────────────────────────────────────────

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <vector>

namespace unigui::layout {

/// One participant in a flex line. All sizes are pixels.
struct FlexItem {
    float basis = 0.0f;      // preferred main-axis size (flex-basis)
    float grow = 0.0f;       // share of leftover space to absorb (flex-grow)
    float shrink = 1.0f;     // share of overflow to give up (flex-shrink)
    float minSize = 0.0f;    // lower clamp on the resolved main-axis size
    float maxSize = FLT_MAX; // upper clamp on the resolved main-axis size
    float crossSize = 0.0f;  // preferred cross-axis size (used by align != Stretch)
};

/// Distribution of leftover main-axis space (CSS justify-content).
enum class FlexJustify { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };

/// Cross-axis placement of each item within the line (CSS align-items).
enum class FlexAlign { Start, Center, End, Stretch };

struct FlexParams {
    float containerSize = 0.0f;               // available main-axis length
    float crossSize = 0.0f;                   // available cross-axis length (0 = unknown)
    float gap = 0.0f;                         // fixed gap between adjacent items
    FlexJustify justify = FlexJustify::Start; // leftover main-axis distribution
    FlexAlign align = FlexAlign::Start;       // cross-axis placement
};

/// Resolved geometry for one item. Main-axis offset+size always; cross-axis
/// offset+size are filled per the align mode (cross fields stay 0 when no
/// container crossSize / crossSize is given).
struct FlexSpan {
    float offset = 0.0f;
    float size = 0.0f;
    float crossOffset = 0.0f;
    float crossSize = 0.0f;
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

        // Cross-axis placement (align-items).
        const float cs = items[i].crossSize;
        switch (params.align) {
        case FlexAlign::Stretch:
            out[i].crossSize = params.crossSize > 0.0f ? params.crossSize : cs;
            out[i].crossOffset = 0.0f;
            break;
        case FlexAlign::Start:
            out[i].crossSize = cs;
            out[i].crossOffset = 0.0f;
            break;
        case FlexAlign::Center:
            out[i].crossSize = cs;
            out[i].crossOffset = std::max(0.0f, (params.crossSize - cs) * 0.5f);
            break;
        case FlexAlign::End:
            out[i].crossSize = cs;
            out[i].crossOffset = std::max(0.0f, params.crossSize - cs);
            break;
        }
    }
    return out;
}

/// Break items into lines along the main axis (CSS flex-wrap: wrap), then resolve
/// each line independently with SolveFlex and stack the lines on the cross axis.
///
/// Greedy line-breaking: items are walked in order, accumulating their basis plus
/// the inter-item gap; when adding the next item would push the running main-axis
/// total past params.containerSize, the next item starts a fresh line. An item
/// whose own basis already exceeds containerSize occupies a line by itself. Each
/// line's sub-vector is then handed to SolveFlex with the same containerSize / gap
/// / justify, so grow/shrink/justify are fully resolved within that line only
/// (space never flows across the wrap boundary, matching CSS).
///
/// Lines stack top-to-bottom on the cross axis: each line's crossOffset is the
/// sum of the preceding lines' effective heights. The effective line height is the
/// caller-supplied `lineHeight` when it is > 0; otherwise it falls back to the
/// tallest FlexItem::crossSize among that line's own items (so naturally-sized
/// lines pack against their own content — CSS align-content with auto-sized
/// lines). A single uniform `lineHeight` is the common case (fixed row height);
/// the per-line max is the sensible default when rows size to content.
///
/// Returns one inner vector per line (item order preserved within each line),
/// outer vector ordered top-to-bottom. Empty input yields an empty outer vector.
inline std::vector<std::vector<FlexSpan>> SolveFlexWrap(const std::vector<FlexItem>& items,
                                                        const FlexParams& params,
                                                        float lineHeight = 0.0f) {
    std::vector<std::vector<FlexSpan>> lines;
    const int n = static_cast<int>(items.size());
    if (n == 0)
        return lines;

    // 1) Greedily partition items into lines along the main axis.
    std::vector<std::vector<FlexItem>> lineItems;
    {
        std::vector<FlexItem> current;
        float used = 0.0f; // running basis + gaps of the current line
        for (int i = 0; i < n; ++i) {
            const float add = items[i].basis;
            const float gap = current.empty() ? 0.0f : params.gap;
            if (!current.empty() && used + gap + add > params.containerSize) {
                lineItems.push_back(std::move(current));
                current.clear();
                used = 0.0f;
            }
            const float leadGap = current.empty() ? 0.0f : params.gap;
            used += leadGap + add;
            current.push_back(items[i]);
        }
        if (!current.empty())
            lineItems.push_back(std::move(current));
    }

    // 2) Solve each line independently and stack it on the cross axis. crossCursor
    // accumulates the heights of the lines above, so lines of differing heights
    // (auto line-height) stack correctly (l * thisLineHeight would be wrong).
    lines.reserve(lineItems.size());
    float crossCursor = 0.0f;
    for (std::size_t l = 0; l < lineItems.size(); ++l) {
        std::vector<FlexSpan> spans = SolveFlex(lineItems[l], params);

        // Effective line height: explicit override when given, else the tallest
        // item crossSize on this line (auto-sized rows pack against their content).
        float effLineHeight = lineHeight;
        if (effLineHeight <= 0.0f) {
            for (const FlexItem& it : lineItems[l])
                effLineHeight = std::max(effLineHeight, it.crossSize);
        }

        for (FlexSpan& s : spans)
            s.crossOffset += crossCursor;
        lines.push_back(std::move(spans));
        crossCursor += effLineHeight; // next line stacks below this one
    }
    return lines;
}

} // namespace unigui::layout
