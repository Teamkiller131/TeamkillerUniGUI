#pragma once
#include <imgui.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace unigui::styling {

struct StyleRule {
    std::string selector;    // "Window", "Button.primary", Button:hover", "#submit"
    std::string type;        // Widget type name
    std::string className;   // ".primary"
    std::string idName;      // "#submit"
    std::string pseudoClass; // ":hover" or empty
    std::unordered_map<std::string, std::string> props;
    int priority() const; // 0=type, 1=class, 2=id
};

struct MediaRule {
    std::string condition; // "min-width: 800px" or "prefers-color-scheme: dark"
    std::vector<StyleRule> rules;
};

class Engine {
public:
    static Engine& Instance();

    /// Load CSS from a file and remember it for hot-reloading. Returns the
    /// number of rules parsed.
    int LoadFile(const std::string& path);
    /// Re-parse the file last passed to LoadFile() if it changed on disk since
    /// the last load/reload. Clears previously-parsed rules and variables first
    /// (single-stylesheet hot-reload — the typical "edit the .css and see it
    /// update" dev workflow). Returns true if a reload actually happened.
    bool ReloadIfChanged();
    /// Path currently watched for hot-reload (empty if LoadFile was never used).
    const std::string& WatchedFile() const { return watchedPath_; }
    /// Drop all parsed rules, media rules, and variables (keeps the watched path).
    void Clear();
    /// Parse CSS from a string. Returns number of rules parsed.
    int Parse(const std::string& css);

    /// Apply matching rules to the ImGui style system.
    void Apply(const std::string& widgetType, const std::string& className = "",
               const std::string& idName = "", bool hovered = false, bool active = false,
               bool focused = false, bool disabled = false, int childIndex = -1);

    /// Apply all loaded rules globally.
    void ApplyAll();

    /// Get/set a CSS variable.
    void SetVar(const std::string& name, const std::string& value);
    std::string GetVar(const std::string& name) const;

    /// Evaluate @media rules against current viewport/view preferences.
    void EvaluateMedia(float viewWidth, float viewHeight, bool darkMode = true);

    /// Public for the context registry (src/detail/context_registry.h) — prefer
    /// Instance(); direct construction bypasses the per-context lifetime.
    Engine() = default;

private:
    std::vector<StyleRule> rules_;
    std::vector<MediaRule> mediaRules_;
    std::unordered_map<std::string, std::string> vars_;
    std::string watchedPath_;    // file passed to LoadFile(), for hot-reload
    long long watchedMtime_ = 0; // last-seen mtime tick of watchedPath_
    void ParseRule(const std::string& block);
    void ParseSelector(StyleRule& rule, const std::string& sel);
    void ApplyRule(const StyleRule& rule);
};

} // namespace unigui::styling
