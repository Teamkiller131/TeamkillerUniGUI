// theme_editor — a live theme-authoring playground (Horizon 5).
//
// Demonstrates the toolkit's theming-authoring building blocks together:
//   * switch ThemePreset (Dark/Light), SurfaceStyle, font size and accent live;
//   * export the active ImGui palette to JSON and re-import it
//     (ExportThemeJSON / ImportThemeJSON);
//   * hot-reload a CSS stylesheet from disk — pass `--css path/to/style.css`
//     and edit the file while the app runs (styling::Engine::ReloadIfChanged()).
//
// Headless-friendly: `--frames N` renders N frames and exits.

#include <unigui/unigui.h>

#include <format>

#ifdef UNIGUI_HAS_STYLING
#include <unigui/styling/style_engine.h>
#endif

#include <cstdio>
#include <fstream>
#include <string>

namespace im = unigui::im;

int main(int argc, char** argv) {
    int maxFrames = 0;
    std::string cssPath;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            maxFrames = std::atoi(argv[++i]);
        else if (a == "--css" && i + 1 < argc)
            cssPath = argv[++i];
    }

    unigui::AppConfig cfg;
    cfg.width = 900;
    cfg.height = 600;
    cfg.title = "UniGUI — Theme Editor";
    if (!unigui::Init(cfg))
        return 1;

    unigui::ThemeConfig theme;
    auto applyTheme = [&] { unigui::ApplyTheme(theme); };
    applyTheme();

#ifdef UNIGUI_HAS_STYLING
    int cssReloads = 0;
    if (!cssPath.empty())
        unigui::styling::Engine::Instance().LoadFile(cssPath);
#endif

    const auto& surfaces = unigui::theme::AllSurfaceStyles();
    const char* kExportFile = "theme_export.json";
    std::string lastExport;

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

#ifdef UNIGUI_HAS_STYLING
        // Poll the watched stylesheet once per frame — the heart of hot-reload.
        if (!cssPath.empty() && unigui::styling::Engine::Instance().ReloadIfChanged()) {
            ++cssReloads;
            unigui::styling::Engine::Instance().ApplyAll();
        }
#endif

        im::SetNextWindowPos(ImVec2(0, 0));
        im::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        {
            unigui::WindowScope window{"Theme Editor", nullptr,
                                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse};

            // ── Controls column ───────────────────────────────────────────────
            im::BeginChild("controls", ImVec2(320, 0), ImGuiChildFlags_Borders);
            im::SeparatorText("Palette");

            int preset = static_cast<int>(theme.preset);
            if (im::RadioButton("Dark", &preset, 0) || im::RadioButton("Light", &preset, 1)) {
                theme.preset = static_cast<unigui::ThemePreset>(preset);
                applyTheme();
            }

            im::SeparatorText("Surface material");
            for (std::size_t i = 0; i < surfaces.size(); ++i) {
                int sel = static_cast<int>(theme.surface);
                if (im::RadioButton(unigui::theme::SurfaceStyleName(surfaces[i]),
                                    sel == static_cast<int>(surfaces[i]))) {
                    theme.surface = surfaces[i];
                    applyTheme();
                }
            }

            im::SeparatorText("Typography");
            if (im::SliderFloat("Font px", &theme.font_size, 10.0f, 28.0f, "%.0f"))
                applyTheme(); // takes effect on the next font rebuild

            im::SeparatorText("Import / Export");
            if (im::Button("Export JSON")) {
                lastExport = unigui::ExportThemeJSON();
                std::ofstream(kExportFile) << lastExport;
            }
            im::SameLine();
            if (im::Button("Import JSON")) {
                std::ifstream f(kExportFile);
                if (f) {
                    std::string json((std::istreambuf_iterator<char>(f)),
                                     std::istreambuf_iterator<char>());
                    unigui::ImportThemeJSON(json);
                }
            }
            im::TextWrapped(std::format("Export size: {} bytes", lastExport.size()));

#ifdef UNIGUI_HAS_STYLING
            im::SeparatorText("CSS hot-reload");
            if (cssPath.empty()) {
                im::TextDisabled("Run with --css <file> to live-edit CSS.");
            } else {
                im::Text(std::format("Watching: {}", cssPath));
                im::Text(std::format("Reloads: {}", cssReloads));
                im::TextDisabled("Edit the file on disk to see it apply.");
            }
#endif
            im::EndChild();

            // ── Live preview column ───────────────────────────────────────────
            im::SameLine();
            im::BeginChild("preview", ImVec2(0, 0), ImGuiChildFlags_Borders);
            im::SeparatorText("Live preview");
            unigui::im::Button("Primary", unigui::im::ButtonVariant::Primary);
            im::SameLine();
            unigui::im::Button("Danger", unigui::im::ButtonVariant::Danger);
            im::SameLine();
            unigui::im::Button("Success", unigui::im::ButtonVariant::Success);

            static float slider = 0.5f;
            unigui::im::SliderFloat("Slider", &slider, 0.f, 1.f);
            static bool toggle = true;
            unigui::im::Checkbox("Checkbox", &toggle);

            float pct = static_cast<float>(frame % 120) / 120.0f;
            unigui::im::ProgressBar(pct);

            if (unigui::im::BeginTabBar("preview_tabs")) {
                if (unigui::im::BeginTabItem("Text")) {
                    unigui::im::TextWrapped("The quick brown fox jumps over the lazy dog.");
                    unigui::im::TextDisabled("Disabled text sample.");
                    unigui::im::EndTabItem();
                }
                if (unigui::im::BeginTabItem("Tree")) {
                    if (unigui::im::TreeNode("Root")) {
                        unigui::im::Text("Child A");
                        unigui::im::Text("Child B");
                        unigui::im::TreePop();
                    }
                    unigui::im::EndTabItem();
                }
                unigui::im::EndTabBar();
            }
            im::EndChild();
        }
        unigui::Render();

        if (maxFrames > 0 && ++frame >= maxFrames)
            break;
    }

    unigui::Shutdown();
    return 0;
}
