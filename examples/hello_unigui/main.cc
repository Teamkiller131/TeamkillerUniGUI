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
    config.width = 1280; config.height = 720; config.title = "Hello UniGUI";
#ifdef _WIN32
    config.backend = unigui::BackendType::DX11;
#endif
    if (!unigui::Init(config)) { std::fprintf(stderr, "Init failed\n"); return 1; }
    std::printf("[unigui] Initialized DX11\n");
    int frame_count = 0; bool done = false;
    while (!done && !unigui::ShouldClose()) {
        unigui::NewFrame();
        ImGui::ShowDemoWindow();
        {
            static auto window = std::make_shared<unigui::Window>("demo", "UniGUI Demo");
            static bool first = true;
            if (first) {
                auto panel = std::make_shared<unigui::Panel>("info", "Information");
                panel->SetContentCallback([]() { ImGui::TextWrapped("Welcome to UniGUI!"); });
                window->AddPanel(panel);
                first = false;
            }
            window->Render();
        }
        unigui::Render(); frame_count++;
        if (max_frames > 0 && frame_count >= max_frames) { done = true; }
    }
    unigui::Shutdown(); std::printf("[unigui] Done\n"); return 0;
}
