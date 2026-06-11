#include <unigui/core/log.h>
#include <unigui/theme/color_tokens.h>
#include <unigui/theme/presets/registry.h>

// All built-in theme headers
#include <unigui/theme/presets/catppuccin.h>
#include <unigui/theme/presets/dracula.h>
#include <unigui/theme/presets/everforest.h>
#include <unigui/theme/presets/fluent.h>
#include <unigui/theme/presets/gruvbox.h>
#include <unigui/theme/presets/material.h>
#include <unigui/theme/presets/nord.h>
#include <unigui/theme/presets/onedark.h>
#include <unigui/theme/presets/solarized.h>
#include <unigui/theme/presets/tokyonight.h>

namespace unigui::theme {

ThemeRegistry& ThemeRegistry::Instance() {
    static ThemeRegistry r;
    return r;
}

void ThemeRegistry::Register(ThemePreset preset) {
    presets_[preset.name] = std::move(preset);
}

const ThemePreset* ThemeRegistry::Get(const std::string& name) const {
    auto it = presets_.find(name);
    return it != presets_.end() ? &it->second : nullptr;
}

std::vector<std::string> ThemeRegistry::List() const {
    std::vector<std::string> names;
    for (auto& [k, _] : presets_)
        names.push_back(k);
    return names;
}

bool ThemeRegistry::Apply(const std::string& name) {
    auto* p = Get(name);
    if (!p) {
        UNIGUI_LOG_WARN("Theme not found: {}", name);
        return false;
    }
    p->apply(ImGui::GetStyle());
    // Auto-derived table colors — follow the new theme's palette
    auto& c = ImGui::GetStyle().Colors;
    c[ImGuiCol_TableHeaderBg] = ImVec4(c[ImGuiCol_FrameBg].x * 0.7f, c[ImGuiCol_FrameBg].y * 0.7f,
                                       c[ImGuiCol_FrameBg].z * 0.75f, 1.00f);
    c[ImGuiCol_TableBorderStrong] = ImVec4(c[ImGuiCol_Border].x * 0.9f, c[ImGuiCol_Border].y * 0.9f,
                                           c[ImGuiCol_Border].z * 0.95f, 1.00f);
    c[ImGuiCol_TableBorderLight] = ImVec4(c[ImGuiCol_Border].x * 0.7f, c[ImGuiCol_Border].y * 0.7f,
                                          c[ImGuiCol_Border].z * 0.75f, 1.00f);
    c[ImGuiCol_TableRowBg] =
        ImVec4(c[ImGuiCol_WindowBg].x, c[ImGuiCol_WindowBg].y, c[ImGuiCol_WindowBg].z, 1.00f);
    c[ImGuiCol_TableRowBgAlt] =
        ImVec4(c[ImGuiCol_WindowBg].x * 1.04f, c[ImGuiCol_WindowBg].y * 1.04f,
               c[ImGuiCol_WindowBg].z * 1.06f, 1.00f);
    // Accent & semantic colour tokens (Step 3) — give every preset the same
    // accent→hover→active relationship (derived from its own accent) and update
    // the active semantic palette for widgets.
    auto& s = ImGui::GetStyle();
    ApplyColorTokens(s, AccentFromStyle(s), StyleIsDark(s));
    current_ = name;
    if (onChange_)
        onChange_(name);
    UNIGUI_LOG_INFO("Theme applied: {}", name);
    return true;
}

void RegisterAllThemes() {
    auto& r = ThemeRegistry::Instance();
    r.Register({"Material Dark", "Google Material Design 3 dark", ApplyMaterialDark});
    r.Register({"Material Light", "Google Material Design 3 light", ApplyMaterialLight});
    r.Register({"Fluent Dark", "Microsoft Fluent 2 dark", ApplyFluentDark});
    r.Register({"Fluent Light", "Microsoft Fluent 2 light", ApplyFluentLight});
    r.Register({"Dracula", "Dark purple theme for coders", ApplyDracula});
    r.Register({"Nord", "Frosty arctic dark blues", ApplyNord});
    r.Register({"Gruvbox", "Retro groove dark theme", ApplyGruvbox});
    r.Register({"Catppuccin Mocha", "Pastel dark with lavender accent", ApplyCatppuccinMocha});
    r.Register({"Solarized Dark", "Precision dark theme", ApplySolarizedDark});
    r.Register({"Solarized Light", "Precision light theme", ApplySolarizedLight});
    r.Register({"TokyoNight", "Cyberpunk-inspired dark theme", ApplyTokyoNight});
    r.Register({"OneDark", "Atom editor dark theme", ApplyOneDark});
    r.Register({"Everforest", "Calm forest green dark theme", ApplyEverforest});
}

} // namespace unigui::theme
