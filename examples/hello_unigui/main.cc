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
    config.width = 1280;
    config.height = 720;
    config.title = "Hello UniGUI";

    if (!unigui::Init(config)) {
        std::fprintf(stderr, "[unigui] Failed to initialize\n");
        return 1;
    }
    std::printf("[unigui] Initialized backend=GLFW+OpenGL3\n");

    int frame_count = 0;
    bool done = false;

    while (!done && !unigui::ShouldClose()) {
        unigui::NewFrame();

        // Diagnostic: direct ImGui window to verify rendering
        ImGui::Begin("Diagnostic");
        ImGui::Text("Frame: %d, FPS: %.1f", frame_count, ImGui::GetIO().Framerate);
        ImGui::End();

        {
            static auto window = std::make_shared<unigui::Window>("demo", "UniGUI Demo");
            static bool first = true;
            if (first) {
                auto panel = std::make_shared<unigui::Panel>("info", "Information");
                panel->SetContentCallback([]() {
                    ImGui::TextWrapped("Welcome to UniGUI! Dark theme + widget library demo.");
                });
                window->AddPanel(panel);

                // v2.0 widgets demo
                auto v2panel = std::make_shared<unigui::Panel>("v2widgets", "v2.0 Widgets");
                v2panel->SetContentCallback([]() {
                    ImGui::Text("v2.0 Widget Demos:");
                    static unigui::TabWidget tabs("v2tabs");
                    tabs.AddTab({"ck", "CheckBox", []() {
                        static unigui::CheckBox cb("cb1", "Enable", true);
                        cb.Render();
                    }});
                    tabs.AddTab({"pg", "Progress", []() {
                        static unigui::ProgressBar pb("pb", 0.65f);
                        pb.SetOverlayText("65%%");
                        pb.Render();
                    }});
                    tabs.AddTab({"cm", "ComboBox", []() {
                        static unigui::ComboBox cb("cmb", "Fruit", {"Apple","Banana","Cherry"}, 0);
                        cb.Render();
                    }});
                    tabs.AddTab({"rg", "Radio", []() {
                        static unigui::RadioGroup rg("rg", {"A","B","C"}, 1);
                        rg.Render();
                    }});
                    tabs.AddTab({"le", "LineEdit", []() {
                        static unigui::LineEdit le("le", "Email");
                        le.SetPlaceholder("user@example.com");
                        le.Render();
                    }});
                    tabs.AddTab({"gb", "Group", []() {
                        static unigui::GroupBox gb("gb", "Settings");
                        gb.SetContentCallback([]() {
                            static unigui::CheckBox c1("o1", "Option 1");
                            c1.Render();
                        });
                        gb.Render();
                    }});
                    tabs.Render();
                });
                window->AddPanel(v2panel);

                window->SetMenuBarEnabled(true);
                first = false;
            }
            window->Render();
        }

        unigui::Render();
        frame_count++;

        if (max_frames > 0 && frame_count >= max_frames) {
            std::printf("[unigui] frame %d/%d rendered\n", frame_count, max_frames);
            done = true;
        }
    }

    unigui::Shutdown();
    std::printf("[unigui] Shutdown complete\n");
    return 0;
}
