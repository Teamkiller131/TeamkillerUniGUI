#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <imgui.h>

namespace unigui::styling {

struct StyleRule {
    std::string selector;     // "Window", "Button.primary", Button:hover", "#submit"
    std::string type;         // Widget type name
    std::string className;    // ".primary"
    std::string idName;       // "#submit"
    std::string pseudoClass;  // ":hover" or empty
    std::unordered_map<std::string, std::string> props;
    int priority() const;     // 0=type, 1=class, 2=id
};

class Engine {
public:
    static Engine& Instance();

    /// Load CSS from a file. Returns number of rules parsed.
    int LoadFile(const std::string& path);
    /// Parse CSS from a string. Returns number of rules parsed.
    int Parse(const std::string& css);

    /// Apply matching rules to the ImGui style system.
    void Apply(const std::string& widgetType, const std::string& className = "",
               const std::string& idName = "", bool hovered = false,
               bool active = false, bool focused = false, bool disabled = false,
               int childIndex = -1);

    /// Apply all loaded rules globally.
    void ApplyAll();

    /// Get/set a CSS variable.
    void SetVar(const std::string& name, const std::string& value);
    std::string GetVar(const std::string& name) const;

private:
    Engine() = default;
    std::vector<StyleRule> rules_;
    std::unordered_map<std::string, std::string> vars_;
    void ParseRule(const std::string& block);
    void ParseSelector(StyleRule& rule, const std::string& sel);
    void ApplyRule(const StyleRule& rule);
};

} // namespace unigui::styling
