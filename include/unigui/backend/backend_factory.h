#pragma once

#include <unigui/backend/backend_types.h>
#include <unigui/backend/platform_backend.h>
#include <unigui/backend/renderer_backend.h>
#include <unigui/core/log.h>

#include <memory>

namespace unigui {

/// Creates a GLFW platform backend. @p type selects whether an OpenGL context is
/// created (GLFW_GL3) or the window is API-agnostic (GLFW_NO_API) so a DX/Metal/
/// WebGPU renderer can own the swapchain on the native handle.
std::unique_ptr<PlatformBackend> CreateGLFWPlatform(BackendType type = BackendType::GLFW_GL3);

/// Creates an SDL3 platform backend.
std::unique_ptr<PlatformBackend> CreateSDL3Platform();

/// Creates an OpenGL 3 renderer backend.
std::unique_ptr<RendererBackend> CreateOpenGL3Renderer();

/// Creates a Vulkan renderer backend.
std::unique_ptr<RendererBackend> CreateVulkanRenderer();

/// Creates a DirectX 11 renderer backend (Windows only). Only defined when the
/// DX11 backend is compiled in (UNIGUI_BACKEND_DX11=ON), mirroring CreateDX12Renderer.
#ifdef UNIGUI_HAS_DX11
std::unique_ptr<RendererBackend> CreateDX11Renderer();
#endif

/// Creates a Metal renderer backend (macOS only).
std::unique_ptr<RendererBackend> CreateMetalRenderer();

/// Creates a DX12 renderer backend (Windows only).
#ifdef UNIGUI_HAS_DX12
std::unique_ptr<RendererBackend> CreateDX12Renderer();
#endif

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
    UNIGUI_LOG_DEBUG("CreateBackend: type={}", (int) type);
    switch (type) {
    case BackendType::GLFW_GL3:
        return {CreateGLFWPlatform(BackendType::GLFW_GL3), CreateOpenGL3Renderer()};
    case BackendType::SDL3_Vulkan:
#if defined(UNIGUI_HAS_SDL3) && defined(UNIGUI_HAS_VULKAN)
        // SDL3 platform + the shared (platform-agnostic) Vulkan renderer.
        return {CreateSDL3Platform(), CreateVulkanRenderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::DX11:
#ifdef UNIGUI_HAS_DX11
        return {CreateGLFWPlatform(BackendType::DX11), CreateDX11Renderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::Metal:
#ifdef UNIGUI_HAS_METAL
        return {CreateGLFWPlatform(BackendType::Metal), CreateMetalRenderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::DX12:
#ifdef UNIGUI_HAS_DX12
        return {CreateGLFWPlatform(BackendType::DX12), CreateDX12Renderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::Vulkan:
#ifdef UNIGUI_HAS_VULKAN
        return {CreateGLFWPlatform(BackendType::Vulkan), CreateVulkanRenderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::WebGPU:
#ifdef UNIGUI_HAS_WEBGPU
        return {CreateGLFWPlatform(BackendType::WebGPU), CreateWebGPURenderer()};
#else
        // WebGPU renderer is a stub (no Dawn/wgpu linked). Honour the {nullptr,nullptr}
        // contract instead of returning a half-pair {valid platform, null renderer}.
        return {nullptr, nullptr};
#endif
    case BackendType::Emscripten:
#ifdef __EMSCRIPTEN__
        // The Emscripten/WebGL port renders through the OpenGL3 (WebGL) backend, NOT
        // the perpetually-null WebGPU stub.
        return {CreateEmscriptenPlatform(), CreateOpenGL3Renderer()};
#else
        return {nullptr, nullptr};
#endif
    }
    return {nullptr, nullptr};
}

/// Creates the default backend pair (GLFW + OpenGL3).
inline DefaultBackend CreateDefaultBackend() {
    return CreateBackend(BackendType::GLFW_GL3);
}

} // namespace unigui
