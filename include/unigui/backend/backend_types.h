#pragma once

namespace unigui {

/// Backend type selection.
enum class BackendType {
    GLFW_GL3,       ///< GLFW platform + OpenGL 3 renderer (default)
    SDL3_Vulkan,    ///< SDL3 platform + shared Vulkan renderer (opt-in; needs SDL3)
    DX11,           ///< DirectX 11 renderer (Windows only)
    DX12,           ///< DirectX 12 renderer (Windows only)
    Metal,          ///< Metal renderer (macOS only)
    WebGPU,         ///< WebGPU renderer (cross-platform via Dawn/WGPU)
    Emscripten,     ///< Emscripten/Web platform
    Vulkan,         ///< GLFW platform + shared Vulkan renderer (cross-platform)
};

/// Configuration for creating a backend.
struct BackendConfig {
    int width = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    BackendType backend = BackendType::GLFW_GL3;
};

} // namespace unigui
