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

    /// Load the system emoji font and set it as a fallback of the default font.
    /// size=0 uses the current default font size. On Windows loads
    /// "Segoe UI Emoji", on macOS "Apple Color Emoji", on Linux attempts
    /// "Noto Color Emoji".
    void LoadSystemEmoji(float size = 0);

private:
    Manager() = default;
    std::unordered_map<std::string, FontEntry> fonts_;
};

} // namespace unigui::fonts
