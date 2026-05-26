/// Example plugin: demonstrates the v2 plugin interface.
/// Build: cmake --build && the DLL goes to the build output.
#include <unigui/v2/plugin_interface.h>
#include <unigui/core/log.h>
#include <imgui.h>
#include <cstdio>

class ExamplePlugin : public unigui::v2::IPlugin {
public:
    unigui::v2::PluginInfo GetInfo() const override {
        return {"ExamplePlugin", "1.0.0", "UniGUI Team", "Demo plugin for v2 plugin system"};
    }
    bool Init() override {
        std::printf("[ExamplePlugin] Initialized\n");
        return true;
    }
    void Shutdown() override {
        std::printf("[ExamplePlugin] Shutdown\n");
    }
    void Render() override {
        ImGui::Begin("Example Plugin");
        ImGui::Text("Hello from ExamplePlugin v1.0.0!");
        ImGui::End();
    }
};

// Required exports
extern "C" {
    __declspec(dllexport) unigui::v2::IPlugin* CreatePlugin() { return new ExamplePlugin(); }
    __declspec(dllexport) void DestroyPlugin(unigui::v2::IPlugin* p) { delete p; }
}
