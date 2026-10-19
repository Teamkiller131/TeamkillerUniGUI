#include <unigui/core/log.h>
#include <unigui/fonts/font_manager.h>

#include "../detail/context_registry.h"

#include <imgui.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#if defined(UNIGUI_HAS_EMBEDDED_CJK)
// Generated at build time by cmake/FontEmbed.cmake from
// fonts/UniGUISansSC-OFL-Subset-Regular.ttf (Noto Sans SC subset, SIL OFL 1.1).
// Exposes `unigui_cjk_data` (bytes) and `unigui_cjk_size` (bytes).
#include <unigui_cjk_font.h>
#endif

namespace unigui::fonts {

namespace {

// Synthetic "path" for the compiled-in OFL CJK subset. It shows up in probe
// plans, diagnostics and logs so a resolution is always attributable.
constexpr const char* kEmbeddedCJKId = kEmbeddedCJKAssetId;

// Well-known CJK file names searched inside caller-provided directories.
const char* const kKnownCJKFileNames[] = {
    "UniGUISansSC-OFL-Subset-Regular.ttf", // this repo's embedded asset
    "NotoSansSC-Regular.ttf",              // Google Fonts / Noto release
    "NotoSansSC-Regular.otf",
    "NotoSansCJKsc-Regular.otf",
    "NotoSansCJK-Regular.ttc",
    "SourceHanSansSC-Regular.otf",
    "SourceHanSansCN-Regular.otf",
};

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

bool FileExists(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return false;
    fclose(fp);
    return true;
}

// Heuristic used for caller-injected search paths: a trailing separator or the
// absence of a file extension means "directory to scan", otherwise "font file".
bool LooksLikeDirectory(const std::string& path) {
    if (path.empty())
        return false;
    if (path.back() == '/' || path.back() == '\\')
        return true;
    const auto slash = path.find_last_of("/\\");
    const auto dot = path.find_last_of('.');
    return dot == std::string::npos || (slash != std::string::npos && dot < slash);
}

std::string JoinPath(std::string dir, const char* file) {
    while (!dir.empty() && (dir.back() == '/' || dir.back() == '\\'))
        dir.pop_back();
    dir += '/';
    dir += file;
    return dir;
}

std::string EnvOrEmpty(const char* name) {
    const char* value = std::getenv(name);
    return (value && *value) ? std::string(value) : std::string();
}

std::string JoinList(const std::vector<std::string>& items) {
    std::string out;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i)
            out += ", ";
        out += items[i];
    }
    return out;
}

bool HasEmbeddedCJK() {
#ifdef UNIGUI_HAS_EMBEDDED_CJK
    return true;
#else
    return false;
#endif
}

// Compiled-in OFL subset bytes (nullptr when the build has no embedded asset).
const void* EmbeddedCJKData(int& sizeOut) {
#ifdef UNIGUI_HAS_EMBEDDED_CJK
    sizeOut = static_cast<int>(unigui_cjk_size);
    return unigui_cjk_data;
#else
    sizeOut = 0;
    return nullptr;
#endif
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
    // Legacy Microsoft fonts are the historical default; Noto/Source Han cover
    // machines where those are absent. None of them is required — the embedded
    // OFL subset is the guaranteed fallback (S0/D37).
    return {dir + "msyh.ttc", dir + "msyh.ttf", dir + "simsun.ttc", dir + "simhei.ttf",
            dir + "NotoSansSC-Regular.ttf", dir + "SourceHanSansSC-Regular.otf"};
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

// Merge `data` into the last font of the atlas as a glyph fallback. `size` = 0
// inherits the destination's reference size; an explicit size is honored only when
// the destination has an explicit reference size (imgui 1.92+ asserts otherwise).
ImFont* MergeIntoDefault(void* data, int dataSize, bool dataOwnedByAtlas, float size,
                         ImFontConfig& cfg, const ImWchar* ranges) {
    auto& io = ImGui::GetIO();

    // MergeMode needs a font to merge INTO. When called before any font is loaded
    // (empty atlas), seed it with ImGui's default font — the same font NewFrame would
    // add lazily — instead of tripping the "Cannot use MergeMode for the first font"
    // assert.
    if (io.Fonts->Fonts.Size == 0)
        io.Fonts->AddFontDefault();

    ImFont* dst = io.Fonts->Fonts.back();
    const bool dstImplicit = (dst->Flags & ImFontFlags_ImplicitRefSize) != 0;
    if (size > 0.f && dstImplicit) {
        UNIGUI_LOG_DEBUG("Font merge: ignoring explicit size {} (destination font uses "
                         "an implicit reference size)",
                         size);
    }
    const float effSize = (size > 0.f && !dstImplicit) ? size : 0.0f;

    cfg.MergeMode = true;
    cfg.FontDataOwnedByAtlas = dataOwnedByAtlas;
    ImFont* font = io.Fonts->AddFontFromMemoryTTF(data, dataSize, effSize, &cfg, ranges);
    if (font && effSize > 0.f)
        UNIGUI_LOG_DEBUG("Font merge: loaded at {}px", effSize);
    return font;
}

} // namespace

const char* FontSourceName(FontSource source) {
    switch (source) {
    case FontSource::Explicit:
        return "explicit";
    case FontSource::Embedded:
        return "embedded";
    case FontSource::System:
        return "system";
    case FontSource::None:
    default:
        return "none";
    }
}

std::string FontProbe::ToString() const {
    if (found)
        return std::string("found ") + FontSourceName(source) + " font: " + path;
    return error.empty() ? std::string("font not found (no reason recorded)") : error;
}

std::string FontDiagnostics::Summary() const {
    std::string s = "fonts: body=";
    s += bodyLoaded ? "loaded" : "MISSING";
    s += " cjk=";
    if (cjkFound) {
        s += FontSourceName(cjkSource);
        s += ":";
        s += cjkPath;
    } else {
        s += "MISSING";
    }
    s += " emoji=";
    if (emojiFound) {
        s += "system:";
        s += emojiPath;
    } else {
        s += "MISSING";
    }
    s += " [embedded-cjk=";
    s += embeddedCJKAvailable ? "yes" : "no";
    s += " system-probe=";
    s += systemProbeEnabled ? "on" : "off";
    s += "]";
    if (!cjkFound && !cjkError.empty()) {
        s += "; cjk error: ";
        s += cjkError;
    }
    if (!emojiFound && !emojiError.empty()) {
        s += "; emoji error: ";
        s += emojiError;
    }
    if (fallbackOnly)
        s += "; FALLBACK ONLY: no CJK font — CJK text renders as tofu boxes";
    return s;
}

Manager& Manager::Instance() {
    return detail::ContextRegistry<Manager>::Instance();
}

// ── Atlas lifecycle ─────────────────────────────────────────────────────────

void Manager::Reset() {
    // The atlas is being cleared/rebuilt: every cached ImFont* is stale. Data
    // buffers are NOT freed here — the atlas owns them (see Unload()).
    fonts_.clear();
    lastCJK_ = FontProbe{};
    lastEmoji_ = FontProbe{};
}

// ── Injected probe inputs ───────────────────────────────────────────────────

void Manager::SetSearchPaths(std::vector<std::string> paths) {
    searchPaths_ = std::move(paths);
}

void Manager::AddSearchPath(const std::string& path) {
    if (!path.empty())
        searchPaths_.push_back(path);
}

std::vector<std::string> Manager::SearchPaths() const {
    return searchPaths_;
}

void Manager::ClearSearchPaths() {
    searchPaths_.clear();
}

void Manager::SetCJKPath(const std::string& path) {
    cjkPath_ = path;
}

std::string Manager::CJKPath() const {
    return cjkPath_;
}

void Manager::SetEmojiPath(const std::string& path) {
    emojiPath_ = path;
}

std::string Manager::EmojiPath() const {
    return emojiPath_;
}

void Manager::SetSystemProbeEnabled(bool enabled) {
    systemProbe_ = enabled;
}

bool Manager::SystemProbeEnabled() const {
    return systemProbe_;
}

void Manager::SetEmbeddedFallbackEnabled(bool enabled) {
    embeddedFallback_ = enabled;
}

bool Manager::EmbeddedFallbackEnabled() const {
    return embeddedFallback_;
}

bool Manager::HasEmbeddedCJKFont() const {
    return HasEmbeddedCJK();
}

// Ordered candidate list: UNIGUI_CJK_FONT env → explicit override → search paths
// (env dirs appended to that group) → embedded asset → system fonts.
std::vector<std::pair<FontSource, std::string>> Manager::cjkCandidates() const {
    std::vector<std::pair<FontSource, std::string>> candidates;

    const std::string envFont = EnvOrEmpty("UNIGUI_CJK_FONT");
    if (!envFont.empty())
        candidates.emplace_back(FontSource::Explicit, envFont);

    if (!cjkPath_.empty())
        candidates.emplace_back(FontSource::Explicit, cjkPath_);

    std::vector<std::string> dirs = searchPaths_;
    {
        // UNIGUI_FONT_DIRS: ';'-separated directory/file list (deployment override
        // without recompiling). Appended after API-injected search paths.
        const std::string envDirs = EnvOrEmpty("UNIGUI_FONT_DIRS");
        std::size_t start = 0;
        while (start < envDirs.size()) {
            const auto end = envDirs.find(';', start);
            const std::string entry =
                envDirs.substr(start, end == std::string::npos ? std::string::npos : end - start);
            if (!entry.empty())
                dirs.push_back(entry);
            if (end == std::string::npos)
                break;
            start = end + 1;
        }
    }
    for (const auto& entry : dirs) {
        if (entry.empty())
            continue;
        if (LooksLikeDirectory(entry)) {
            for (const char* name : kKnownCJKFileNames)
                candidates.emplace_back(FontSource::Explicit, JoinPath(entry, name));
        } else {
            candidates.emplace_back(FontSource::Explicit, entry);
        }
    }

    if (HasEmbeddedCJKFont() && embeddedFallback_)
        candidates.emplace_back(FontSource::Embedded, kEmbeddedCJKId);

#ifndef __EMSCRIPTEN__
    if (systemProbe_) {
        for (const auto& path : CJKCandidates())
            candidates.emplace_back(FontSource::System, path);
    }
#endif
    return candidates;
}

std::vector<std::pair<FontSource, std::string>> Manager::emojiCandidates() const {
    std::vector<std::pair<FontSource, std::string>> candidates;
    const std::string envFont = EnvOrEmpty("UNIGUI_EMOJI_FONT");
    if (!envFont.empty())
        candidates.emplace_back(FontSource::Explicit, envFont);
    if (!emojiPath_.empty())
        candidates.emplace_back(FontSource::Explicit, emojiPath_);
#ifndef __EMSCRIPTEN__
    if (systemProbe_) {
        for (const auto& path : EmojiCandidates())
            candidates.emplace_back(FontSource::System, path);
    }
#endif
    return candidates;
}

std::vector<std::string> Manager::CJKProbePlan() const {
    std::vector<std::string> plan;
    for (const auto& [source, path] : cjkCandidates())
        plan.push_back(path);
    return plan;
}

FontProbe Manager::resolveCJK() const {
    FontProbe probe;
    for (const auto& [source, path] : cjkCandidates()) {
        probe.probed.push_back(path);
        // The embedded asset is compiled in: its presence is a build fact, not a
        // filesystem lookup.
        if (source == FontSource::Embedded || FileExists(path)) {
            probe.found = true;
            probe.source = source;
            probe.path = path;
            return probe;
        }
    }
    probe.error = "CJK font not found; probed " + std::to_string(probe.probed.size()) +
                  " candidate(s): [" + JoinList(probe.probed) + "]";
    if (!systemProbe_)
        probe.error += " (system font probe disabled)";
    if (!HasEmbeddedCJKFont())
        probe.error += " (this build has no embedded OFL CJK subset)";
    else if (!embeddedFallback_)
        probe.error += " (embedded CJK fallback disabled)";
    return probe;
}

FontProbe Manager::resolveEmoji() const {
    FontProbe probe;
    for (const auto& [source, path] : emojiCandidates()) {
        probe.probed.push_back(path);
        if (FileExists(path)) {
            probe.found = true;
            probe.source = source;
            probe.path = path;
            return probe;
        }
    }
    probe.error = "emoji font not found; probed " + std::to_string(probe.probed.size()) +
                  " candidate(s): [" + JoinList(probe.probed) + "]";
    if (!systemProbe_)
        probe.error += " (system font probe disabled)";
    return probe;
}

FontProbe Manager::ProbeCJK() const {
    return resolveCJK();
}

FontProbe Manager::ProbeEmoji() const {
    return resolveEmoji();
}

FontDiagnostics Manager::Diagnostics() const {
    FontDiagnostics diag;
    diag.embeddedCJKAvailable = HasEmbeddedCJK();
    diag.systemProbeEnabled = systemProbe_;
    diag.bodyLoaded = ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().Fonts != nullptr &&
                      ImGui::GetIO().Fonts->Fonts.Size > 0;

    if (fonts_.find("cjk") != fonts_.end()) {
        diag.cjkFound = true;
        diag.cjkSource = lastCJK_.source;
        diag.cjkPath = lastCJK_.path;
        diag.cjkProbed = lastCJK_.probed;
    } else {
        FontProbe probe = resolveCJK();
        diag.cjkFound = probe.found;
        diag.cjkSource = probe.source;
        diag.cjkPath = probe.path;
        diag.cjkError = probe.error;
        diag.cjkProbed = probe.probed;
    }

    if (fonts_.find("emoji") != fonts_.end()) {
        diag.emojiFound = true;
        diag.emojiPath = lastEmoji_.path;
    } else {
        FontProbe probe = resolveEmoji();
        diag.emojiFound = probe.found;
        diag.emojiPath = probe.path;
        diag.emojiError = probe.error;
    }

    diag.fallbackOnly = diag.bodyLoaded && !diag.cjkFound;
    return diag;
}

// ── Loading ─────────────────────────────────────────────────────────────────

ImFont* Manager::Load(const std::string& name, const std::string& path, float size) {
    return LoadChecked(name, path, size).font;
}

FontLoadResult Manager::LoadChecked(const std::string& name, const std::string& path, float size) {
    FontLoadResult result;
    result.path = path;
    result.source = FontSource::Explicit;

    if (name.empty()) {
        result.error = "font name is empty";
        UNIGUI_LOG_ERROR("Font load failed: {}", result.error);
        return result;
    }
    if (path.empty()) {
        result.error = "font path is empty for '" + name + "'";
        UNIGUI_LOG_ERROR("Font load failed: {}", result.error);
        return result;
    }
    if (auto it = fonts_.find(name); it != fonts_.end()) {
        UNIGUI_LOG_WARN("Font '{}' already loaded ({}); returning the registered instance", name,
                        path);
        result.ok = true;
        result.font = it->second.font;
        return result;
    }

    long sz = 0;
    void* data = ReadFontFile(path, sz);
    if (!data) {
        result.error = "font file not found or unreadable: " + path;
        UNIGUI_LOG_ERROR("Font load failed: {} (requested as '{}')", result.error, name);
        return result;
    }

    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = true;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, static_cast<int>(sz), size,
                                                              &cfg);
    if (!font) {
        IM_FREE(data);
        result.error = "ImGui rejected font data (corrupt or unsupported TTF): " + path;
        UNIGUI_LOG_ERROR("Font load failed: {} (requested as '{}')", result.error, name);
        return result;
    }

    FontEntry e;
    e.name = name;
    e.font = font;
    e.data = data;
    e.dataSize = static_cast<int>(sz);
    fonts_[name] = e;
    result.ok = true;
    result.font = font;
    UNIGUI_LOG_INFO("Font loaded: {} ({}px) from {}", name, static_cast<int>(size), path);
    return result;
}

ImFont* Manager::LoadFromMemory(const std::string& name, const void* data, int size,
                                float fontSize) {
    if (fonts_.count(name)) {
        UNIGUI_LOG_WARN("Font '{}' already loaded", name);
        return fonts_[name].font;
    }
    if (!data || size <= 0) {
        UNIGUI_LOG_ERROR("Font load from memory failed: empty data for '{}'", name);
        return nullptr;
    }
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*) data, size, fontSize, &cfg);
    if (!font) {
        UNIGUI_LOG_ERROR("Font load from memory failed: ImGui rejected data for '{}'", name);
        return nullptr;
    }
    FontEntry e;
    e.name = name;
    e.font = font;
    fonts_[name] = e;
    UNIGUI_LOG_INFO("Font loaded from memory: {} ({}px)", name, static_cast<int>(fontSize));
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

// ── System font fallbacks ───────────────────────────────────────────────────

FontProbe Manager::LoadSystemCJKEx(float size, const ImWchar* ranges) {
    if (auto it = fonts_.find("cjk"); it != fonts_.end()) {
        // Idempotent: already merged into the current atlas.
        if (lastCJK_.found)
            return lastCJK_;
        FontProbe known;
        known.found = true;
        known.source = FontSource::System;
        known.path = "<already merged>";
        return known;
    }

    FontProbe probe = resolveCJK();
    if (!probe.found) {
        lastCJK_ = probe;
        UNIGUI_LOG_ERROR("CJK font unavailable — CJK text will render as tofu boxes: {}",
                         probe.error);
        return probe;
    }

    int dataSize = 0;
    void* data = nullptr;
    bool ownedByAtlas = true;
    if (probe.source == FontSource::Embedded) {
        data = const_cast<void*>(EmbeddedCJKData(dataSize));
        ownedByAtlas = false; // static blob — the atlas must not free it
        if (!data || dataSize <= 0) {
            probe.found = false;
            probe.error = "embedded CJK asset is empty (build misconfiguration)";
            lastCJK_ = probe;
            UNIGUI_LOG_ERROR("CJK font unavailable: {}", probe.error);
            return probe;
        }
    } else {
        long sz = 0;
        data = ReadFontFile(probe.path, sz);
        dataSize = static_cast<int>(sz);
        if (!data) {
            probe.found = false;
            probe.error = "CJK candidate disappeared between probe and load: " + probe.path;
            lastCJK_ = probe;
            UNIGUI_LOG_ERROR("CJK font unavailable: {}", probe.error);
            return probe;
        }
    }

    ImFontConfig cfg;
    ImFont* font = MergeIntoDefault(data, dataSize, ownedByAtlas, size, cfg, ranges);
    if (!font) {
        if (ownedByAtlas)
            IM_FREE(data);
        probe.found = false;
        probe.error = "ImGui rejected the CJK font data: " + probe.path;
        lastCJK_ = probe;
        UNIGUI_LOG_ERROR("CJK font unavailable: {}", probe.error);
        return probe;
    }

    FontEntry e;
    e.name = "cjk";
    e.font = font;
    if (ownedByAtlas) {
        e.data = data;
        e.dataSize = dataSize;
    }
    fonts_["cjk"] = e;
    lastCJK_ = probe;
    UNIGUI_LOG_INFO("CJK font merged: {} [{}]{}", probe.path, FontSourceName(probe.source),
                    probe.source == FontSource::Embedded
                        ? " (embedded OFL subset, no system CJK font required)"
                        : "");
    return probe;
}

ImFont* Manager::LoadSystemCJK(float size) {
    FontProbe probe = LoadSystemCJKEx(size, nullptr);
    return probe.found ? Get("cjk") : nullptr;
}

FontProbe Manager::LoadSystemEmojiEx(float size) {
    if (auto it = fonts_.find("emoji"); it != fonts_.end()) {
        if (lastEmoji_.found)
            return lastEmoji_;
        FontProbe known;
        known.found = true;
        known.path = "<already merged>";
        return known;
    }

    FontProbe probe = resolveEmoji();
    if (!probe.found) {
        lastEmoji_ = probe;
        // Emoji is a cosmetic fallback: WARN (not ERROR) — but never silent.
        UNIGUI_LOG_WARN("Emoji font unavailable (emoji render as boxes): {}", probe.error);
        return probe;
    }

    // Emoji glyph ranges: misc symbols, dingbats, emoticons, pictographs
    static const ImWchar emoji_ranges[] = {
        0x2600, 0x26FF, // Misc symbols
        0x2700, 0x27BF, // Dingbats
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

    long sz = 0;
    void* data = ReadFontFile(probe.path, sz);
    if (!data) {
        probe.found = false;
        probe.error = "emoji candidate disappeared between probe and load: " + probe.path;
        lastEmoji_ = probe;
        UNIGUI_LOG_WARN("Emoji font unavailable: {}", probe.error);
        return probe;
    }

    ImFontConfig cfg;
    ImFont* font =
        MergeIntoDefault(data, static_cast<int>(sz), true, size, cfg, emoji_ranges);
    if (!font) {
        IM_FREE(data);
        probe.found = false;
        probe.error = "ImGui rejected the emoji font data: " + probe.path;
        lastEmoji_ = probe;
        UNIGUI_LOG_WARN("Emoji font unavailable: {}", probe.error);
        return probe;
    }

    FontEntry e;
    e.name = "emoji";
    e.font = font;
    e.data = data;
    e.dataSize = static_cast<int>(sz);
    fonts_["emoji"] = e;
    lastEmoji_ = probe;
    UNIGUI_LOG_INFO("Emoji font merged: {} [{}]", probe.path, FontSourceName(probe.source));
    return probe;
}

void Manager::LoadSystemEmoji(float size) {
    (void) LoadSystemEmojiEx(size);
}

} // namespace unigui::fonts
