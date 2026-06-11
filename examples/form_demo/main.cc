#include <unigui/unigui.h>

#include <cstdio>
#include <cstdlib>
#include <string_view>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
    }
    unigui::AppConfig cfg;
    cfg.title = "Form Demo";
    if (!unigui::Init(cfg))
        return 1;
    std::printf("[form_demo] Init\n");

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();
        static auto win = std::make_shared<unigui::Window>("main", "Form Demo");
        static bool first = true;
        if (first) {
            auto panel = std::make_shared<unigui::Panel>("form", "Registration Form");
            panel->SetContentCallback([]() {
                static unigui::Form form("reg", "Registration");
                static bool init = false;
                if (!init) {
                    form.AddTextField("name", "Name", true);
                    form.AddTextField("email", "Email", true);
                    form.AddNumberField("age", "Age", 0, 120);
                    form.AddComboField("role", "Role", {"Admin", "User", "Guest"});
                    form.AddSliderField("exp", "Experience", 0, 20);
                    form.AddCheckbox("agree", "I agree to terms");
                    form.SetOnSubmit([] { std::printf("[form_demo] Submitted!\n"); });
                    init = true;
                }
                form.Render();
            });
            win->AddPanel(panel);
            first = false;
        }
        win->Render();
        unigui::Render();
        frame++;
        if (max_frames > 0 && frame >= max_frames)
            break;
    }
    unigui::Shutdown();
    return 0;
}
