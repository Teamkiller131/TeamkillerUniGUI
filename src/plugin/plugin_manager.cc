#include <unigui/core/log.h>
#include <unigui/plugin/plugin_manager.h>

#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace unigui::plugin {

Manager& Manager::Instance() {
    static Manager pm;
    return pm;
}

static void* LoadLibOS(const std::string& path) {
#ifdef _WIN32
    return (void*) LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_LAZY);
#endif
}
static void FreeLibOS(void* h) {
    if (!h)
        return;
#ifdef _WIN32
    FreeLibrary((HMODULE) h);
#else
    dlclose(h);
#endif
}
static void* GetSymOS(void* h, const char* n) {
#ifdef _WIN32
    return (void*) GetProcAddress((HMODULE) h, n);
#else
    return dlsym(h, n);
#endif
}

IPlugin* Manager::Load(const std::string& path) {
    for (auto& p : plugins_)
        if (p.path == path) {
            UNIGUI_LOG_WARN("Plugin already loaded: {}", path);
            return p.raw;
        }

    void* h = LoadLibOS(path);
    if (!h) {
        UNIGUI_LOG_ERROR("Failed to load: {}", path);
        return nullptr;
    }

    auto cf = (CreatePluginFn) GetSymOS(h, "CreatePlugin");
    auto df = (DestroyPluginFn) GetSymOS(h, "DestroyPlugin");
    if (!cf || !df) {
        UNIGUI_LOG_ERROR("Missing exports: {}", path);
        FreeLibOS(h);
        return nullptr;
    }

    IPlugin* raw = cf();
    if (!raw) {
        UNIGUI_LOG_ERROR("CreatePlugin returned null: {}", path);
        FreeLibOS(h);
        return nullptr;
    }
    if (!raw->Init()) {
        UNIGUI_LOG_ERROR("Init failed: {}", path);
        df(raw);
        FreeLibOS(h);
        return nullptr;
    }

    auto info = raw->GetInfo();
    LoadedPlugin lp;
    lp.path = path;
    lp.name = info.name;
    lp.handle = h;
    lp.raw = raw;
    lp.ptr = std::unique_ptr<IPlugin, std::function<void(IPlugin*)>>(raw, [df, h](IPlugin* p) {
        df(p);
        FreeLibOS(h);
    });
    UNIGUI_LOG_INFO("Plugin loaded: {} ({})", info.name, path);
    plugins_.push_back(std::move(lp));
    return plugins_.back().raw;
}

bool Manager::Unload(const std::string& name) {
    auto it =
        std::find_if(plugins_.begin(), plugins_.end(), [&](auto& p) { return p.name == name; });
    if (it == plugins_.end())
        return false;
    it->raw->Shutdown();
    UNIGUI_LOG_INFO("Plugin unloaded: {}", name);
    plugins_.erase(it);
    return true;
}

IPlugin* Manager::Reload(const std::string& name) {
    std::string path;
    for (auto& p : plugins_)
        if (p.name == name) {
            path = p.path;
            break;
        }
    if (path.empty())
        return nullptr;
    Unload(name);
    auto* p = Load(path);
    if (p && onReload_)
        onReload_(name);
    return p;
}

std::vector<std::string> Manager::List() const {
    std::vector<std::string> ns;
    for (auto& p : plugins_)
        ns.push_back(p.name);
    return ns;
}

IPlugin* Manager::Get(const std::string& name) const {
    for (auto& p : plugins_)
        if (p.name == name)
            return p.raw;
    return nullptr;
}

IPlugin* Manager::Register(IPlugin* plugin) {
    auto info = plugin->GetInfo();
    if (!plugin->Init()) {
        UNIGUI_LOG_ERROR("Init failed: {}", info.name);
        delete plugin;
        return nullptr;
    }
    LoadedPlugin lp;
    lp.name = info.name;
    lp.raw = plugin;
    lp.ptr = std::unique_ptr<IPlugin, std::function<void(IPlugin*)>>(plugin,
                                                                     [](IPlugin* p) { delete p; });
    UNIGUI_LOG_INFO("Built-in plugin registered: {}", info.name);
    plugins_.push_back(std::move(lp));
    return plugins_.back().raw;
}

void Manager::Shutdown() {
    for (auto& p : plugins_)
        if (p.raw)
            p.raw->Shutdown();
    plugins_.clear();
}

} // namespace unigui::plugin
