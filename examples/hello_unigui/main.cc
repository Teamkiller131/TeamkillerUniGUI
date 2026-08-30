#include <unigui/unigui.h>

#include <cstdlib>
#include <format>
#include <string_view>

namespace im = unigui::im;

// ── v3.4: demonstrate RunApp + fluent widget API ─────────────────────────────

static void BuildDemoWindow() {
    static auto window = std::make_shared<unigui::Window>("demo", "UniGUI Demo");
    static bool built = false;
    if (!built) {
        // ── Welcome panel ────────────────────────────────────────────────
        auto welcome = std::make_shared<unigui::Panel>("info", "Welcome");
        welcome->SetContentCallback([]() {
            im::TextWrapped("Welcome to UniGUI v" UNIGUI_VERSION_STRING "! "
                            "A modern Dear ImGui C++ wrapper with 83+ widgets, "
                            "dark/light theme, DPI auto-scaling, DX11/OpenGL/Vulkan backends, "
                            "and full CJK font support.");
        });
        window->AddPanel(welcome);

        // ── Auto-wrap panel ──────────────────────────────────────────────
        auto wrapPanel = std::make_shared<unigui::Panel>("wrap", "Auto-Wrap Demo");
        wrapPanel->SetWrapEnabled(true);
        wrapPanel->SetContentCallback([]() {
            im::TextUnformatted("This panel has auto-wrap enabled. Long text will automatically "
                                "wrap to fit the panel width. No need to manually insert line "
                                "breaks or call TextWrapped.");
        });
        window->AddPanel(wrapPanel);

        // ── i18n panel ───────────────────────────────────────────────────
        auto i18nPanel = std::make_shared<unigui::Panel>("i18n", "i18n Test");
        i18nPanel->SetContentCallback([]() {
            im::TextUnformatted("English: Hello, world! This is a Dear ImGui C++ wrapper library.");
            im::Separator();
            im::TextUnformatted("Chinese: 你好，世界！这是一个 Dear ImGui C++ 封装库。");
            im::TextUnformatted("Japanese: こんにちは世界！これはImGuiのラッパーです。");
            im::TextUnformatted("Korean: 안녕하세요 세계! 이것은 ImGui 래퍼입니다.");
            im::Separator();
            im::TextUnformatted("Arabic: مرحبا بالعالم! هذا هو مغلف ImGui.");
            im::TextUnformatted("Thai: สวัสดีชาวโลก! นี่คือไลบรารี ImGui.");
            im::Separator();
            im::TextUnformatted("Emoji: 🎉 🚀 ✨ 💻 🎨 (emoji support test)");
        });
        window->AddPanel(i18nPanel);

        // ── i18n / MRU panel ─────────────────────────────────────────────
        auto localePanel = std::make_shared<unigui::Panel>("locale", "i18n Settings");
        localePanel->SetContentCallback([]() {
            static int curLocale = 0;
            const char* locales[] = {"en_US", "zh_CN", "ja_JP"};
            if (im::Combo("Locale", &curLocale, {"English", "Chinese", "Japanese"}))
                unigui::Locale::SetCurrent(locales[curLocale]);
            im::Text(std::format("App Title: {}", unigui::Locale::Tr("app.title")));
            im::Text(std::format("File: {} | Edit: {} | Help: {}", unigui::Locale::Tr("menu.file"),
                                 unigui::Locale::Tr("menu.edit"), unigui::Locale::Tr("menu.help")));
            im::Separator();
            im::Text(std::format("OK: {} | Cancel: {} | Close: {}", unigui::Locale::Tr("btn.ok"),
                                 unigui::Locale::Tr("btn.cancel"),
                                 unigui::Locale::Tr("btn.close")));
            im::Separator();
            if (im::Button("Add this demo to MRU"))
                unigui::Settings::Instance().AddRecentFile("hello_unigui.exe");
            auto recent = unigui::Settings::Instance().GetRecentFiles();
            if (!recent.empty()) {
                im::Text("Recent files:");
                for (auto& f : recent)
                    im::BulletText(f);
            }
        });
        window->AddPanel(localePanel);

        // ── Fluent API + immediate-mode demo panel (v3.4 / usability) ─────
        auto fluentPanel = std::make_shared<unigui::Panel>("fluent", "Fluent + Immediate API");
        fluentPanel->SetContentCallback([]() {
            namespace im = unigui::im;
            static bool dirty = true;
            im::Checkbox("Dirty (enables Save button)", &dirty);
            im::Separator();

            // Retained-mode Button configured via the CRTP fluent chain — base
            // helpers (WithTooltip/WithEnabled) and Button-specific helpers
            // (WithPrimary) compose because the chain stays Button&.
            static auto saveBtn = std::make_shared<unigui::Button>("save", "Save");
            saveBtn->WithTooltip("Ctrl+S — Save the current file")
                .WithEnabled(dirty)
                .WithPrimary()
                .WithShadow(true, 6.f, 2.f, 2.f);
            saveBtn->Render();

            im::SameLine();

            // Immediate-mode button — no shared_ptr / name / Render() needed.
            {
                unigui::DisabledScope d{!dirty};
                if (im::Button("Discard", im::ButtonVariant::Danger))
                    dirty = false;
            }

            im::TextDisabled("Save uses retained-mode fluent API; "
                             "Discard uses unigui::im + DisabledScope.");
        });
        window->AddPanel(fluentPanel);

        built = true;
    }
    window->Render();
    // ShowDemoWindow LAST — popups get top input priority
    ImGui::ShowDemoWindow();
}

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "Hello UniGUI v" UNIGUI_VERSION_STRING;
#ifdef _WIN32
    config.backend = unigui::BackendType::DX11;
#endif

    // v3.4: RunApp = Init + loop + Shutdown in one call.
    // Setup that must happen after Init goes in a lambda that runs once on the
    // first frame (safe because the ImGui context is fully ready at that point).
    return unigui::RunApp(
        config,
        [&] {
            static bool setup_done = false;
            if (!setup_done) {
                unigui::Locale::LoadBuiltin();
                unigui::Settings::Instance().EnableAutoSave("unigui_settings.ini");
                setup_done = true;
            }
            BuildDemoWindow();
        },
        max_frames);
}
