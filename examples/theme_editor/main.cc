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

#ifdef UNIGUI_HAS_STYLING
#include <unigui/styling/style_engine.h>
#endif

#include <cstdio>
#include <fstream>
#include <string>

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

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Theme Editor", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // ── Controls column ───────────────────────────────────────────────
        ImGui::BeginChild("controls", ImVec2(320, 0), ImGuiChildFlags_Borders);
        ImGui::SeparatorText("Palette");

        int preset = static_cast<int>(theme.preset);
        if (ImGui::RadioButton("Dark", &preset, 0) || ImGui::RadioButton("Light", &preset, 1)) {
            theme.preset = static_cast<unigui::ThemePreset>(preset);
            applyTheme();
        }

        ImGui::SeparatorText("Surface material");
        for (std::size_t i = 0; i < surfaces.size(); ++i) {
            int sel = static_cast<int>(theme.surface);
            if (ImGui::RadioButton(unigui::theme::SurfaceStyleName(surfaces[i]),
                                   sel == static_cast<int>(surfaces[i]))) {
                theme.surface = surfaces[i];
                applyTheme();
            }
        }

        ImGui::SeparatorText("Typography");
        if (ImGui::SliderFloat("Font px", &theme.font_size, 10.0f, 28.0f, "%.0f"))
            applyTheme(); // takes effect on the next font rebuild

        ImGui::SeparatorText("Import / Export");
        if (ImGui::Button("Export JSON")) {
            lastExport = unigui::ExportThemeJSON();
            std::ofstream(kExportFile) << lastExport;
        }
        ImGui::SameLine();
        if (ImGui::Button("Import JSON")) {
            std::ifstream f(kExportFile);
            if (f) {
                std::string json((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
                unigui::ImportThemeJSON(json);
            }
        }
        ImGui::TextWrapped("Export size: %zu bytes", lastExport.size());

#ifdef UNIGUI_HAS_STYLING
        ImGui::SeparatorText("CSS hot-reload");
        if (cssPath.empty()) {
            ImGui::TextDisabled("Run with --css <file> to live-edit CSS.");
        } else {
            ImGui::Text("Watching: %s", cssPath.c_str());
            ImGui::Text("Reloads: %d", cssReloads);
            ImGui::TextDisabled("Edit the file on disk to see it apply.");
        }
#endif
        ImGui::EndChild();

        // ── Live preview column ───────────────────────────────────────────
        ImGui::SameLine();
        ImGui::BeginChild("preview", ImVec2(0, 0), ImGuiChildFlags_Borders);
        ImGui::SeparatorText("Live preview");
        unigui::im::Button("Primary", unigui::im::ButtonVariant::Primary);
        ImGui::SameLine();
        unigui::im::Button("Danger", unigui::im::ButtonVariant::Danger);
        ImGui::SameLine();
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
        ImGui::EndChild();

        ImGui::End();
        unigui::Render();

        if (maxFrames > 0 && ++frame >= maxFrames)
            break;
    }

    unigui::Shutdown();
    return 0;
}
