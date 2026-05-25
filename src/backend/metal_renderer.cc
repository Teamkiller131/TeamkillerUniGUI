// Metal renderer — placeholder. Metal is macOS-only.
// Full implementation requires Objective-C++ (`.mm` file) on macOS.
#include <unigui/backend/backend_factory.h>
#include <cstdio>

namespace unigui {

std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    std::fprintf(stderr, "[unigui] Metal backend is only available on macOS\n");
    return nullptr;
}

} // namespace unigui
