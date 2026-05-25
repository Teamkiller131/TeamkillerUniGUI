#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>
#include <string_view>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig config;
    config.width = 800; config.height = 600;
    config.title = "Vulkan Triangle";
    config.backend = unigui::BackendType::SDL3_Vulkan;

    if (!unigui::Init(config)) {
        std::fprintf(stderr, "[vulkan_triangle] Init failed\n");
        return 1;
    }
    std::printf("[vulkan_triangle] SDL3+Vulkan initialized\n");

    int frame_count = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();
        unigui::Render();
        frame_count++;
        if (max_frames > 0 && frame_count >= max_frames) {
            std::printf("[vulkan_triangle] frame %d/%d rendered\n", frame_count, max_frames);
            break;
        }
    }
    unigui::Shutdown();
    std::printf("[vulkan_triangle] Shutdown complete\n");
    return 0;
}
