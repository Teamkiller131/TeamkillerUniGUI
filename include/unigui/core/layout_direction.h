#pragma once

namespace unigui {

/// UI layout direction — the mirroring switch for right-to-left presentation.
///
/// `RightToLeft` mirrors what a wrapper layer can mirror without forking the
/// layout engine: single-line text blocks right-align, so an Arabic/Hebrew/
/// Persian/Urdu UI reads right-to-left where it counts most. Deep mirroring
/// (control internals, table column order, tree indents, bidi line shaping) is
/// an ImGui layout-engine concern and is NOT implied by this flag — see
/// DEVELOPMENT_PLAN §7.
enum class LayoutDirection {
    LeftToRight, ///< default; the classic LTR presentation
    RightToLeft, ///< mirrored presentation for RTL scripts
};

/// Set the app-wide layout direction. Applies from the next frame; state is
/// process-global (per-app by design, like `config::Store` — not per-ImGui-
/// context).
void SetLayoutDirection(LayoutDirection direction);

/// The current layout direction (LeftToRight before any SetLayoutDirection).
LayoutDirection GetLayoutDirection();

/// Convenience predicate for `GetLayoutDirection() == LayoutDirection::RightToLeft`.
bool IsRightToLeft();

} // namespace unigui
