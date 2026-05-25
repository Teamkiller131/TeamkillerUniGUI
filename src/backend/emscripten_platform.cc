// Emscripten platform backend — Web/HTML5 canvas via Emscripten
// Requires: emscripten/html5.h, imgui_impl_sdl3.h or custom HTML5 bindings
// Compile with: emcmake cmake .. -DUNIGUI_BACKEND=EMSCRIPTEN

#include <unigui/backend/backend_factory.h>
#include <cstdio>
#include <memory>

namespace unigui {

#ifdef __EMSCRIPTEN__
// Full Emscripten implementation uses emscripten/html5.h
// Canvas, input, gamepad, clipboard, timing
// See: imgui_impl_sdl3.h for Emscripten support via SDL3
namespace {
class EmscriptenPlatform : public PlatformBackend {
public:
    bool Init(void* handle=nullptr) override { std::fprintf(stderr,"[unigui] Emscripten init\n"); return false; }
    void Shutdown() override {}
    void NewFrame() override {}
    void PollEvents() override {}
    bool ShouldClose() const override { return false; }
};
} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform() { return std::make_unique<EmscriptenPlatform>(); }
#else
std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform() {
    std::fprintf(stderr, "[unigui] Emscripten platform requires __EMSCRIPTEN__\n");
    return nullptr;
}
#endif

} // namespace unigui
