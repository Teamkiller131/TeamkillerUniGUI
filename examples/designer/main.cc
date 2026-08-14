// designer — the live DSL/CSS preview + code-emission tool (P2 · L).
//
// The designer is the "emit code" counterpart of theme_editor (which is the
// theme-authoring half): pick a built-in DSL scene, watch it render live,
// hot-reload a CSS stylesheet on top of it (--css path, same engine as
// theme_editor), and read the C++ builder expression for the scene — the
// `dsl::ToSource` output — ready to copy into an app.
//
// In-app scene editing (increment 2): scenes can now be written as TEXT —
// the indentation-based scene format parsed by `dsl::ParseScene` (see
// docs/DSL.md) — typed into the built-in editor pane and applied live, or
// loaded from disk with `--scene file.dsl` and hot-reloaded as the file
// changes. A parse error is reported inline (line + message) and never
// crashes the app.
//
// Headless-friendly: `--frames N` renders N frames and exits.
//
// Note: this example draws with the DSL + im layers only (per the wrapper
// contract, no raw ImGui:: calls).

#include <unigui/dsl/dsl_scene.h>
#include <unigui/unigui.h>

#include <format>

#ifdef UNIGUI_HAS_STYLING
#include <unigui/styling/style_engine.h>
#endif

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace im = unigui::im;
namespace dsl = unigui::dsl;
namespace fs = std::filesystem;

namespace {

struct Scene {
    std::string name;
    dsl::NodePtr tree;
    std::string source; // scene text (empty for C++-authored built-ins)
    bool editable = false;
};

// One scene per row of the selector. Stateful nodes hold their state inside
// the tree (persists across frames); sliders/checkboxes are interactive so the
// preview is live, not a screenshot.
std::vector<Scene> BuildScenes() {
    using namespace unigui::dsl;
    return {
        {"Settings form",
         Window("Settings",
                VBox({
                    TextWrapped("Designer scene: a small settings form. Every control below is "
                                "live - click, type and drag it."),
                    Separator(),
                    HBox({CheckBox("Wireframe"), CheckBox("Vsync")}),
                    SliderFloat("Opacity", 0.0f, 1.0f),
                    SliderFloat("Scale", 0.25f, 2.0f),
                    InputText("Nickname"),
                    Separator(),
                    HBox({dsl::Button("Save", ButtonVariant::Primary), dsl::Button("Discard"),
                          dsl::Button("Reset", ButtonVariant::Danger)}),
                }))},
        {"Dashboard",
         Window("Dashboard", VBox({
                                 HBox({Text("Portfolio"), TextDisabled("(live)")}),
                                 Separator(),
                                 Flex({Label("Equity"), Label("Bonds"), Label("Cash")},
                                      {2.0f, 1.0f, 1.0f}, 8.0f, unigui::layout::FlexJustify::Start),
                                 SliderFloat("Target risk", 0.0f, 1.0f),
                                 For(3,
                                     [](int i) {
                                         return HBox({Text(std::format("Row {}", i + 1)),
                                                      dsl::Button("Open", ButtonVariant::Primary)});
                                     }),
                                 Separator(),
                                 dsl::Button("Refresh"),
                             }))},
        {"Playground",
         Window("Playground",
                VBox({
                    Text("Structure playground:"),
                    For(2, [](int i) { return BulletText(std::format("bullet {}", i)); }),
                    IfElse([] { return true; }, TextDisabled("if-branch"),
                           TextDisabled("else-branch")),
                    Spacing(),
                    Custom([] {
                        im::Text("Custom escape hatch: arbitrary im:: code.");
                        im::SameLine();
                        im::TextDisabled("(not recoverable by ToSource)");
                    }),
                }))},
    };
}

bool LoadFileText(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f)
        return false;
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

} // namespace

int main(int argc, char** argv) {
    int maxFrames = 0;
    std::string cssPath;
    std::string scenePath;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            maxFrames = std::atoi(argv[++i]);
        else if (a == "--css" && i + 1 < argc)
            cssPath = argv[++i];
        else if (a == "--scene" && i + 1 < argc)
            scenePath = argv[++i];
    }

    unigui::AppConfig cfg;
    cfg.width = 1280;
    cfg.height = 800;
    cfg.title = "UniGUI - Designer (DSL/CSS preview + code emission)";
    if (!unigui::Init(cfg))
        return 1;

    auto scenes = BuildScenes();
    int selected = 0;
    std::string emitted;
    auto regenerate = [&] { emitted = dsl::ToSource(scenes[selected].tree); };
    regenerate();

    // ── Scene-file support (--scene): load once, hot-reload on mtime change ────
    std::size_t fileSceneIndex = 0;
    bool hasFileScene = false;
    fs::file_time_type sceneMtime;
    std::string sceneStatus;
    auto loadSceneFile = [&](bool initial) {
        std::string text;
        if (!LoadFileText(scenePath, text)) {
            if (initial)
                sceneStatus = "--scene: cannot read " + scenePath;
            return;
        }
        const auto parsed = dsl::ParseScene(text);
        if (!parsed.tree) {
            sceneStatus = std::format("--scene parse error: {}", parsed.error);
            return;
        }
        if (!hasFileScene) {
            scenes.push_back({fs::path(scenePath).stem().string(), parsed.tree, text, true});
            fileSceneIndex = scenes.size() - 1;
            hasFileScene = true;
            selected = static_cast<int>(fileSceneIndex);
            regenerate();
        } else {
            scenes[fileSceneIndex].tree = parsed.tree;
            scenes[fileSceneIndex].source = text;
            if (selected == static_cast<int>(fileSceneIndex))
                regenerate();
        }
        sceneStatus = std::format("{}: loaded (hot-reload on)", scenePath);
    };
    if (!scenePath.empty()) {
        loadSceneFile(true);
        std::error_code ec;
        sceneMtime = fs::last_write_time(scenePath, ec);
    }

    // ── In-app scene-text editor state ─────────────────────────────────────────
    std::string editBuf;
    std::string parseError;
    auto syncEditor = [&] {
        if (scenes[selected].editable)
            editBuf = scenes[selected].source;
        else
            editBuf = "# built-in scene (C++ builders) - not editable as text\n";
        parseError.clear();
    };
    syncEditor();

#ifdef UNIGUI_HAS_STYLING
    int cssReloads = 0;
    if (!cssPath.empty())
        unigui::styling::Engine::Instance().LoadFile(cssPath);
#endif

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

#ifdef UNIGUI_HAS_STYLING
        if (!cssPath.empty() && unigui::styling::Engine::Instance().ReloadIfChanged()) {
            ++cssReloads;
            unigui::styling::Engine::Instance().ApplyAll();
        }
#endif
        // Scene-file hot-reload: poll the file's mtime once per frame.
        if (hasFileScene) {
            std::error_code ec;
            const auto mtime = fs::last_write_time(scenePath, ec);
            if (!ec && mtime != sceneMtime) {
                sceneMtime = mtime;
                loadSceneFile(false);
                if (selected == static_cast<int>(fileSceneIndex))
                    syncEditor();
            }
        }

        im::SetNextWindowPos(ImVec2(0, 0));
        im::SetNextWindowSize(im::GetIO().DisplaySize);
        {
            unigui::WindowScope window{"Designer", nullptr,
                                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse};

            // ── Scene selector + tooling column ──────────────────────────────
            im::BeginChild("toolbar", ImVec2(300, 0), ImGuiChildFlags_Borders);
            im::SeparatorText("Scene");
            for (std::size_t i = 0; i < scenes.size(); ++i) {
                if (im::RadioButton(scenes[i].name, &selected, static_cast<int>(i))) {
                    regenerate();
                    syncEditor();
                }
            }
            if (!sceneStatus.empty())
                im::TextDisabled(sceneStatus);

            im::SeparatorText("Scene source (in-app editor)");
            im::InputTextMultiline("##scene_src", &editBuf, ImVec2(0, 120));
            if (im::Button("Apply")) {
                const auto parsed = dsl::ParseScene(editBuf);
                if (parsed.tree) {
                    scenes[selected].tree = parsed.tree;
                    scenes[selected].source = editBuf;
                    scenes[selected].editable = true;
                    parseError.clear();
                    regenerate();
                } else {
                    parseError = parsed.error;
                }
            }
            if (!parseError.empty())
                im::TextColored(ImVec4(1.0f, 0.35f, 0.30f, 1.0f), parseError);

            im::SeparatorText("Emit code");
            im::TextWrapped("The C++ builder expression for the selected scene "
                            "(dsl::ToSource):");
            if (im::Button("Copy to clipboard"))
                im::SetClipboardText(emitted);

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

            // ── Live preview ─────────────────────────────────────────────────
            im::SameLine();
            im::BeginChild("preview", ImVec2(0, 0), ImGuiChildFlags_Borders);
            im::SeparatorText("Live preview");
            dsl::Render(scenes[selected].tree);
            im::EndChild();

            // ── Emitted code ─────────────────────────────────────────────────
            im::SameLine();
            im::BeginChild("code", ImVec2(0, 0), ImGuiChildFlags_Borders);
            im::SeparatorText("Emitted code");
            im::InputTextMultiline("##src", &emitted, ImVec2(0, 0), ImGuiInputTextFlags_ReadOnly);
            im::EndChild();
        }
        unigui::Render();

        if (maxFrames > 0 && ++frame >= maxFrames)
            break;
    }

    unigui::Shutdown();
    return 0;
}
