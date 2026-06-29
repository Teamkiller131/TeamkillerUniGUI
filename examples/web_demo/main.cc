// UniGUI on the Web (WebAssembly + WebGL2 / WebGPU).
//
// A self-contained showcase of the UniGUI framework running in the browser: the
// retained-mode Window/Panel widgets, the fluent retained-mode Button, the immediate
// `unigui::im` helpers, and the embedded JetBrains Mono Nerd Font (CJK + emoji). It
// builds to a .html via Emscripten and hands the frame loop to the browser through
// emscripten_set_main_loop (inside unigui::Run). Open the generated web_demo.html
// through a local web server (WebGL/WebGPU need http(s), not file://).
#include <unigui/unigui.h>

#include <memory>

static void DrawUI() {
    namespace im = unigui::im;
    static auto window = std::make_shared<unigui::Window>("web", "UniGUI on the Web");
    static bool built = false;
    static bool show_imgui_demo = false;
    if (!built) {
        window->SetPosition(40, 40);
        window->SetSize(560, 640);

        // ── Intro ────────────────────────────────────────────────────────────
        auto intro = std::make_shared<unigui::Panel>("intro", "WebAssembly + WebGL2 / WebGPU");
        intro->SetContentCallback([] {
            ImGui::TextWrapped("This is UniGUI v" UNIGUI_VERSION_STRING
                               " compiled to WebAssembly. The same C++23 retained-mode "
                               "widgets, theme engine, and immediate-mode helpers that run "
                               "on the desktop render here in the browser — through WebGL2 "
                               "(GLES3) or WebGPU.");
        });
        window->AddPanel(intro);

        // ── Widgets (fluent retained-mode + immediate `im`) ──────────────────
        auto widgets = std::make_shared<unigui::Panel>("widgets", "Widgets");
        widgets->SetContentCallback([] {
            static bool enabled = true;
            static float value = 0.5f;
            static int choice = 0;
            static char name[64] = "edit me";

            // Retained-mode Button via the CRTP fluent chain (stays Button&).
            static auto primary = std::make_shared<unigui::Button>("save", "Primary Button");
            primary->WithTooltip("A retained-mode unigui::Button configured fluently")
                .WithEnabled(enabled)
                .WithPrimary()
                .WithShadow(true, 6.f, 2.f, 2.f);
            primary->Render();
            im::SameLine();
            // Immediate-mode buttons — no shared_ptr / Render() needed.
            if (im::Button("Danger", im::ButtonVariant::Danger))
                value = 0.0f;
            im::SameLine();
            if (im::Button("Success", im::ButtonVariant::Success))
                value = 1.0f;

            im::Separator();
            im::Checkbox("Enable the primary button", &enabled);
            ImGui::SliderFloat("Slider", &value, 0.0f, 1.0f);
            ImGui::InputText("Text input", name, sizeof(name));
            ImGui::Combo("Combo", &choice, "WebGL2\0WebGPU\0Desktop\0");
            ImGui::ProgressBar(value, ImVec2(-1, 0));
            im::TextDisabled("Fluent retained-mode + immediate unigui::im, same as desktop.");
        });
        window->AddPanel(widgets);

        // ── International text (the embedded Nerd Font: CJK + emoji) ──────────
        auto i18n = std::make_shared<unigui::Panel>("i18n", "Unicode / CJK font");
        i18n->SetContentCallback([] {
            ImGui::TextUnformatted("English: Hello, world!");
            ImGui::TextUnformatted("中文：你好，世界！");
            ImGui::TextUnformatted("日本語：こんにちは世界！");
            ImGui::TextUnformatted("한국어: 안녕하세요 세계!");
            ImGui::TextUnformatted("Emoji: 🎉 🚀 ✨ 💻 🎨");
        });
        window->AddPanel(i18n);

        // ── Interop ──────────────────────────────────────────────────────────
        auto interop = std::make_shared<unigui::Panel>("interop", "Dear ImGui interop");
        interop->SetContentCallback([] {
            ImGui::TextWrapped("UniGUI is a layer over Dear ImGui, so raw ImGui calls work "
                               "anywhere too.");
            im::Checkbox("Show the stock Dear ImGui demo window", &show_imgui_demo);
        });
        window->AddPanel(interop);

        built = true;
    }
    window->Render();
    if (show_imgui_demo)
        ImGui::ShowDemoWindow(&show_imgui_demo);
}

int main() {
    unigui::AppConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "UniGUI Web Demo";
#if defined(__EMSCRIPTEN__) && defined(UNIGUI_HAS_WEBGPU)
    // GLFW (NO_API) canvas + WebGPU renderer (built with -DUNIGUI_WEB_WEBGPU=ON).
    config.backend = unigui::BackendType::WebGPU;
#elif defined(__EMSCRIPTEN__)
    // emscripten platform (emscripten's GLFW) + OpenGL3/WebGL2 renderer.
    config.backend = unigui::BackendType::Emscripten;
#endif

    // On the web Run() never returns — the browser owns the loop (emscripten_set_main_loop),
    // so maxFrames is ignored and there is no post-loop Shutdown.
    return unigui::RunApp(config, [] { DrawUI(); }, /*maxFrames=*/0);
}
