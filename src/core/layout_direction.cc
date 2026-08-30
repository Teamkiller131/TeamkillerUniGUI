#include <unigui/core/layout_direction.h>

namespace unigui {

namespace {
// Process-global by design: the direction is an app-wide presentation choice
// (like the theme), not per-context state.
LayoutDirection g_layoutDirection = LayoutDirection::LeftToRight;
} // namespace

void SetLayoutDirection(LayoutDirection direction) {
    g_layoutDirection = direction;
}

LayoutDirection GetLayoutDirection() {
    return g_layoutDirection;
}

bool IsRightToLeft() {
    return g_layoutDirection == LayoutDirection::RightToLeft;
}

} // namespace unigui
