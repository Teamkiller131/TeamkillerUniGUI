// Implementation of the stable C ABI (include/unigui/capi/unigui_capi.h).
//
// Every function here is a thin, ownership-clean bridge over the C++ layer: no
// C++ types cross the boundary, no exception can escape, and the ABI contract
// lives entirely in the header. See docs/C_API.md for the growth policy and
// the ABI stability rules.

#include <unigui/app/app.h>
#include <unigui/capi/unigui_capi.h>
#include <unigui/core/version.h>
#include <unigui/im/im.h>

#include <imgui.h> // TextV (va_list formatting behind the ABI)

#include <cstdarg>

// Opaque-handle definition: clients only ever see `unigui_app*`; the layout
// lives here (it is NOT part of the ABI).
struct unigui_app {
    unigui::AppConfig config;
    unigui_frame_fn frame = nullptr;
    void* userdata = nullptr;
    bool initialized = false;
};

namespace {

unigui::BackendType ToBackendType(unigui_backend b) {
    switch (b) {
    case UNIGUI_BACKEND_DEFAULT:
        return unigui::BackendType::GLFW_GL3; // replaced below
    case UNIGUI_BACKEND_GLFW_GL3:
        return unigui::BackendType::GLFW_GL3;
    case UNIGUI_BACKEND_SDL3_VULKAN:
        return unigui::BackendType::SDL3_Vulkan;
    case UNIGUI_BACKEND_DX11:
        return unigui::BackendType::DX11;
    case UNIGUI_BACKEND_DX12:
        return unigui::BackendType::DX12;
    case UNIGUI_BACKEND_METAL:
        return unigui::BackendType::Metal;
    case UNIGUI_BACKEND_WEBGPU:
        return unigui::BackendType::WebGPU;
    case UNIGUI_BACKEND_EMSCRIPTEN:
        return unigui::BackendType::Emscripten;
    case UNIGUI_BACKEND_VULKAN:
        return unigui::BackendType::Vulkan;
    }
    return unigui::BackendType::GLFW_GL3;
}

} // namespace

void unigui_app_config_init(unigui_app_config* cfg) {
    if (!cfg)
        return;
    cfg->width = 1280;
    cfg->height = 720;
    cfg->title = "UniGUI Application"; // static storage — never freed
    cfg->backend = UNIGUI_BACKEND_DEFAULT;
    cfg->dpi_scale_fonts = 0;
    cfg->accessibility = 0;
    cfg->multi_viewport = 0;
}

// ── Version & ABI gate ────────────────────────────────────────────────────────

int unigui_version_major() {
    return UNIGUI_VERSION_MAJOR;
}
int unigui_version_minor() {
    return UNIGUI_VERSION_MINOR;
}
int unigui_version_patch() {
    return UNIGUI_VERSION_PATCH;
}
const char* unigui_version_string() {
    return UNIGUI_VERSION_STRING;
}

int unigui_capi_abi_compatible(int reported) {
    // Additive-only growth: a binding asks for the oldest ABI it was built
    // against; anything >= that is compatible. A reported value above ours
    // means the binding is newer than the library — reject.
    return reported >= 1 && reported <= UNIGUI_CAPI_ABI_VERSION;
}

// ── App lifecycle ─────────────────────────────────────────────────────────────

unigui_app* unigui_app_create(const unigui_app_config* config) {
    auto* app = new unigui_app;
    if (config) {
        app->config.width = config->width;
        app->config.height = config->height;
        if (config->title)
            app->config.title = config->title;
        if (config->backend != UNIGUI_BACKEND_DEFAULT)
            app->config.backend = ToBackendType(config->backend);
        // UNIGUI_BACKEND_DEFAULT keeps AppConfig's platform default (DX11 on
        // Windows, GLFW+GL3 elsewhere) instead of forcing GLFW_GL3 here.
        app->config.dpiScaleFonts = config->dpi_scale_fonts != 0;
        app->config.accessibility = config->accessibility != 0;
        app->config.multiViewport = config->multi_viewport != 0;
    }
    if (!unigui::Init(app->config)) {
        delete app;
        return nullptr; // failure already logged by the app layer
    }
    app->initialized = true;
    return app;
}

void unigui_app_destroy(unigui_app* app) {
    if (!app)
        return;
    if (app->initialized)
        unigui::Shutdown();
    delete app;
}

int unigui_app_new_frame(unigui_app* app) {
    if (!app || !app->initialized)
        return 0;
    return unigui::NewFrame() ? 1 : 0;
}

void unigui_app_render(unigui_app* app) {
    if (!app || !app->initialized)
        return;
    unigui::Render();
}

int unigui_app_should_close(const unigui_app* app) {
    if (!app || !app->initialized)
        return 1; // a dead app should stop any loop still spinning on it
    return unigui::ShouldClose() ? 1 : 0;
}

int unigui_app_run(unigui_app* app, unigui_frame_fn frame, void* userdata, int max_frames) {
    if (!app || !app->initialized)
        return 1;
    app->frame = frame;
    app->userdata = userdata;
    unigui::Run(
        [app] {
            if (app->frame)
                app->frame(app->userdata);
        },
        max_frames);
    // unigui::Run calls Shutdown() when the loop ends; the handle stays valid
    // so the caller can free it (and inspect nothing else) via destroy.
    app->initialized = false;
    return 0;
}

void* unigui_app_native_window_handle(const unigui_app* app) {
    if (!app || !app->initialized)
        return nullptr;
    return unigui::GetNativeWindowHandle();
}

void unigui_app_set_content_scale(unigui_app* app, float scale) {
    if (!app || !app->initialized)
        return;
    unigui::SetContentScale(scale);
}

float unigui_app_get_content_scale(const unigui_app* app) {
    if (!app || !app->initialized)
        return 1.0f;
    return unigui::GetContentScale();
}

// ── Immediate-mode drawing subset ─────────────────────────────────────────────

int unigui_begin(const char* title, int* p_open) {
    // im::Begin takes a bool*; the C ABI's int* is 4 bytes wide, so bridge
    // through a local and write back (the write-back also makes *p_open exact,
    // never a truncated bool).
    bool open = p_open ? (*p_open != 0) : true;
    const bool shown = unigui::im::Begin(title ? title : "", p_open ? &open : nullptr);
    if (p_open)
        *p_open = open ? 1 : 0;
    return shown ? 1 : 0;
}

void unigui_end() {
    unigui::im::End();
}

void unigui_text_unformatted(const char* text) {
    unigui::im::TextUnformatted(text ? text : "");
}

void unigui_text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt ? fmt : "", args);
    va_end(args);
}

int unigui_button(const char* label) {
    return unigui::im::Button(label ? label : "") ? 1 : 0;
}

int unigui_checkbox(const char* label, int* value) {
    if (!value)
        return 0;
    bool v = *value != 0;
    const bool changed = unigui::im::Checkbox(label ? label : "", &v);
    *value = v ? 1 : 0;
    return changed ? 1 : 0;
}

int unigui_slider_float(const char* label, float* value, float v_min, float v_max) {
    if (!value)
        return 0;
    return unigui::im::SliderFloat(label ? label : "", value, v_min, v_max) ? 1 : 0;
}

void unigui_separator() {
    unigui::im::Separator();
}
