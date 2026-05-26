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
void Run(const std::function<void()>& callback);

/// v1.9: Get native window handle. Returns HWND on Windows, GLFWwindow* elsewhere.
void* GetNativeWindowHandle();

} // namespace unigui
