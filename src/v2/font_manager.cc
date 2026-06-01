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

void Manager::LoadSystemEmoji(float size) {
    auto& io = ImGui::GetIO();

    if (size <= 0.f) {
        ImFont* def = io.FontDefault;
        size = def ? ImGui::GetFontSize() : 16.0f;
    }

    // Emoji glyph ranges: misc symbols, dingbats, emoticons, pictographs
    static const ImWchar emoji_ranges[] = {
        0x2600, 0x26FF,   // Misc symbols
        0x2700, 0x27BF,   // Dingbats
        0x1F300, 0x1F5FF, // Misc Symbols and Pictographs
        0x1F600, 0x1F64F, // Emoticons
        0x1F680, 0x1F6FF, // Transport and Map
        0x1F900, 0x1F9FF, // Supplemental Symbols and Pictographs
        0x1FA00, 0x1FA6F, // Chess Symbols
        0x1FA70, 0x1FAFF, // Symbols and Pictographs Extended-A
        0,
    };

#ifdef _WIN32
    const char* emoji_path = "C:/Windows/Fonts/seguiemj.ttf";
#elif defined(__APPLE__)
    const char* emoji_path = "/System/Library/Fonts/Apple Color Emoji.ttc";
#else
    const char* emoji_path = "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf";
#endif

    // Try primary path; fallback for Linux
    FILE* fp = fopen(emoji_path, "rb");
#ifdef __linux__
    if (!fp) {
        emoji_path = "/usr/share/fonts/noto/NotoColorEmoji.ttf";
        fp = fopen(emoji_path, "rb");
    }
#endif
    if (!fp) {
        UNIGUI_LOG_WARN("Emoji font not found at {}", emoji_path);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0) { fclose(fp); return; }

    void* data = IM_ALLOC(sz);
    fread(data, 1, sz, fp);
    fclose(fp);

    ImFontConfig cfg;
    cfg.MergeMode = true;
    cfg.FontDataOwnedByAtlas = true;

    ImFont* font = io.Fonts->AddFontFromMemoryTTF(data, (int)sz, size, &cfg, emoji_ranges);
    if (font) {
        FontEntry e;
        e.name = "emoji";
        e.font = font;
        e.data = data;
        e.dataSize = (int)sz;
        fonts_["emoji"] = e;
        UNIGUI_LOG_INFO("Emoji font loaded: {} ({}px)", emoji_path, (int)size);
    } else {
        IM_FREE(data);
        UNIGUI_LOG_WARN("Failed to load emoji font from {}", emoji_path);
    }
}

} // namespace unigui::fonts
