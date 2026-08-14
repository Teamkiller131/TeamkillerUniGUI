#pragma once
#include <imgui.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace unigui::fonts {

struct FontEntry {
    std::string name;
    ImFont* font = nullptr;
    void* data = nullptr; // owned memory (AddFontFromMemoryTTF)
    int dataSize = 0;
    std::vector<std::string> fallbacks; // fallback font names
};

class Manager {
public:
    static Manager& Instance();

    /// Load a font from a TTF file. Returns ImFont* or nullptr.
    ImFont* Load(const std::string& name, const std::string& path, float size);

    /// Load a font from memory. Returns ImFont* or nullptr.
    ImFont* LoadFromMemory(const std::string& name, const void* data, int size, float fontSize);

    /// Get a loaded font by name. Returns nullptr if not found.
    ImFont* Get(const std::string& name) const;

    /// Unload a font by name. Returns true if found.
    bool Unload(const std::string& name);

    /// Set the default font for ImGui.
    void SetDefault(const std::string& name);

    /// Add a fallback chain: if glyph missing in 'name', try 'fallbackName'.
    void SetFallback(const std::string& name, const std::string& fallbackName);

    /// List all loaded font names.
    std::vector<std::string> List() const;

    /// Push/Pop font for current ImGui scope.
    void Push(const std::string& name);
    void Pop();

    /// Build the font atlas. Must be called after all fonts are loaded and
    /// BEFORE any text rendering.  **WARNING**: Rebuilding the atlas
    /// invalidates ALL previously obtained ImFont* pointers.  To preserve
    /// them, call Get() again after Build() — the Manager re-caches pointers
    /// from the new atlas.  Alternatively, load ALL fonts first, then Build()
    /// once — this avoids pointer churn.
    void Build();

    /// Load the system emoji font and merge it into the default font as a glyph
    /// fallback. size=0 uses the current default font size. Platform-aware: probes
    /// "Segoe UI Emoji" under %WINDIR% on Windows, "Apple Color Emoji" on macOS, and
    /// the common "Noto Color Emoji" install locations across Linux distros. A no-op
    /// on the web build (no system fonts); registered as "emoji" (see Get()) on
    /// success. Idempotent.
    void LoadSystemEmoji(float size = 0);

    /// Load a system CJK (Chinese/Japanese/Korean) font and merge it into the default
    /// font as a glyph fallback — covers CJK ideographs, kana, Hangul, CJK punctuation
    /// and full-width forms. size=0 uses the current default font size. Platform-aware:
    /// probes Microsoft YaHei/SimSun under %WINDIR% on Windows, PingFang/Hiragino on
    /// macOS, and Noto Sans CJK/WenQuanYi locations across Linux distros. Returns the
    /// merged font, or nullptr when no candidate exists (always on the web build — load
    /// a CJK font explicitly there). Registered as "cjk"; idempotent.
    ImFont* LoadSystemCJK(float size = 0);

    /// Public for the context registry (src/detail/context_registry.h) — prefer
    /// Instance(); direct construction bypasses the per-context lifetime.
    Manager() = default;

private:
    /// Probe candidates in order, merge the first hit into the default font over
    /// `ranges`, register it as `regName`. Returns the existing entry when already
    /// loaded, nullptr when nothing was found.
    ImFont* loadSystemMerged(const std::string& regName, const std::vector<std::string>& candidates,
                             float size, const ImWchar* ranges);
    std::unordered_map<std::string, FontEntry> fonts_;
};

} // namespace unigui::fonts
