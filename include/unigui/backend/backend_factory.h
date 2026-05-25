#pragma once

#include <unigui/backend/platform_backend.h>
#include <unigui/backend/renderer_backend.h>
#include <unigui/backend/backend_types.h>
#include <unigui/core/log.h>
#include <memory>

namespace unigui {

/// Creates a GLFW platform backend.
std::unique_ptr<PlatformBackend> CreateGLFWPlatform();

/// Creates an SDL3 platform backend.
std::unique_ptr<PlatformBackend> CreateSDL3Platform();

/// Creates an OpenGL 3 renderer backend.
std::unique_ptr<RendererBackend> CreateOpenGL3Renderer();

/// Creates a Vulkan renderer backend.
std::unique_ptr<RendererBackend> CreateVulkanRenderer();

/// Creates a DirectX 11 renderer backend (Windows only).
std::unique_ptr<RendererBackend> CreateDX11Renderer();

/// Creates a Metal renderer backend (macOS only).
std::unique_ptr<RendererBackend> CreateMetalRenderer();

/// Creates a DX12 renderer backend (Windows only).
std::unique_ptr<RendererBackend> CreateDX12Renderer();

/// Creates a WebGPU renderer backend.
std::unique_ptr<RendererBackend> CreateWebGPURenderer();

/// Creates an Emscripten platform backend.
std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform();

/// Creates the default platform + renderer backend pair.
struct DefaultBackend {
    std::unique_ptr<PlatformBackend> platform;
    std::unique_ptr<RendererBackend> renderer;
};

/// Creates backends based on the specified type.
inline DefaultBackend CreateBackend(BackendType type) {
    UNIGUI_LOG_DEBUG("CreateBackend: type={}", (int)type);
    switch (type) {
    case BackendType::GLFW_GL3:
        return { CreateGLFWPlatform(), CreateOpenGL3Renderer() };
    case BackendType::SDL3_Vulkan:
#ifdef UNIGUI_HAS_SDL3_VULKAN
        return { CreateSDL3Platform(), CreateVulkanRenderer() };
#else
        return { nullptr, nullptr };
#endif
    case BackendType::DX11:
#ifdef UNIGUI_HAS_DX11
        return { CreateGLFWPlatform(), CreateDX11Renderer() };
#else
        return { nullptr, nullptr };
#endif
    case BackendType::Metal:
#ifdef __APPLE__
        return { CreateGLFWPlatform(), CreateMetalRenderer() };
#else
        return { nullptr, nullptr };
#endif
    case BackendType::DX12:
#ifdef UNIGUI_HAS_DX12
        return { CreateGLFWPlatform(), CreateDX12Renderer() };
#else
        return { nullptr, nullptr };
#endif
    case BackendType::WebGPU:
        return { CreateGLFWPlatform(), CreateWebGPURenderer() };
    case BackendType::Emscripten:
        return { CreateEmscriptenPlatform(), CreateWebGPURenderer() };
    }
    return { nullptr, nullptr };
}

/// Creates the default backend pair (GLFW + OpenGL3).
inline DefaultBackend CreateDefaultBackend() {
    return CreateBackend(BackendType::GLFW_GL3);
}

} // namespace unigui
