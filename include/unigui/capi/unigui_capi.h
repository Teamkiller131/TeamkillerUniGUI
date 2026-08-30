#ifndef UNIGUI_CAPI_H
#define UNIGUI_CAPI_H
// -----------------------------------------------------------------------------
// unigui_capi.h - the stable C ABI
//
// A small, frozen-at-the-boundary C surface over the UniGUI C++ library, for
// hosts that cannot (or prefer not to) consume C++ directly: C applications,
// C#/Python/Go FFI bindings, embedding layers, and ABI-stability-sensitive
// integrations. It is deliberately a SUBSET: the full C++ API stays the
// first-class surface, and this layer grows additively as demand appears.
//
// ABI policy (mirrors docs/C_API.md):
//   - UNIGUI_CAPI_ABI_VERSION is the ABI contract revision. It is bumped ONLY
//     by additive change: new functions, new trailing struct fields, new enum
//     values. Existing function signatures, struct layouts and enum values are
//     frozen within an ABI version.
//   - A binding compiled against ABI version N works against any library whose
//     reported ABI version is >= N. Check at startup with
//     unigui_capi_abi_compatible(your_abi_version) and fail fast with a clear
//     message if it returns 0.
//   - The ABI version is independent of the library version
//     (unigui_version_*): a 4.9.1 patch release does not change the ABI number
//     unless the C surface changed.
//
// Conventions:
//   - Every function and type here is plain C99 (no bool in signatures, no
//     exceptions can cross the boundary, no C++ types). Strings are UTF-8
//     `const char*` and are owned by the caller unless documented otherwise.
//   - Boolean results are `int`: 0 = false, non-zero = true.
//   - All calls must come from the thread that created the app (same contract
//     as the C++ app API); none of these functions are thread-safe by
//     themselves.
//   - An app handle owns one application. The C++ app layer is process-global,
//     so at most one live app exists at a time - create a second one only
//     after destroying the first.
// -----------------------------------------------------------------------------

#include <unigui/unigui_export.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Import/export annotation for the C surface. In a static build this expands
/// to nothing; in a shared build it applies the library's dllexport/dllimport
/// (or ELF visibility) attribute.
#define UNIGUI_CAPI extern UNIGUI_API

/// The C ABI contract revision (see the policy block above).
#define UNIGUI_CAPI_ABI_VERSION 2

/// Renderer/platform selection, mirroring `unigui::BackendType`. The enum
/// values are frozen; new backends append at the end.
typedef enum unigui_backend {
    UNIGUI_BACKEND_DEFAULT = 0, ///< Platform default (DX11 on Windows, GLFW+GL3 elsewhere)
    UNIGUI_BACKEND_GLFW_GL3,    ///< GLFW platform + OpenGL 3 renderer
    UNIGUI_BACKEND_SDL3_VULKAN, ///< SDL3 platform + shared Vulkan renderer
    UNIGUI_BACKEND_DX11,        ///< DirectX 11 renderer (Windows only)
    UNIGUI_BACKEND_DX12,        ///< DirectX 12 renderer (Windows only)
    UNIGUI_BACKEND_METAL,       ///< Metal renderer (macOS only)
    UNIGUI_BACKEND_WEBGPU,      ///< WebGPU renderer (cross-platform via Dawn/WGPU)
    UNIGUI_BACKEND_EMSCRIPTEN,  ///< Emscripten/Web platform
    UNIGUI_BACKEND_VULKAN       ///< GLFW platform + shared Vulkan renderer
} unigui_backend;

/// Application configuration, mirroring `unigui::AppConfig`. Zero-initialise
/// and set what you need, or call unigui_app_config_init() for the defaults.
/// New fields may be appended in a future ABI version (never reordered).
typedef struct unigui_app_config {
    int width;              ///< Window width in logical points (default 1280)
    int height;             ///< Window height in logical points (default 720)
    const char* title;      ///< UTF-8 window title (default "UniGUI Application");
                            ///< must remain valid for the app's lifetime
    unigui_backend backend; ///< Renderer/platform (default UNIGUI_BACKEND_DEFAULT)
    int dpi_scale_fonts;    ///< 1 = re-rasterise fonts as the monitor scale changes
    int accessibility;      ///< 1 = a11y element tree + platform screen-reader bridge
    int multi_viewport;     ///< 1 = ImGui windows may pop out into OS windows
} unigui_app_config;

/// Fill @p cfg with the library defaults (mirrors `unigui::AppConfig`'s
/// default member initialisers).
UNIGUI_CAPI void unigui_app_config_init(unigui_app_config* cfg);

// -- Version & ABI gate --------------------------------------------------------

UNIGUI_CAPI int unigui_version_major(void);
UNIGUI_CAPI int unigui_version_minor(void);
UNIGUI_CAPI int unigui_version_patch(void);
/// Static string, e.g. "4.9.0" - valid for the process lifetime.
UNIGUI_CAPI const char* unigui_version_string(void);
/// 1 when a binding compiled against ABI version @p reported can safely run
/// against this library (the library's ABI is >= reported). Call this first.
UNIGUI_CAPI int unigui_capi_abi_compatible(int reported);

// -- App lifecycle -------------------------------------------------------------

/// Opaque application handle (heap-allocated by create; freed by destroy).
typedef struct unigui_app unigui_app;

/// Per-frame UI callback: invoked once per frame between NewFrame and Render,
/// exactly where a C++ app's lambda would run. @p userdata is the pointer
/// passed to unigui_app_run().
typedef void (*unigui_frame_fn)(void* userdata);

/// Create and initialise an application (window + backend + ImGui context).
/// Returns NULL if initialisation failed - the failure is already logged.
/// The returned handle must be released with unigui_app_destroy().
UNIGUI_CAPI unigui_app* unigui_app_create(const unigui_app_config* config);

/// Shut down (if still running) and free the handle. NULL is a no-op.
UNIGUI_CAPI void unigui_app_destroy(unigui_app* app);

/// Begin a frame (poll events, start the backend frame, NewFrame). Returns 1
/// when the frame started; 0 when the app is not initialised. Follow with
/// drawing calls, then unigui_app_render().
UNIGUI_CAPI int unigui_app_new_frame(unigui_app* app);

/// End the frame: Render() + backend present.
UNIGUI_CAPI void unigui_app_render(unigui_app* app);

/// 1 when the window is closing (the user closed it or the platform asked to
/// quit) - break the manual loop when this turns 1.
UNIGUI_CAPI int unigui_app_should_close(const unigui_app* app);

/// Run the full loop: @p frame is called once per frame until the window
/// closes or @p max_frames frames have run (0 = no frame limit). Shuts the app
/// down when the loop ends. Requires a handle from unigui_app_create().
/// Returns 0 on success, 1 if the app was not initialised.
UNIGUI_CAPI int unigui_app_run(unigui_app* app, unigui_frame_fn frame, void* userdata,
                               int max_frames);

/// The platform window handle: HWND on Windows, GLFWwindow* elsewhere.
/// NULL before initialisation / after shutdown.
UNIGUI_CAPI void* unigui_app_native_window_handle(const unigui_app* app);

/// HiDPI content scale (1.0 = 100%, 1.5 = 150%, ...) - see `unigui::SetContentScale`.
UNIGUI_CAPI void unigui_app_set_content_scale(unigui_app* app, float scale);
UNIGUI_CAPI float unigui_app_get_content_scale(const unigui_app* app);

// -- Immediate-mode drawing subset ---------------------------------------------
//
// The first tranche of the `unigui::im` layer over the ABI - enough for a
// "hello world" host and the binding smoke tests. More `im` calls land as the
// binding demand grows (see docs/C_API.md for the growth policy). All of these
// require a live app (or at least a current ImGui context) and must be called
// between unigui_app_new_frame()/unigui_app_render() - or inside
// unigui_app_run()'s frame callback.

/// Begin a window. Returns 1 while the window is visible; still call
/// unigui_end() when it returns 0 (clipped/collapsed windows pair the same way).
/// @p p_open may be NULL. When non-NULL the window shows a title-bar close
/// button, and *p_open is written back 0 when the user closes the window -
/// stop calling unigui_begin() for it on subsequent frames (the C++ contract;
/// the flag is a close hook, not a visibility switch).
UNIGUI_CAPI int unigui_begin(const char* title, int* p_open);
UNIGUI_CAPI void unigui_end(void);
UNIGUI_CAPI void unigui_text_unformatted(const char* text);
/// printf-style formatted text (UTF-8, positional args unsupported).
UNIGUI_CAPI void unigui_text(const char* fmt, ...);
/// Returns 1 on the frame the button is clicked.
UNIGUI_CAPI int unigui_button(const char* label);
/// Toggles *value in place; returns 1 on the frame it changed.
UNIGUI_CAPI int unigui_checkbox(const char* label, int* value);
/// Slider over [v_min, v_max]; writes *value; returns 1 on the frame it changed.
UNIGUI_CAPI int unigui_slider_float(const char* label, float* value, float v_min, float v_max);
UNIGUI_CAPI void unigui_separator(void);

// -- Form & layout tranche (added in ABI v2; everything above is unchanged) ----
//
// ABI growth rule in action: this tranche only APPENDS functions, so a binding
// compiled against ABI v1 keeps working (unigui_capi_abi_compatible(1) still
// passes); bindings that need these calls compile against ABI v2.

/// Layout: put the next item on the same line as the previous one / add
/// vertical spacing.
UNIGUI_CAPI void unigui_same_line(void);
UNIGUI_CAPI void unigui_spacing(void);
/// Radio group member: returns 1 on the frame it becomes selected; *current
/// holds the shared selection index.
UNIGUI_CAPI int unigui_radio_button(const char* label, int* current, int value);
/// Drop-down over a caller-owned, NULL-terminated item array of @p items_count
/// entries. *current is the selected index; returns 1 on the frame it changes.
UNIGUI_CAPI int unigui_combo(const char* label, int* current, const char* const* items,
                             int items_count);
/// Single-line text input into a caller-owned buffer of @p buf_capacity bytes
/// (includes the NUL). Returns 1 while the value is being edited.
UNIGUI_CAPI int unigui_input_text(const char* label, char* buf, size_t buf_capacity);
/// Integer / float drag inputs. Return 1 on the frame the value changed.
UNIGUI_CAPI int unigui_input_int(const char* label, int* value);
UNIGUI_CAPI int unigui_input_float(const char* label, float* value);
/// Integer slider over [v_min, v_max]; returns 1 on the frame it changed.
UNIGUI_CAPI int unigui_slider_int(const char* label, int* value, int v_min, int v_max);
/// Horizontal progress bar; @p fraction in [0, 1] (clamped).
UNIGUI_CAPI void unigui_progress_bar(float fraction);
/// Tooltip for the PREVIOUS item (call right after it, while it is hovered).
UNIGUI_CAPI void unigui_set_tooltip(const char* text);
/// Selectable row: @p selected pre-highlights it; returns 1 on the clicked frame.
UNIGUI_CAPI int unigui_selectable(const char* label, int selected);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // UNIGUI_CAPI_H
