#include <unigui/core/log.h>
#include <unigui/fonts/font_manager.h>

#include "../detail/context_registry.h"

#include <imgui.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace unigui::fonts {

namespace {

// Read a whole file into an IM_ALLOC'd buffer (the atlas takes ownership on success).
// Returns nullptr if the file is missing/empty; size is returned through szOut.
void* ReadFontFile(const std::string& path, long& szOut) {
    szOut = 0;
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return nullptr;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(fp);
        return nullptr;
    }
    void* data = IM_ALLOC(sz);
    fread(data, 1, sz, fp);
    fclose(fp);
    szOut = sz;
    return data;
}

#ifndef __EMSCRIPTEN__
// Candidate system-font locations per platform. Lists (not single paths) because the
// install location varies by distro/release; the first file that exists wins.
#ifdef _WIN32
// %WINDIR% honors relocated Windows installs; fall back to the conventional root.
std::string WinFontsDir() {
    const char* windir = std::getenv("WINDIR");
    if (!windir || !*windir)
        windir = "C:/Windows";
    return std::string(windir) + "/Fonts/";
}
std::vector<std::string> EmojiCandidates() {
    return {WinFontsDir() + "seguiemj.ttf"};
}
std::vector<std::string> CJKCandidates() {
    const std::string dir = WinFontsDir();
    // Microsoft YaHei (SC + kana coverage) first; SimSun as the pre-Vista fallback.
    return {dir + "msyh.ttc", dir + "msyh.ttf", dir + "simsun.ttc"};
}
#elif defined(__APPLE__)
std::vector<std::string> EmojiCandidates() {
    return {"/System/Library/Fonts/Apple Color Emoji.ttc"};
}
std::vector<std::string> CJKCandidates() {
    return {"/System/Library/Fonts/PingFang.ttc", "/System/Library/Fonts/STHeiti Light.ttc",
            "/System/Library/Fonts/Hiragino Sans GB.ttc"};
}
#else
std::vector<std::string> EmojiCandidates() {
    return {
        "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",     // Debian/Ubuntu
        "/usr/share/fonts/noto/NotoColorEmoji.ttf",              // Arch
        "/usr/share/fonts/google-noto-emoji/NotoColorEmoji.ttf", // Fedora
        "/usr/share/fonts/emoji/NotoColorEmoji.ttf",
        "/usr/local/share/fonts/NotoColorEmoji.ttf", // BSD / manual install
    };
}
std::vector<std::string> CJKCandidates() {
    return {
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", // Debian/Ubuntu
        "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",      // Arch
        "/usr/share/fonts/opentype/noto/NotoSansCJKsc-Regular.otf",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc", // WenQuanYi fallback
        "/usr/share/fonts/wenquanyi/wqy-microhei/wqy-microhei.ttc",
    };
}
#endif
#endif // !__EMSCRIPTEN__

} // namespace

Manager& Manager::Instance() {
    return detail::ContextRegistry<Manager>::Instance();
}

ImFont* Manager::Load(const std::string& name, const std::string& path, float size) {
    if (fonts_.count(name)) {
        UNIGUI_LOG_WARN("Font '{}' already loaded", name);
        return fonts_[name].font;
    }

    long sz = 0;
    void* data = ReadFontFile(path, sz);
    if (!data) {
        UNIGUI_LOG_WARN("Font file not found: {}", path);
        return nullptr;
    }

    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = true;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, (int) sz, size, &cfg);
    if (!font) {
        IM_FREE(data);
        UNIGUI_LOG_ERROR("Failed to load font: {}", name);
        return nullptr;
    }

    FontEntry e;
    e.name = name;
    e.font = font;
    e.data = data;
    e.dataSize = (int) sz;
    fonts_[name] = e;
    UNIGUI_LOG_INFO("Font loaded: {} ({}px) from {}", name, (int) size, path);
    return font;
}

ImFont* Manager::LoadFromMemory(const std::string& name, const void* data, int size,
                                float fontSize) {
    if (fonts_.count(name)) {
        UNIGUI_LOG_WARN("Font '{}' already loaded", name);
        return fonts_[name].font;
    }
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*) data, size, fontSize, &cfg);
    if (!font) {
        UNIGUI_LOG_ERROR("Failed to load font from memory: {}", name);
        return nullptr;
    }
    FontEntry e;
    e.name = name;
    e.font = font;
    fonts_[name] = e;
    UNIGUI_LOG_INFO("Font loaded from memory: {} ({}px)", name, (int) fontSize);
    return font;
}

ImFont* Manager::Get(const std::string& name) const {
    auto it = fonts_.find(name);
    return it != fonts_.end() ? it->second.font : nullptr;
}

bool Manager::Unload(const std::string& name) {
    auto it = fonts_.find(name);
    if (it == fonts_.end())
        return false;
    // Do NOT free entry.data here: every file-loaded font is added with
    // FontDataOwnedByAtlas=true, so the atlas frees that buffer at DestroyContext /
    // atlas rebuild — freeing it here too was a double-free (AV in DestroyContext).
    // Unload only forgets the registry entry; the glyphs stay in the atlas until the
    // next rebuild.
    fonts_.erase(it);
    UNIGUI_LOG_INFO("Font unloaded: {}", name);
    return true;
}

void Manager::SetDefault(const std::string& name) {
    auto* f = Get(name);
    if (f)
        ImGui::GetIO().FontDefault = f;
}

void Manager::SetFallback(const std::string& name, const std::string& fallbackName) {
    auto it = fonts_.find(name);
    if (it != fonts_.end())
        it->second.fallbacks.push_back(fallbackName);
}

std::vector<std::string> Manager::List() const {
    std::vector<std::string> ns;
    for (auto& [k, _] : fonts_)
        ns.push_back(k);
    return ns;
}

void Manager::Push(const std::string& name) {
    if (auto* f = Get(name))
        ImGui::PushFont(f);
}
void Manager::Pop() {
    ImGui::PopFont();
}

void Manager::Build() {
    ImGui::GetIO().Fonts->Build();
}

ImFont* Manager::loadSystemMerged(const std::string& regName,
                                  const std::vector<std::string>& candidates, float size,
                                  const ImWchar* ranges) {
    if (auto it = fonts_.find(regName); it != fonts_.end())
        return it->second.font; // idempotent — already merged into the atlas

    auto& io = ImGui::GetIO();

    // MergeMode needs a font to merge INTO. When called before any font is loaded
    // (empty atlas), seed it with ImGui's default font — the same font NewFrame would
    // add lazily — instead of tripping the "Cannot use MergeMode for the first font"
    // assert.
    if (io.Fonts->Fonts.Size == 0)
        io.Fonts->AddFontDefault();

    // Size 0 = inherit the destination font's reference size — the natural choice for
    // a glyph-fallback merge. An explicit caller size is honored only when the
    // destination font itself has an explicit reference size: imgui (1.92+) asserts on
    // an explicit-size merge into an implicit-ref-size font (e.g. AddFontDefault()).
    ImFont* dst = io.Fonts->Fonts.back();
    const bool dstImplicit = (dst->Flags & ImFontFlags_ImplicitRefSize) != 0;
    if (size > 0.f && dstImplicit) {
        UNIGUI_LOG_DEBUG("System {} font: ignoring explicit size {} (destination font "
                         "uses an implicit reference size)",
                         regName, size);
    }
    const float effSize = (size > 0.f && !dstImplicit) ? size : 0.0f;

    // First candidate that exists wins.
    long sz = 0;
    void* data = nullptr;
    const std::string* found = nullptr;
    for (const auto& path : candidates) {
        data = ReadFontFile(path, sz);
        if (data) {
            found = &path;
            break;
        }
    }
    if (!data) {
        UNIGUI_LOG_WARN("System {} font not found ({} locations probed)", regName,
                        (int) candidates.size());
        return nullptr;
    }

    ImFontConfig cfg;
    cfg.MergeMode = true; // merge into the current/default font as a glyph fallback
    cfg.FontDataOwnedByAtlas = true;

    ImFont* font = io.Fonts->AddFontFromMemoryTTF(data, (int) sz, effSize, &cfg, ranges);
    if (!font) {
        IM_FREE(data);
        UNIGUI_LOG_WARN("Failed to load {} font from {}", regName, *found);
        return nullptr;
    }
    FontEntry e;
    e.name = regName;
    e.font = font;
    e.data = data;
    e.dataSize = (int) sz;
    fonts_[regName] = e;
    if (effSize > 0.f)
        UNIGUI_LOG_INFO("System {} font loaded: {} ({}px)", regName, *found, (int) effSize);
    else
        UNIGUI_LOG_INFO("System {} font loaded: {} (inherits default font size)", regName, *found);
    return font;
}

void Manager::LoadSystemEmoji(float size) {
#ifdef __EMSCRIPTEN__
    // The web build has no system fonts (MEMFS is empty), so there is nothing to probe —
    // skip rather than log a misleading "not found at /usr/share/fonts/…" warning. To get
    // emoji/CJK on the web, load a font explicitly via the font manager.
    (void) size;
    UNIGUI_LOG_DEBUG("Emoji font: skipped on the web build (no system fonts)");
#else
    // Emoji glyph ranges: misc symbols, dingbats, emoticons, pictographs
    static const ImWchar emoji_ranges[] = {
        0x2600,  0x26FF, // Misc symbols
        0x2700,  0x27BF, // Dingbats
#ifdef IMGUI_USE_WCHAR32
        0x1F300, 0x1F5FF, // Misc Symbols and Pictographs
        0x1F600, 0x1F64F, // Emoticons
        0x1F680, 0x1F6FF, // Transport and Map
        0x1F900, 0x1F9FF, // Supplemental Symbols and Pictographs
        0x1FA00, 0x1FA6F, // Chess Symbols
        0x1FA70, 0x1FAFF, // Symbols and Pictographs Extended-A
#endif
        0,
    };
    loadSystemMerged("emoji", EmojiCandidates(), size, emoji_ranges);
#endif // !__EMSCRIPTEN__
}

ImFont* Manager::LoadSystemCJK(float size) {
#ifdef __EMSCRIPTEN__
    // Same as LoadSystemEmoji: no system fonts on MEMFS — load a CJK font explicitly
    // (Load/LoadFromMemory) to render Chinese/Japanese/Korean on the web.
    (void) size;
    UNIGUI_LOG_DEBUG("CJK font: skipped on the web build (no system fonts)");
    return nullptr;
#else
    // BMP-only CJK coverage (works with 16-bit ImWchar): punctuation, kana, Hangul,
    // unified ideographs, compatibility ideographs, and full/half-width forms.
    static const ImWchar cjk_ranges[] = {
        0x3000, 0x303F, // CJK symbols & punctuation
        0x3040, 0x309F, // Hiragana
        0x30A0, 0x30FF, // Katakana
        0x31F0, 0x31FF, // Katakana phonetic extensions
        0x4E00, 0x9FFF, // CJK unified ideographs
        0xAC00, 0xD7A3, // Hangul syllables
        0xF900, 0xFAFF, // CJK compatibility ideographs
        0xFF00, 0xFFEF, // Full/half-width forms
        0,
    };
    return loadSystemMerged("cjk", CJKCandidates(), size, cjk_ranges);
#endif // !__EMSCRIPTEN__
}

} // namespace unigui::fonts
