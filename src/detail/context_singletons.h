#pragma once

// Reset helper for the singletons migrated to ContextRegistry — the app loop
// calls this on Shutdown (before DestroyContext) so per-context instances don't
// outlive their context. Lives in src/detail (internal; the public API surface
// is unchanged).

#include "context_registry.h"

#include <imgui.h>

#include <unigui/fonts/font_manager.h>
#include <unigui/fx/animation.h>
#include <unigui/styling/style_engine.h>
#ifdef UNIGUI_HAS_WIDGETS
#include <unigui/widgets/toast.h>
#endif

namespace unigui::detail {

inline void ResetContextSingletons(ImGuiContext* ctx) {
    ContextRegistry<fonts::Manager>::Reset(ctx);
    ContextRegistry<fx::AnimationManager>::Reset(ctx);
    ContextRegistry<styling::Engine>::Reset(ctx);
#ifdef UNIGUI_HAS_WIDGETS
    ContextRegistry<Toast>::Reset(ctx);
#endif
}

} // namespace unigui::detail
