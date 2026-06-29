// UniGUI on the Web (WebAssembly + WebGL2 / WebGPU).
//
// A self-contained, single-window showcase of the UniGUI framework running in the
// browser: the unified dark theme, the fluent retained-mode Button, and the immediate
// `unigui::im` helpers. It builds to a .html via Emscripten and hands the frame loop to
// the browser through emscripten_set_main_loop (inside unigui::Run). Open the generated
// web_demo.html through a local web server (WebGL/WebGPU need http(s), not file://).
#include <unigui/unigui.h>

#include <memory>

static void DrawUI() {
    namespace im = unigui::im;
    static bool show_imgui_demo = false;

    ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(680, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("UniGUI on the Web")) {
        ImGui::TextWrapped("This is UniGUI v" UNIGUI_VERSION_STRING
                           " compiled to WebAssembly. The same C++23 retained-mode widgets, "
                           "theme engine, and immediate-mode helpers that run on the desktop "
                           "render here in the browser — through WebGL2 (GLES3) or WebGPU.");

        // ── Fluent retained-mode + immediate `im` widgets ─────────────────────
        ImGui::SeparatorText("Widgets");
        static bool enabled = true;
        static float value = 0.5f;
        static int choice = 0;
        static char name[64] = "edit me";

        // Retained-mode Button via the CRTP fluent chain (the chain stays Button&, so
        // base helpers and Button-specific helpers compose).
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
        ImGui::Combo("Renderer", &choice, "WebGL2\0WebGPU\0Desktop\0");
        ImGui::ProgressBar(value, ImVec2(-1.0f, 0.0f));
        im::TextDisabled("Fluent retained-mode Button + immediate unigui::im, same as desktop.");

        // ── Theme ─────────────────────────────────────────────────────────────
        ImGui::SeparatorText("Theme");
        ImGui::TextWrapped("The dark + Glass surface styling above is UniGUI's built-in theme "
                           "engine (one of 13 presets), rendering identically in the browser.");

        // ── Fonts note (honest about the web build) ───────────────────────────
        ImGui::SeparatorText("Fonts");
        ImGui::TextWrapped("The web build embeds only the Latin JetBrains Mono Nerd Font and has "
                           "no access to system fonts, so CJK / emoji show as missing glyphs here "
                           "— load a CJK font via the font manager to render them (the desktop "
                           "build merges system CJK ranges automatically).");

        // ── Dear ImGui interop ────────────────────────────────────────────────
        ImGui::SeparatorText("Dear ImGui interop");
        ImGui::TextWrapped("UniGUI is a thin layer over Dear ImGui, so raw ImGui calls work too.");
        im::Checkbox("Show the stock Dear ImGui demo window", &show_imgui_demo);
    }
    ImGui::End();

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
