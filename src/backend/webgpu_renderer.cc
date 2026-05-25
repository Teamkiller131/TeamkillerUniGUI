#include <unigui/backend/backend_factory.h>
#include <cstdio>
namespace unigui {
// WebGPU backend — requires Dawn/WGPU implementation (Emscripten or native)
// Full implementation deferred to v2.8 when Dawn integration is complete
std::unique_ptr<RendererBackend> CreateWebGPURenderer() {
    std::fprintf(stderr, "[unigui] WebGPU backend requires Dawn/WGPU\n");
    return nullptr;
}
}
