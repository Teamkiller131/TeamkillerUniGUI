#pragma once

namespace unigui {

/// Backend type selection.
enum class BackendType {
    GLFW_GL3,    ///< GLFW platform + OpenGL 3 renderer (default)
    SDL3_Vulkan, ///< SDL3 platform + shared Vulkan renderer (opt-in; needs SDL3)
    DX11,        ///< DirectX 11 renderer (Windows only)
    DX12,        ///< DirectX 12 renderer (Windows only)
    Metal,       ///< Metal renderer (macOS only)
    WebGPU,      ///< WebGPU renderer (cross-platform via Dawn/WGPU)
    Emscripten,  ///< Emscripten/Web platform
    Vulkan,      ///< GLFW platform + shared Vulkan renderer (cross-platform)
};

/// Configuration for creating a backend.
struct BackendConfig {
    int width = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    BackendType backend = BackendType::GLFW_GL3;
};

/// One physical display, as reported by the platform. Positions and sizes are
/// in virtual-desktop coordinates (a secondary monitor left of the primary has
/// a negative `x`); the work area excludes the taskbar/dock.
struct MonitorInfo {
    int x = 0, y = 0;                  ///< display rect origin
    int width = 0, height = 0;         ///< display rect size
    int workX = 0, workY = 0;          ///< work-area origin (taskbar excluded)
    int workWidth = 0, workHeight = 0; ///< work-area size
    float dpiScale = 1.0f;             ///< content scale (1.0 = 96dpi, 1.5 = 144dpi)
};

} // namespace unigui
