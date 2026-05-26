#include <unigui/fonts/font_manager.h>
#include <unigui/core/log.h>
#include <cstdio>
#include <imgui.h>

namespace unigui::fonts {

Manager& Manager::Instance() { static Manager fm; return fm; }

ImFont* Manager::Load(const std::string& name, const std::string& path, float size) {
    if (fonts_.count(name)) { UNIGUI_LOG_WARN("Font '{}' already loaded", name); return fonts_[name].font; }

    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) { UNIGUI_LOG_WARN("Font file not found: {}", path); return nullptr; }
    fseek(fp, 0, SEEK_END); long sz = ftell(fp); fseek(fp, 0, SEEK_SET);
    if (sz <= 0) { fclose(fp); return nullptr; }

    void* data = IM_ALLOC(sz);
    fread(data, 1, sz, fp); fclose(fp);

    ImFontConfig cfg; cfg.FontDataOwnedByAtlas = true;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, (int)sz, size, &cfg);
    if (!font) { IM_FREE(data); UNIGUI_LOG_ERROR("Failed to load font: {}", name); return nullptr; }

    FontEntry e; e.name = name; e.font = font; e.data = data; e.dataSize = (int)sz;
    fonts_[name] = e;
    UNIGUI_LOG_INFO("Font loaded: {} ({}px) from {}", name, (int)size, path);
    return font;
}

ImFont* Manager::LoadFromMemory(const std::string& name, const void* data, int size, float fontSize) {
    if (fonts_.count(name)) { UNIGUI_LOG_WARN("Font '{}' already loaded", name); return fonts_[name].font; }
    ImFontConfig cfg; cfg.FontDataOwnedByAtlas = false;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)data, size, fontSize, &cfg);
    if (!font) { UNIGUI_LOG_ERROR("Failed to load font from memory: {}", name); return nullptr; }
    FontEntry e; e.name = name; e.font = font;
    fonts_[name] = e;
    UNIGUI_LOG_INFO("Font loaded from memory: {} ({}px)", name, (int)fontSize);
    return font;
}

ImFont* Manager::Get(const std::string& name) const {
    auto it = fonts_.find(name); return it != fonts_.end() ? it->second.font : nullptr;
}

bool Manager::Unload(const std::string& name) {
    auto it = fonts_.find(name);
    if (it == fonts_.end()) return false;
    if (it->second.data) IM_FREE(it->second.data);
    fonts_.erase(it);
    UNIGUI_LOG_INFO("Font unloaded: {}", name);
    return true;
}

void Manager::SetDefault(const std::string& name) {
    auto* f = Get(name);
    if (f) ImGui::GetIO().FontDefault = f;
}

void Manager::SetFallback(const std::string& name, const std::string& fallbackName) {
    auto it = fonts_.find(name);
    if (it != fonts_.end()) it->second.fallbacks.push_back(fallbackName);
}

std::vector<std::string> Manager::List() const {
    std::vector<std::string> ns;
    for (auto& [k,_] : fonts_) ns.push_back(k);
    return ns;
}

void Manager::Push(const std::string& name) {
    if (auto* f = Get(name)) ImGui::PushFont(f);
}
void Manager::Pop() { ImGui::PopFont(); }

void Manager::Build() { ImGui::GetIO().Fonts->Build(); }

} // namespace unigui::fonts
