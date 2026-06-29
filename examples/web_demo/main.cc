// UniGUI on the Web (WebAssembly + WebGL2).
//
// A minimal demo that builds to a self-contained .html via Emscripten. It selects the
// Emscripten platform backend (emscripten's GLFW + the OpenGL3/WebGL2 renderer) and
// hands the frame loop to the browser through emscripten_set_main_loop (inside
// unigui::Run). Build with the `web_demo` target under an emcmake configure; open the
// generated web_demo.html through a local web server (WebGL needs http(s), not file://).
#include <unigui/unigui.h>

#include <memory>

static void DrawUI() {
    static auto window = std::make_shared<unigui::Window>("web", "UniGUI on the Web");
    static bool built = false;
    if (!built) {
        auto intro = std::make_shared<unigui::Panel>("intro", "WebAssembly + WebGL2");
        intro->SetContentCallback([] {
            ImGui::TextWrapped("This is UniGUI v" UNIGUI_VERSION_STRING
                               " compiled to WebAssembly and rendering through WebGL2 "
                               "(GLES3). The same retained-mode widgets, theme engine, "
                               "and immediate-mode helpers run unchanged in the browser.");
        });
        window->AddPanel(intro);

        auto demo = std::make_shared<unigui::Panel>("controls", "Live Controls");
        demo->SetContentCallback([] {
            namespace im = unigui::im;
            static float v = 0.5f;
            static bool on = true;
            static char buf[64] = "edit me";
            im::Checkbox("Toggle", &on);
            ImGui::SliderFloat("Value", &v, 0.0f, 1.0f);
            ImGui::InputText("Text", buf, sizeof(buf));
            if (im::Button("Primary", im::ButtonVariant::Primary))
                v = 0.5f;
        });
        window->AddPanel(demo);
        built = true;
    }
    window->Render();
    ImGui::ShowDemoWindow();
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
