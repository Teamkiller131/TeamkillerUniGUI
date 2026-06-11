/// Example plugin: demonstrates the plugin interface.
/// Build: cmake --build && the DLL goes to the build output.
#include <unigui/core/log.h>
#include <unigui/plugin/plugin_interface.h>

#include <imgui.h>

#include <cstdio>

class ExamplePlugin : public unigui::plugin::IPlugin {
public:
    unigui::plugin::PluginInfo GetInfo() const override {
        return {"ExamplePlugin", "1.0.0", "UniGUI Team", "Demo plugin"};
    }
    bool Init() override {
        std::printf("[ExamplePlugin] Initialized\n");
        return true;
    }
    void Shutdown() override { std::printf("[ExamplePlugin] Shutdown\n"); }
    void Render() override {
        ImGui::Begin("Example Plugin");
        ImGui::Text("Hello from ExamplePlugin v1.0.0!");
        ImGui::End();
    }
};

// Required exports
extern "C" {
__declspec(dllexport) unigui::plugin::IPlugin* CreatePlugin() {
    return new ExamplePlugin();
}
__declspec(dllexport) void DestroyPlugin(unigui::plugin::IPlugin* p) {
    delete p;
}
}
