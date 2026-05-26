#pragma once
#include <imgui.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

namespace unigui::v2 {

struct FontEntry {
    std::string name;
    ImFont* font = nullptr;
    void* data = nullptr;    // owned memory (AddFontFromMemoryTTF)
    int dataSize = 0;
    std::vector<std::string> fallbacks; // fallback font names
};

class FontManager {
public:
    static FontManager& Instance();

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

    /// Build the font atlas (call after all fonts are loaded).
    void Build();

private:
    FontManager() = default;
    std::unordered_map<std::string, FontEntry> fonts_;
};

} // namespace unigui::v2
