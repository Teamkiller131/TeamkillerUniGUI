#pragma once

#include <unigui/backend/backend_types.h>
#include <unigui/theme/theme.h>

#include <functional>

namespace unigui {

struct AppConfig {
    int width = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    ThemeConfig theme = {ThemePreset::Dark, 0.0f, 16.0f}; // auto-DPI, 16px logical
#ifdef _WIN32
    BackendType backend = BackendType::DX11; // DX11 is stable on Windows
#else
    BackendType backend = BackendType::GLFW_GL3;
#endif
    /// HiDPI: let Dear ImGui (≥1.92) re-rasterise fonts on the fly as the monitor
    /// content scale changes (sets `io.ConfigDpiScaleFonts`). With the dynamic
    /// font system this keeps text crisp across DPIs without pre-building glyph
    /// sizes — recommended for multi-monitor / fractional-DPI setups.
    bool dpiScaleFonts = false;
};

bool Init(const AppConfig& config);
void Shutdown();
bool NewFrame();
void Render();
bool ShouldClose();

/// Run the main loop, invoking @p callback once per frame between NewFrame()
/// and Render(). Calls Shutdown() automatically when the loop ends.
///
/// @param callback  Per-frame UI code.
/// @param maxFrames Stop after this many frames (0 = run until the window
///                  closes). Useful for screenshots, tests and CI smoke runs.
void Run(const std::function<void()>& callback, int maxFrames = 0);

/// One-call entry point: Init(config) followed by Run(callback, maxFrames),
/// with initialization-failure handling. Reduces a typical main() to:
///
///     int main() {
///         unigui::AppConfig cfg; cfg.title = "My App";
///         return unigui::RunApp(cfg, [] { ImGui::Text("Hello"); });
///     }
///
/// @return 0 on success, 1 if initialization failed.
int RunApp(const AppConfig& config, const std::function<void()>& callback, int maxFrames = 0);

/// v1.9: Get native window handle. Returns HWND on Windows, GLFWwindow* elsewhere.
void* GetNativeWindowHandle();

/// HiDPI content scale. Sets the per-viewport font DPI factor
/// (`ImGuiStyle::FontScaleDpi`, Dear ImGui ≥1.92), so fonts re-rasterise crisply
/// at the given scale (1.0 = 100%, 1.5 = 150%, …) using the dynamic font system —
/// no glyph-range pre-building. Requires a live ImGui context. Pairs with
/// `AppConfig::dpiScaleFonts` for automatic per-monitor scaling.
void SetContentScale(float scale);
/// Current content scale (`ImGuiStyle::FontScaleDpi`); 1.0 if no context.
float GetContentScale();

} // namespace unigui
