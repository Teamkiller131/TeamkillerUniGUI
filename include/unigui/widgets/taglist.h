#pragma once

#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <string>
#include <vector>

namespace unigui {

/// One chip in a TagList: a short label coloured by a semantic role or an
/// explicit RGBA. (Named `TagItem` to avoid colliding with the retained `Tag`
/// widget.)
struct TagItem {
    std::string text;
    theme::Semantic role = theme::Semantic::Accent; ///< used when `color == 0`
    ImU32 color = 0;                                 ///< explicit RGBA; 0 = use `role`
};

/// Render 0..N coloured chips packed inline on the current line, wrapping to the
/// next line when they exceed `wrapWidth` (0 = the available content width).
/// Replaces the hand-rolled `SameLine` + per-tag `PushStyleColor` idiom used for
/// limit-up/down / 科创 / 可融 / order-state flags. Immediate-mode; the caller
/// computes the tags, the widget only packs and colours them.
void TagList(const std::vector<TagItem>& tags, float wrapWidth = 0.f);

} // namespace unigui
