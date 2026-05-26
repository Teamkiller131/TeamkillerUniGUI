#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace unigui::v2 {

struct PluginInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
};

/// Base class for all v2 plugins. Must be implemented by plugin DLLs.
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginInfo GetInfo() const = 0;
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float dt) {}  // optional per-frame update
    virtual void Render()  {}         // optional per-frame render
};

/// Factory function type. Each plugin DLL exports CreatePlugin/ DestroyPlugin.
using CreatePluginFn = IPlugin*(*)();
using DestroyPluginFn = void(*)(IPlugin*);

} // namespace unigui::v2
