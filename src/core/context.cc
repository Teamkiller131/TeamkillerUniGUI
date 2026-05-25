#include <unigui/core/context.h>
#include <imgui.h>

namespace unigui {

static ImGuiContext* g_context = nullptr;
static bool g_initialized = false;

bool CreateContext() {
    if (g_initialized) {
        return true; // Already initialized — idempotent
    }
    IMGUI_CHECKVERSION();
    g_context = ImGui::CreateContext();
    g_initialized = true;
    return true;
}

void DestroyContext() {
    if (!g_initialized) {
        return;
    }
    ImGui::DestroyContext();
    g_context = nullptr;
    g_initialized = false;
}

ImGuiContext* GetContext() {
    return g_context;
}

} // namespace unigui
