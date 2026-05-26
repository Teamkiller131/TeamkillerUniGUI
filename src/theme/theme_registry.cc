#include <unigui/theme/presets/registry.h>
#include <unigui/core/log.h>

// All built-in theme headers
#include <unigui/theme/presets/material.h>
#include <unigui/theme/presets/fluent.h>
#include <unigui/theme/presets/dracula.h>
#include <unigui/theme/presets/nord.h>
#include <unigui/theme/presets/gruvbox.h>
#include <unigui/theme/presets/catppuccin.h>
#include <unigui/theme/presets/solarized.h>
#include <unigui/theme/presets/tokyonight.h>
#include <unigui/theme/presets/onedark.h>
#include <unigui/theme/presets/everforest.h>

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
    for (auto& [k, _] : presets_) names.push_back(k);
    return names;
}

bool ThemeRegistry::Apply(const std::string& name) {
    auto* p = Get(name);
    if (!p) { UNIGUI_LOG_WARN("Theme not found: {}", name); return false; }
    p->apply(ImGui::GetStyle());
    current_ = name;
    if (onChange_) onChange_(name);
    UNIGUI_LOG_INFO("Theme applied: {}", name);
    return true;
}

void RegisterAllThemes() {
    auto& r = ThemeRegistry::Instance();
    r.Register({"Material Dark",     "Google Material Design 3 dark",  ApplyMaterialDark});
    r.Register({"Material Light",    "Google Material Design 3 light", ApplyMaterialLight});
    r.Register({"Fluent Dark",       "Microsoft Fluent 2 dark",         ApplyFluentDark});
    r.Register({"Fluent Light",      "Microsoft Fluent 2 light",        ApplyFluentLight});
    r.Register({"Dracula",           "Dark purple theme for coders",    ApplyDracula});
    r.Register({"Nord",              "Frosty arctic dark blues",        ApplyNord});
    r.Register({"Gruvbox",           "Retro groove dark theme",         ApplyGruvbox});
    r.Register({"Catppuccin Mocha",  "Pastel dark with lavender accent", ApplyCatppuccinMocha});
    r.Register({"Solarized Dark",   "Precision dark theme",             ApplySolarizedDark});
    r.Register({"Solarized Light",  "Precision light theme",            ApplySolarizedLight});
    r.Register({"TokyoNight",        "Cyberpunk-inspired dark theme",   ApplyTokyoNight});
    r.Register({"OneDark",           "Atom editor dark theme",          ApplyOneDark});
    r.Register({"Everforest",        "Calm forest green dark theme",    ApplyEverforest});
}

} // namespace unigui::theme
