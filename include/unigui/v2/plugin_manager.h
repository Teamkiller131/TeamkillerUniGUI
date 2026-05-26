#pragma once
#include <unigui/v2/plugin_interface.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace unigui::v2 {

struct LoadedPlugin {
    std::string path, name;
    void* handle = nullptr;
    IPlugin* raw = nullptr;
    std::unique_ptr<IPlugin, std::function<void(IPlugin*)>> ptr; // RAII cleanup
};

/// Manages plugin lifecycle: load, unload, reload, list.
class PluginManager {
public:
    static PluginManager& Instance();

    /// Load a plugin DLL. Returns nullptr on failure.
    IPlugin* Load(const std::string& path);

    /// Unload a plugin by name. Returns true if found.
    bool Unload(const std::string& name);

    /// Reload a plugin (unload + load). Returns new instance or nullptr.
    IPlugin* Reload(const std::string& name);

    /// List all loaded plugin names.
    std::vector<std::string> List() const;

    /// Find a loaded plugin by name. Returns nullptr if not found.
    IPlugin* Get(const std::string& name) const;

    /// Register a static (built-in) plugin. Manager takes ownership.
    IPlugin* Register(IPlugin* plugin);

    /// Set hot-reload callback: called when a plugin is reloaded.
    void SetOnReload(std::function<void(const std::string& name)> cb) { onReload_ = std::move(cb); }

    /// Shutdown all plugins and free all handles.
    void Shutdown();

private:
    PluginManager() = default;
    void* LoadLibraryOS(const std::string& path);
    void FreeLibraryOS(void* handle);
    void* GetSymbolOS(void* handle, const char* name);

    std::vector<LoadedPlugin> plugins_;
    std::function<void(const std::string&)> onReload_;
};

} // namespace unigui::v2
