#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace unigui::plugin {

/// ABI version of the plugin interface. Bump on ANY ABI-breaking change (adding /
/// removing / reordering virtuals, changing the exported symbol set); plugins built
/// against a different version are rejected at load time. The C++ interface is
/// compiler-ABI-sensitive, so the version gate is the contract: within a version the
/// interface is frozen, and only additive changes at the END of IPlugin keep it valid.
/// See docs/MODULES.md §Plugins and docs/API_STABILITY.md.
inline constexpr std::int32_t kPluginInterfaceVersion = 1;

struct PluginInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
};

/// Base class for all plugins. Must be implemented by plugin DLLs.
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginInfo GetInfo() const = 0;
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Update([[maybe_unused]] float dt) {} // optional per-frame update
    virtual void Render() {}         // optional per-frame render
};

/// Factory function types. Each plugin DLL exports CreatePlugin/DestroyPlugin.
using CreatePluginFn = IPlugin* (*) ();
using DestroyPluginFn = void (*)(IPlugin*);
/// Version-report export. Every plugin DLL exports PluginInterfaceVersion so the
/// manager can reject ABI-mismatched plugins before instantiating them.
using InterfaceVersionFn = std::int32_t (*) ();

/// True when a plugin reporting @p reported can be hosted (exact match today; a
/// compatibility range would live here if the ABI ever grows additively).
inline constexpr bool InterfaceVersionCompatible(std::int32_t reported) {
    return reported == kPluginInterfaceVersion;
}

} // namespace unigui::plugin
