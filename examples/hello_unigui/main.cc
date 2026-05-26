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
        {
            static auto window = std::make_shared<unigui::Window>("demo", "UniGUI Demo");
            static bool first = true;
            if (first) {
                auto panel = std::make_shared<unigui::Panel>("info", "Welcome");
                panel->SetContentCallback([]() {
                    ImGui::TextWrapped("Welcome to UniGUI! A modern Dear ImGui C++ wrapper with 48+ widgets, "
                                       "dark/light theme, DPI auto-scaling, DX11/OpenGL/Vulkan backends, "
                                       "and full CJK font support.");
                });
                window->AddPanel(panel);

                auto wrapPanel = std::make_shared<unigui::Panel>("wrap", "Auto-Wrap Demo");
                wrapPanel->SetWrapEnabled(true);
                wrapPanel->SetContentCallback([]() {
                    ImGui::TextUnformatted("This panel has auto-wrap enabled. "
                        "Long text will automatically wrap to fit the panel width. "
                        "No need to manually insert line breaks or call TextWrapped — "
                        "the PushTextWrapPos is applied automatically before your content callback.");
                });
                window->AddPanel(wrapPanel);

                auto i18nPanel = std::make_shared<unigui::Panel>("i18n", "i18n Test");
                i18nPanel->SetContentCallback([]() {
                    ImGui::TextUnformatted("English: Hello, world! This is a Dear ImGui C++ wrapper library.");
                    ImGui::Separator();
                    ImGui::TextUnformatted("Chinese: 你好，世界！这是一个 Dear ImGui C++ 封装库。");
                    ImGui::TextUnformatted("Japanese: こんにちは世界！これはImGuiのラッパーです。");
                    ImGui::TextUnformatted("Korean: 안녕하세요 세계! 이것은 ImGui 래퍼입니다.");
                    ImGui::Separator();
                    ImGui::TextUnformatted("Arabic: مرحبا بالعالم! هذا هو مغلف ImGui.");
                    ImGui::TextUnformatted("Thai: สวัสดีชาวโลก! นี่คือไลบรารี ImGui.");
                    ImGui::Separator();
                    ImGui::TextUnformatted("Emoji: 🎉 🚀 ✨ 💻 🎨 (emoji support test)");
                });
                window->AddPanel(i18nPanel);
                first = false;
            }
            window->Render();
        }
        // ShowDemoWindow LAST — popups get top input priority
        ImGui::ShowDemoWindow();
        unigui::Render(); frame_count++;
        if (max_frames > 0 && frame_count >= max_frames) { done = true; }
    }
    unigui::Shutdown(); std::printf("[unigui] Done\n"); return 0;
}
