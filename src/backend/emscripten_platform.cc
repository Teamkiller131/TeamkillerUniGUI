// Emscripten platform backend — Web/HTML5 canvas (WebAssembly + WebGL2).
//
// Emscripten's GLFW port implements the standard glfw3.h window/context/input APIs
// against the HTML5 canvas and a WebGL2 context, and ImGui's glfw + opengl3 backends
// support Emscripten directly. So the portable GLFWPlatform (backend/glfw_platform.cc)
// works unchanged on the web — CreateEmscriptenPlatform() reuses it instead of carrying
// a parallel platform. (A previous bespoke class adopted a window handle but never
// CREATED one, and its NewFrame didn't drive ImGui_ImplGlfw, so BackendType::Emscripten
// brought up no GL context and rendered nothing.)
#include <unigui/backend/backend_factory.h>

#include <cstdio>
#include <memory>

namespace unigui {

#ifdef __EMSCRIPTEN__

std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform() {
    // emscripten's GLFW maps a requested GL context version >= 3.0 onto WebGL2, so the
    // GLFW_GL3 path (real context + ImGui_ImplGlfw_InitForOpenGL) is exactly what the
    // web needs. The app loop hands control to the browser via emscripten_set_main_loop
    // (see app.cc Run()).
    return CreateGLFWPlatform(BackendType::GLFW_GL3);
}

#else // !__EMSCRIPTEN__

std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform() {
    std::fprintf(stderr, "[unigui] Emscripten platform requires the __EMSCRIPTEN__ compiler\n");
    return nullptr;
}

#endif // __EMSCRIPTEN__

} // namespace unigui
