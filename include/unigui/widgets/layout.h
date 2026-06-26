#pragma once
#include <unigui/core/flex_layout.h>

#include <imgui.h>

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// Simple declarative layout helpers. (Note: the *pure* flex math lives in the
/// separate `unigui::layout` namespace — `core/flex_layout.h`; this `Layout`
/// namespace holds the ImGui-facing helpers that apply it.)
namespace Layout {

/// Horizontal box: render children side by side.
inline void HBox(std::initializer_list<std::function<void()>> children) {
    for (auto& c : children) {
        c();
        if (&c != children.end() - 1)
            ImGui::SameLine();
    }
}

inline void VBox(std::initializer_list<std::function<void()>> children) {
    for (auto& c : children) {
        c();
    }
}

/// Begin a horizontal group. Call EndHBox() after rendering children.
inline void BeginHBox() {
    ImGui::BeginGroup();
}
inline void EndHBox() {
    ImGui::EndGroup();
}

/// Begin a vertical split with ratio. ratio=0.5 means 50/50.
inline void BeginHSplit(float leftRatio = 0.5f) {
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::BeginChild("##left", ImVec2(avail * leftRatio, 0), ImGuiChildFlags_Borders);
}
inline void NextHSplit() {
    ImGui::EndChild();
    ImGui::SameLine();
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::BeginChild("##right", ImVec2(avail, 0), ImGuiChildFlags_Borders);
}
inline void EndHSplit() {
    ImGui::EndChild();
}

/// One child of a FlexRow: its flex sizing (basis / grow / shrink / min / max,
/// from `unigui::layout::FlexItem`) plus a render callback drawn inside a child
/// region sized to the resolved flex width.
struct FlexChild {
    layout::FlexItem item;
    std::function<void()> render;
};

/// Options for FlexRow (all optional; designated-initializer friendly, e.g.
/// `FlexRow("toolbar", kids, {.gap = 8.f, .justify = FlexJustify::SpaceBetween})`).
struct FlexRowOptions {
    float width = 0.0f;  ///< main-axis length; <=0 uses the available content width
    float height = 0.0f; ///< child height; <=0 fills the remaining vertical space
    float gap = 0.0f;    ///< fixed gap between adjacent children
    layout::FlexJustify justify = layout::FlexJustify::Start;
    /// Cross-axis (vertical) alignment of children within the row's height (CSS
    /// align-items). Effective only when children set `FlexChild::item.crossSize`
    /// (Start/Center/End) or `opt.height>0` (Stretch); the default Start with no
    /// per-child crossSize preserves the legacy uniform-height behavior.
    layout::FlexAlign align = layout::FlexAlign::Start;
};

/// Lay children out in a horizontal flex line, sizing each via the flexbox solver
/// and drawing it inside an ImGui child region of its resolved width. `id` scopes
/// the per-child region IDs (ID safety — see CLAUDE.md), so multiple FlexRows can
/// coexist in one window.
///
/// `opt.width<=0` uses the available content width; if there is no horizontal
/// room the row renders nothing. `opt.height<=0` makes every child fill the
/// remaining vertical space — in that mode a FlexRow must be the LAST element in
/// its container (it forfeits the cursor flow for anything after it); pass a
/// positive height to lay further content out below the row.
///
/// Cross-axis alignment (`opt.align`, CSS align-items) positions each child
/// vertically within the row. The row's cross-axis extent is `opt.height` (or the
/// remaining content height when `opt.height<=0`). A child's effective height is
/// its solved cross size when it sets `FlexChild::item.crossSize` (or under
/// Stretch), otherwise it falls back to `opt.height`. The default (align=Start,
/// no per-child crossSize) keeps the legacy uniform-height layout unchanged.
inline void FlexRow(const char* id, const std::vector<FlexChild>& children,
                    const FlexRowOptions& opt = {}) {
    if (children.empty())
        return;

    // Sanitize inputs (ternaries also reject NaN, since NaN > 0 is false).
    const float gap = opt.gap > 0.0f ? opt.gap : 0.0f;
    const float height = opt.height > 0.0f ? opt.height : 0.0f;
    const float container = opt.width > 0.0f ? opt.width : ImGui::GetContentRegionAvail().x;
    if (!(container > 0.0f))
        return; // no horizontal room (or NaN) — nothing sensible to lay out

    std::vector<layout::FlexItem> items;
    items.reserve(children.size());
    for (const auto& c : children)
        items.push_back(c.item);

    // Cross-axis extent: explicit height, else the remaining content height. This
    // feeds the solver so Center/End/Stretch have a box to align within. (A
    // non-positive avail is harmless: crossOffsets clamp to 0 and Stretch falls
    // back to the per-child crossSize.)
    const float crossSize = height > 0.0f ? height : ImGui::GetContentRegionAvail().y;

    layout::FlexParams params;
    params.containerSize = container;
    params.crossSize = crossSize;
    params.gap = gap;
    params.justify = opt.justify;
    params.align = opt.align;
    const std::vector<layout::FlexSpan> spans = layout::SolveFlex(items, params);

    const ImVec2 start = ImGui::GetCursorPos();
    // Reserve the row's footprint up front with one invisible item. Children are
    // positioned absolutely below (and, under cross-axis align, may sit short of
    // the row's bottom), so without this the exact resume to start.y+height would
    // over-extend the window and trip ImGui's "SetCursorPos extended bounds
    // without an item" assertion. The Dummy claims the full rect; children draw on
    // top. (height<=0 fills the remaining space — children extend it themselves.)
    if (height > 0.0f)
        ImGui::Dummy(ImVec2(container, height));

    ImGui::PushID(id);
    for (std::size_t i = 0; i < children.size(); ++i) {
        // Omit a child the solver collapses to ~0 width: ImGui treats a child
        // width of exactly 0 as "fill the remaining space", which would make a
        // zero-width child balloon over its neighbours.
        if (!(spans[i].size > 0.5f))
            continue;
        // Cross (Y) axis from the solved span: a positive solved cross size wins
        // (real alignment), otherwise fall back to opt.height so a caller passing
        // only opt.height keeps the legacy uniform-height behavior (height 0 ⇒
        // BeginChild fills the remaining vertical space). crossOffset is 0 under
        // the default Start align, so the Y offset is harmless there.
        const float childY = start.y + spans[i].crossOffset;
        const float childH = spans[i].crossSize > 0.0f ? spans[i].crossSize : height;
        ImGui::SetCursorPos(ImVec2(start.x + spans[i].offset, childY));
        ImGui::BeginChild((std::string("##fc") + std::to_string(i)).c_str(),
                          ImVec2(spans[i].size, childH), ImGuiChildFlags_None);
        if (children[i].render)
            children[i].render();
        ImGui::EndChild();
    }
    ImGui::PopID();

    // Resume exactly at the row's bottom edge so following content flows
    // underneath with no extra spacing. This is in-bounds (the reserve Dummy above
    // already extended the content to start.y+height). Callers wanting a gap add
    // Spacing(). (height<=0 fills the remaining vertical space — nothing flows
    // after; see the doc note.)
    if (height > 0.0f)
        ImGui::SetCursorPos(ImVec2(start.x, start.y + height));
}

} // namespace Layout

// ── RAII horizontal layout ───────────────────────────────────────────────
/// Auto SameLine between children + restores spacing.
/// Usage: { HBox h; Button...; Button...; }
class HBox {
public:
    HBox(float spacing = -1.0f)
            : spacing_(spacing) {
        if (spacing_ >= 0)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing_, 0));
    }
    ~HBox() {
        if (spacing_ >= 0)
            ImGui::PopStyleVar();
    }
    static void VSeparator() {
        ImGui::SameLine();
        ImGui::TextUnformatted("|");
    }

private:
private:
    float spacing_ = -1.0f;
};

// ── RAII vertical layout ──────────────────────────────────────────────────
/// Auto-stacks children vertically with optional spacing override.
/// Usage: { VBox v; Button...; Button...; }
class VBox {
public:
    VBox(float spacing = -1.0f)
            : spacing_(spacing) {
        if (spacing_ >= 0)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, spacing_));
    }
    ~VBox() {
        if (spacing_ >= 0)
            ImGui::PopStyleVar();
    }

private:
    float spacing_ = -1.0f;
};

} // namespace unigui
