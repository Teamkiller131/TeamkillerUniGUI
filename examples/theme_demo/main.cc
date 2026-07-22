/// theme_demo — cycles through all 10 themes automatically
#include <unigui/unigui.h>

#include <cstdio>
#include <format>

namespace im = unigui::im;

int main(int argc, char** argv) {
    int max_frames = 0;
    bool autoCycle = true;
    int switchInterval = 60; // frames

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
        else if (a == "--interval" && i + 1 < argc)
            switchInterval = std::atoi(argv[++i]);
        else if (a == "--manual")
            autoCycle = false;
    }

    unigui::AppConfig cfg;
    cfg.width = 800;
    cfg.height = 500;
    cfg.title = "UniGUI v3.0 — Theme Demo";
    if (!unigui::Init(cfg))
        return 1;

    auto names = unigui::theme::ThemeRegistry::Instance().List();
    int currentTheme = 0;
    unigui::theme::ThemeRegistry::Instance().Apply(names[currentTheme]);

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

        im::SetNextWindowPos(ImVec2(0, 0));
        im::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        {
            unigui::WindowScope window{"Theme Demo", nullptr,
                                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse};

            // Theme name display
            im::SetCursorPos(ImVec2(20, 20));
            im::Text(std::format("Current Theme: {} ({}/{})", names[currentTheme], currentTheme + 1,
                                 (int) names.size()));

            // Progress bar
            float pct = (float) (frame % switchInterval) / (float) switchInterval;
            im::SetCursorPos(ImVec2(20, 50));
            im::ProgressBar(pct, ImVec2(760, 20));

            // Card with theme info
            im::SetCursorPos(ImVec2(20, 90));
            im::BeginChild("card", ImVec2(360, 200), ImGuiChildFlags_Borders);
            im::TextWrapped(std::format("This demo cycles through all {} built-in themes.",
                                        (int) names.size()));
            im::Spacing();
            im::Text("Available themes:");
            for (auto& n : names) {
                im::BulletText(n);
            }
            im::EndChild();

            // Buttons
            im::SetCursorPos(ImVec2(400, 90));
            im::BeginChild("buttons", ImVec2(380, 200), ImGuiChildFlags_Borders);
            im::Text("Animated Buttons:");
            static unigui::Button b1("tb1", "Primary");
            b1.SetColorVariant(unigui::Button::Primary);
            static unigui::Button b2("tb2", "Danger");
            b2.SetColorVariant(unigui::Button::Danger);
            static unigui::Button b3("tb3", "Success");
            b3.SetColorVariant(unigui::Button::Success);
            b1.Render();
            im::SameLine();
            b2.Render();
            im::SameLine();
            b3.Render();

            im::Spacing();
            im::Text("Badges:");
            static unigui::Badge dot("");
            dot.SetVariant(unigui::Badge::Dot);
            static unigui::Badge cnt("");
            cnt.SetCount(7);
            im::Text("Dot:");
            im::SameLine();
            dot.Render();
            im::SameLine(60);
            im::Text("Count:");
            im::SameLine();
            cnt.Render();

            im::Spacing();
            im::Text("Animated ProgressBar:");
            static unigui::ProgressBar pb("tpb", 0.f);
            static float pbVal = 0.f;
            pbVal += 0.008f;
            if (pbVal > 1.f)
                pbVal = 0.f;
            pb.SetFraction(pbVal);
            pb.Render();
            im::EndChild();

            // Footer
            im::SetCursorPos(ImVec2(20, 340));
            if (autoCycle) {
                im::Text(std::format("Auto-cycling every {} frames. Use --manual to stop.",
                                     switchInterval));
                if (frame % switchInterval == 0 && frame > 0) {
                    currentTheme = (currentTheme + 1) % (int) names.size();
                    unigui::theme::ThemeRegistry::Instance().Apply(names[currentTheme]);
                }
            } else {
                im::Text("Manual mode. Use --auto to enable auto-cycle.");
                if (im::Button("< Prev")) {
                    currentTheme = (currentTheme - 1 + (int) names.size()) % (int) names.size();
                    unigui::theme::ThemeRegistry::Instance().Apply(names[currentTheme]);
                }
                im::SameLine();
                if (im::Button("Next >")) {
                    currentTheme = (currentTheme + 1) % (int) names.size();
                    unigui::theme::ThemeRegistry::Instance().Apply(names[currentTheme]);
                }
            }
        }
        unigui::Render();
        frame++;
        if (max_frames > 0 && frame >= max_frames)
            break;
    }

    unigui::Shutdown();
    std::printf("[theme_demo] Done — %d frames\n", frame);
    return 0;
}
