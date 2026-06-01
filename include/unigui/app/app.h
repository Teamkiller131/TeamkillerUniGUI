#pragma once

#include <unigui/theme/theme.h>
#include <unigui/backend/backend_types.h>
#include <functional>

namespace unigui {

struct AppConfig {
    int width = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    ThemeConfig theme = { ThemePreset::Dark, 0.0f, 16.0f }; // auto-DPI, 16px logical
#ifdef _WIN32
    BackendType backend = BackendType::DX11; // DX11 is stable on Windows
#else
    BackendType backend = BackendType::GLFW_GL3;
#endif
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
int RunApp(const AppConfig& config,
           const std::function<void()>& callback,
           int maxFrames = 0);


/// v1.9: Get native window handle. Returns HWND on Windows, GLFWwindow* elsewhere.
void* GetNativeWindowHandle();

} // namespace unigui
