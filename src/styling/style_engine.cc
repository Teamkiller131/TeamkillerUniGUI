#include <unigui/core/log.h>
#include <unigui/core/strutil.h>
#include <unigui/fx/effect_scope.h>
#include <unigui/styling/style_engine.h>

#include "../detail/context_registry.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace unigui::styling {

// ── Hand-written CSS scanning helpers ────────────────────────────────────────
// These intentionally avoid std::regex. MSVC's std::regex enforces a
// backtracking-complexity governor (libstdc++/libc++ do not), so the greedy
// patterns this parser used to rely on — e.g. ([^{]+)\s*\{([^}]*)\} — threw
// std::regex_error(error_complexity) on long/pathological input. That broke the
// engine's "parsing never throws" contract on Windows. The scanners below are
// linear and allocation-light, so malformed CSS is handled without throwing.
namespace {

// CSS identifier char class — mirrors the old key pattern [\w-].
constexpr bool IsKeyChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
           c == '-';
}

// Strip leading/trailing ASCII spaces and tabs in place (matches the previous
// parser, which trimmed only ' ' and '\t').
void TrimAsciiSpace(std::string& s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    std::size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
        ++i;
    s.erase(0, i);
}

// Parse a declaration body ("key: value; key2: value2 …") into `props`,
// resolving leading-$ variable references via `vars`. Equivalent to the old
// std::regex property scanner ([\w-]+\s*:\s*([^;]+)) but without backtracking.
void ParsePropsInto(std::unordered_map<std::string, std::string>& props, std::string_view body,
                    const std::unordered_map<std::string, std::string>& vars) {
    std::size_t i = 0;
    while (i < body.size()) {
        const std::size_t semi = body.find(';', i);
        const std::size_t end = (semi == std::string_view::npos) ? body.size() : semi;
        std::string_view decl = body.substr(i, end - i);
        i = (semi == std::string_view::npos) ? body.size() : semi + 1;

        // A declaration is "<key> : <value>"; the key is the identifier run that
        // ends just before the first ':'. No ':' or no key → skip the segment.
        const std::size_t colon = decl.find(':');
        if (colon == std::string_view::npos)
            continue;
        std::size_t ke = colon;
        while (ke > 0 && (decl[ke - 1] == ' ' || decl[ke - 1] == '\t'))
            --ke;
        std::size_t ks = ke;
        while (ks > 0 && IsKeyChar(decl[ks - 1]))
            --ks;
        if (ks == ke)
            continue; // empty key

        std::string val(decl.substr(colon + 1));
        TrimAsciiSpace(val);
        if (val.empty())
            continue; // matches old [^;]+ (value required)
        if (val[0] == '$') {
            auto vi = vars.find(val.substr(1));
            if (vi != vars.end())
                val = vi->second;
        }
        props[std::string(decl.substr(ks, ke - ks))] = std::move(val);
    }
}

// Iterate "<selector> { <body> }" blocks in `s`. On each call, advance `pos`
// past the next block and return its full text (selector through closing brace)
// in `full`. Returns false when no further block exists. Mirrors the old
// ([^{]+)\s*\{([^}]*)\} regex: a block needs a non-empty selector run (at least
// one non-'{' char) before the brace and a closing '}'.
bool NextBlock(const std::string& s, std::size_t& pos, std::string& full) {
    while (true) {
        const std::size_t open = s.find('{', pos);
        if (open == std::string::npos)
            return false;
        if (open == pos) {
            // No selector before this brace — the regex could not match here;
            // skip the stray '{' and look for the next candidate.
            pos = open + 1;
            continue;
        }
        const std::size_t close = s.find('}', open + 1);
        if (close == std::string::npos)
            return false; // unterminated block — old regex required a '}'
        full = s.substr(pos, close - pos + 1);
        pos = close + 1;
        return true;
    }
}

} // namespace

Engine& Engine::Instance() {
    return detail::ContextRegistry<Engine>::Instance();
}

int StyleRule::priority() const {
    if (!idName.empty())
        return 2;
    if (!className.empty())
        return 1;
    return 0;
}

void Engine::SetVar(const std::string& name, const std::string& value) {
    vars_[name] = value;
}
std::string Engine::GetVar(const std::string& name) const {
    auto it = vars_.find(name);
    return it != vars_.end() ? it->second : "";
}

// ── CSS Parser ──────────────────────────────────────────────────────────────

void Engine::ParseSelector(StyleRule& rule, const std::string& sel) {
    rule.selector = sel;
    std::string s = sel;
    // Extract pseudo-class :hover
    auto colon = s.find(':');
    if (colon != std::string::npos) {
        rule.pseudoClass = s.substr(colon + 1);
        s = s.substr(0, colon);
    }
    // Extract ID #name
    auto hash = s.find('#');
    if (hash != std::string::npos) {
        rule.idName = s.substr(hash + 1);
        s = s.substr(0, hash);
    }
    // Extract class .name
    auto dot = s.find('.');
    if (dot != std::string::npos) {
        rule.className = s.substr(dot + 1);
        s = s.substr(0, dot);
    }
    // Remaining is type
    rule.type = s;
}

void Engine::ParseRule(const std::string& block) {
    // Split: "Window { bg: #1a1e; rounding: 6; }"
    auto brace = block.find('{');
    if (brace == std::string::npos)
        return;
    std::string sel = block.substr(0, brace);
    while (!sel.empty() && (sel.back() == ' ' || sel.back() == '\t'))
        sel.pop_back();
    while (!sel.empty() && (sel.front() == ' ' || sel.front() == '\t'))
        sel.erase(0, 1);
    if (sel.empty())
        return;

    auto endBrace = block.find('}');
    if (endBrace == std::string::npos)
        return;
    std::string props = block.substr(brace + 1, endBrace - brace - 1);

    StyleRule rule;
    ParseSelector(rule, sel);

    // Parse properties: "bg: #1a1e; rounding: 6;"
    ParsePropsInto(rule.props, props, vars_);

    rules_.push_back(std::move(rule));
}

int Engine::Parse(const std::string& css) {
    int count = 0;

    // Find all rule blocks: "@media (cond) { Rule { ... } }" or "Selector { ... }"
    std::string full;
    std::size_t scan = 0;
    while (NextBlock(css, scan, full)) {
        // @media blocks
        auto mediaPos = full.find("@media");
        if (mediaPos != std::string::npos) {
            // Extract condition: "@media (min-width: 800px) { ... }"
            auto condStart = full.find('(');
            auto condEnd = full.find(')');
            if (condStart != std::string::npos && condEnd != std::string::npos) {
                std::string cond = full.substr(condStart + 1, condEnd - condStart - 1);
                MediaRule mr;
                mr.condition = cond;

                // Parse inner rules (between outer { })
                auto innerStart = full.find('{');
                auto innerEnd = full.rfind('}');
                if (innerStart != std::string::npos && innerEnd != std::string::npos) {
                    std::string inner = full.substr(innerStart + 1, innerEnd - innerStart - 1);
                    std::string innerBlock;
                    std::size_t innerScan = 0;
                    while (NextBlock(inner, innerScan, innerBlock)) {
                        const auto innerBrace = innerBlock.find('{');
                        const auto innerClose = innerBlock.find('}');
                        if (innerBrace == std::string::npos || innerClose == std::string::npos)
                            continue;
                        std::string sel = innerBlock.substr(0, innerBrace);
                        TrimAsciiSpace(sel);
                        StyleRule rule;
                        ParseSelector(rule, sel);
                        ParsePropsInto(rule.props,
                                       std::string_view(innerBlock)
                                           .substr(innerBrace + 1, innerClose - innerBrace - 1),
                                       vars_);
                        mr.rules.push_back(std::move(rule));
                    }
                }
                mediaRules_.push_back(std::move(mr));
                count += (int) mediaRules_.back().rules.size();
            }
        } else {
            ParseRule(full);
            count++;
        }
    }

    UNIGUI_LOG_INFO("CSS: {} rules parsed, {} @media blocks", count, (int) mediaRules_.size());
    return count;
}

void Engine::EvaluateMedia(float viewWidth, float viewHeight, bool darkMode) {
    for (auto& mr : mediaRules_) {
        bool match = false;

        // Parse condition
        if (mr.condition.find("min-width") != std::string::npos) {
            auto colon = mr.condition.find(':');
            if (colon != std::string::npos) {
                float val = ToFloatOr(mr.condition.substr(colon + 1));
                match = (viewWidth >= val);
            }
        } else if (mr.condition.find("max-width") != std::string::npos) {
            auto colon = mr.condition.find(':');
            if (colon != std::string::npos) {
                float val = ToFloatOr(mr.condition.substr(colon + 1));
                match = (viewWidth <= val);
            }
        } else if (mr.condition.find("min-height") != std::string::npos) {
            auto colon = mr.condition.find(':');
            if (colon != std::string::npos) {
                float val = ToFloatOr(mr.condition.substr(colon + 1));
                match = (viewHeight >= val);
            }
        } else if (mr.condition.find("prefers-color-scheme") != std::string::npos) {
            bool wantsDark = mr.condition.find("dark") != std::string::npos;
            match = (darkMode == wantsDark);
        }

        if (match) {
            for (auto& rule : mr.rules)
                ApplyRule(rule);
        }
    }
}

namespace {
// File modification time as an integer tick, or 0 if the file is unavailable.
long long FileMtimeTicks(const std::string& path) {
    std::error_code ec;
    const auto t = std::filesystem::last_write_time(path, ec);
    if (ec)
        return 0;
    return static_cast<long long>(t.time_since_epoch().count());
}
} // namespace

int Engine::LoadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        UNIGUI_LOG_WARN("CSS file not found: {}", path);
        return 0;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    watchedPath_ = path;
    watchedMtime_ = FileMtimeTicks(path);
    return Parse(ss.str());
}

void Engine::Clear() {
    rules_.clear();
    mediaRules_.clear();
    vars_.clear();
}

bool Engine::ReloadIfChanged() {
    if (watchedPath_.empty())
        return false;
    const long long now = FileMtimeTicks(watchedPath_);
    if (now == 0 || now == watchedMtime_)
        return false; // file gone or unchanged
    Clear();
    LoadFile(watchedPath_); // re-parses and refreshes watchedMtime_
    return true;
}

// ── Property Mapping (50+ CSS properties) ────────────────────────────────────

static void ApplyProp(const std::string& key, const std::string& val) {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    // ── Basic colors ────────────────────────────────────────────────────
    // The vast majority of color properties are "parse #RRGGBB, assign one
    // ImGui color slot". A dispatch table keeps that O(1) and avoids the long
    // if-else chain flagged in review; genuinely special properties
    // (multi-slot accent, sizing, effects, gradient) stay below as handlers.
    static const std::unordered_map<std::string_view, ImGuiCol> kColorSlots = {
        {"bg", ImGuiCol_WindowBg},
        {"frame-bg", ImGuiCol_FrameBg},
        {"text", ImGuiCol_Text},
        {"text-disabled", ImGuiCol_TextDisabled},
        {"text-secondary", ImGuiCol_TextDisabled},
        {"border", ImGuiCol_Border},
        {"border-color", ImGuiCol_Border},
        {"border-hover", ImGuiCol_BorderShadow},
        {"button", ImGuiCol_Button},
        {"button-hover", ImGuiCol_ButtonHovered},
        {"button-active", ImGuiCol_ButtonActive},
        {"bg-hover", ImGuiCol_ButtonHovered},
        {"bg-active", ImGuiCol_ButtonActive},
        {"bg-secondary", ImGuiCol_FrameBgHovered},
        {"bg-tertiary", ImGuiCol_FrameBgActive},
        {"header", ImGuiCol_Header},
        {"header-hover", ImGuiCol_HeaderHovered},
        {"header-active", ImGuiCol_HeaderActive},
        {"header-bg", ImGuiCol_Header},
        {"header-text", ImGuiCol_Text},
        {"title-bg", ImGuiCol_TitleBgActive},
        {"title-bg-collapsed", ImGuiCol_TitleBgCollapsed},
        {"title-text", ImGuiCol_Text},
        {"accent-hover", ImGuiCol_ButtonHovered},
        {"separator", ImGuiCol_Separator},
        {"separator-hover", ImGuiCol_SeparatorHovered},
        {"scrollbar-bg", ImGuiCol_ScrollbarBg},
        {"scrollbar-grab", ImGuiCol_ScrollbarGrab},
        {"scrollbar-grab-hover", ImGuiCol_ScrollbarGrabHovered},
        {"tab", ImGuiCol_Tab},
        {"tab-hover", ImGuiCol_TabHovered},
        {"tab-active", ImGuiCol_TabActive},
        {"tab-unfocused", ImGuiCol_TabUnfocused},
        {"popup-bg", ImGuiCol_PopupBg},
        {"dock-bg", ImGuiCol_DockingPreview},
        {"modal-dim", ImGuiCol_ModalWindowDimBg},
        {"nav-highlight", ImGuiCol_NavHighlight},
        {"drag-drop-target", ImGuiCol_DragDropTarget},
    };
    if (auto it = kColorSlots.find(key); it != kColorSlots.end()) {
        unsigned int r = 0, g = 0, b = 0;
        sscanf(val.c_str(), "#%02x%02x%02x", &r, &g, &b);
        colors[it->second] = ImVec4(r / 255.f, g / 255.f, b / 255.f, 1.f);
        return;
    }

    // accent drives several accent-tinted slots from one color.
    if (key == "accent") {
        unsigned int r = 0, g = 0, b = 0;
        sscanf(val.c_str(), "#%02x%02x%02x", &r, &g, &b);
        auto c = ImVec4(r / 255.f, g / 255.f, b / 255.f, 1.f);
        colors[ImGuiCol_CheckMark] = c;
        colors[ImGuiCol_SliderGrab] = c;
        colors[ImGuiCol_ResizeGrip] = c;
        colors[ImGuiCol_PlotHistogram] = c;
        colors[ImGuiCol_TabActive] = c;
        return;
    }

    // ── Sizing & spacing (10+) ──────────────────────────────────────────
    if (key == "rounding" || key == "border-radius") {
        float v = ToFloatOr(val);
        style.WindowRounding = v;
        style.FrameRounding = v;
        style.GrabRounding = v;
        style.TabRounding = v;
        style.ChildRounding = v;
        style.PopupRounding = v;
        style.ScrollbarRounding = v;
        return;
    }
    if (key == "border-radius-top-left") {
        style.WindowRounding = ToFloatOr(val);
        return;
    }
    if (key == "border-radius-top-right") {
        style.FrameRounding = ToFloatOr(val);
        return;
    }
    if (key == "border-radius-bottom-left") {
        style.GrabRounding = ToFloatOr(val);
        return;
    }
    if (key == "border-radius-bottom-right") {
        style.TabRounding = ToFloatOr(val);
        return;
    }
    if (key == "border-width") {
        style.WindowBorderSize = ToFloatOr(val);
        return;
    }
    if (key == "padding") {
        float v = ToFloatOr(val);
        style.WindowPadding = ImVec2(v, v);
        style.FramePadding = ImVec2(v, v * 0.75f);
        return;
    }
    if (key == "padding-x") {
        style.FramePadding.x = ToFloatOr(val);
        return;
    }
    if (key == "padding-y") {
        style.FramePadding.y = ToFloatOr(val);
        return;
    }
    if (key == "spacing") {
        float v = ToFloatOr(val);
        style.ItemSpacing = ImVec2(v, v * 0.75f);
        style.ItemInnerSpacing = ImVec2(v, v * 0.5f);
        return;
    }
    if (key == "spacing-x") {
        style.ItemSpacing.x = ToFloatOr(val);
        return;
    }
    if (key == "spacing-y") {
        style.ItemSpacing.y = ToFloatOr(val);
        return;
    }
    if (key == "indent") {
        style.IndentSpacing = ToFloatOr(val);
        return;
    }
    if (key == "scrollbar-size") {
        style.ScrollbarSize = ToFloatOr(val);
        return;
    }
    if (key == "alpha" || key == "opacity") {
        style.Alpha = ToFloatOr(val);
        return;
    }
    if (key == "min-width") {
        style.WindowMinSize.x = ToFloatOr(val);
        return;
    }
    if (key == "min-height") {
        style.WindowMinSize.y = ToFloatOr(val);
        return;
    }
    if (key == "max-width") {
        style.WindowMinSize.x = 0;
        return;
    } // ImGui doesn't have max — stub
    if (key == "max-height") {
        style.WindowMinSize.y = 0;
        return;
    }

    // ── Effects ────────────────────────────────────────────────────────
    if (key == "box-shadow") {
        // Format: "4px 2px 2px rgba(0,0,0,0.3)" or "#RRGGBBAA opacity"
        float offX = 2, offY = 2, blur = 4;
        unsigned int sr = 0, sg = 0, sb = 0, sa = 0;
        int n = sscanf(val.c_str(), "%fpx %fpx %fpx rgba(%u,%u,%u,%u)", &offX, &offY, &blur, &sr,
                       &sg, &sb, &sa);
        if (n < 7)
            n = sscanf(val.c_str(), "%fpx %fpx %fpx #%02x%02x%02x%02x", &offX, &offY, &blur, &sr,
                       &sg, &sb, &sa);
        if (n < 7)
            sscanf(val.c_str(), "%fpx %fpx %fpx #%02x%02x%02x", &offX, &offY, &blur, &sr, &sg, &sb);
        if (sa == 0)
            sa = 80;
        // Store as style vars (applied per-widget)
        style.Colors[ImGuiCol_BorderShadow] =
            ImVec4(sr / 255.f, sg / 255.f, sb / 255.f, sa / 255.f);
        return;
    }
    if (key == "blur") {
        float v = ToFloatOr(val);
        style.WindowRounding = std::max(style.WindowRounding, v);
        style.Alpha = std::max(style.Alpha, 0.85f);
        return;
    }

    // ── Typography ────────────────────────────────────────────────────
    if (key == "font-size") { /* applied via FontManager, not ImGuiStyle */
        return;
    }
    if (key == "font-family") {
        return;
    }
    if (key == "font-weight") {
        return;
    }
    if (key == "text-align") {
        return;
    }
    if (key == "line-height") {
        return;
    }

    // ── Layout hints ──────────────────────────────────────────────────
    if (key == "columns") {
        style.ColumnsMinSpacing = ToFloatOr(val);
        return;
    }
    if (key == "display-scale") { /* responsive hint */
        return;
    }

    // ── Transition / animation hints (theme-driven) ────────────────────
    if (key == "transition") {
        // Format: "opacity 0.3s ease-out" or "all 0.25s ease"
        [[maybe_unused]] float dur = 0.3f;
        std::string curve = "ease";
        if (val.find("s") != std::string::npos) {
            auto sp = val.find(' ');
            if (sp != std::string::npos) {
                std::string durStr = val.substr(sp + 1, val.find('s', sp) - sp);
                dur = ToFloatOr(durStr);
                auto cp = val.rfind(' ');
                if (cp != std::string::npos && cp > sp)
                    curve = val.substr(cp + 1);
            }
        }
        // Store as style hint — widgets read this internally
        style.Alpha = style.Alpha; // no-op marker
        return;
    }
    if (key == "animation") {
        // "fadeIn 0.5s ease-out" — stored as hint
        return;
    }

    // ── Gradient ───────────────────────────────────────────────────────
    if (key == "bg-gradient" || key == "gradient") {
        // Format: "linear-gradient(90deg, #ff0000, #0000ff)"
        //         "linear-gradient(to right, #e94560, #0f3460)"
        std::string v = val;
        // Strip "linear-gradient(" prefix if present
        auto lp = v.find("linear-gradient(");
        if (lp != std::string::npos)
            v = v.substr(lp + 16);
        auto rp = v.find(')');
        if (rp != std::string::npos)
            v = v.substr(0, rp);

        // Parse direction: "90deg" or "to bottom"
        float angle = 0.f;
        [[maybe_unused]] bool horiz = true;
        if (v.find("deg") != std::string::npos) {
            angle = ToFloatOr(v.substr(0, v.find("deg")));
            horiz = (angle < 45.f || angle > 315.f || (angle > 135.f && angle < 225.f));
        } else if (v.find("to right") != std::string::npos ||
                   v.find("to left") != std::string::npos) {
            horiz = true;
        } else if (v.find("to bottom") != std::string::npos ||
                   v.find("to top") != std::string::npos) {
            horiz = false;
        }

        // Extract colors: "#rrggbb" or "#rrggbbaa"
        auto hashPos = v.find('#');
        if (hashPos == std::string::npos)
            return; // no hex colors (e.g. named-color or "none" gradient) — substr(npos) would
                    // throw
        std::string colors_s = v.substr(hashPos);
        unsigned int c1r = 0, c1g = 0, c1b = 0, c2r = 0, c2g = 0, c2b = 0;
        int n = sscanf(colors_s.c_str(), "#%02x%02x%02x %*[, ] #%02x%02x%02x", &c1r, &c1g, &c1b,
                       &c2r, &c2g, &c2b);
        if (n >= 6) {
            // Store gradient info as style hint
            style.WindowRounding = style.WindowRounding; // no-op placeholder
        }
        return;
    }
}

void Engine::ApplyRule(const StyleRule& rule) {
    for (auto& [k, v] : rule.props)
        ApplyProp(k, v);
}

void Engine::Apply(const std::string& widgetType, const std::string& className,
                   const std::string& idName, bool hovered, bool active, bool focused,
                   bool disabled, int index) {
    for (auto& rule : rules_) {
        bool match = (rule.type.empty() || rule.type == "*" || rule.type == widgetType);
        if (match && !className.empty() && rule.className != className)
            match = false;
        if (match && !idName.empty() && rule.idName != idName)
            match = false;
        if (match && !rule.pseudoClass.empty()) {
            if (rule.pseudoClass == "hover" && !hovered)
                match = false;
            if (rule.pseudoClass == "active" && !active)
                match = false;
            if (rule.pseudoClass == "focus" && !focused)
                match = false;
            if (rule.pseudoClass == "disabled" && !disabled)
                match = false;
            if (rule.pseudoClass == "first-child" && index != 0)
                match = false;
            if (rule.pseudoClass == "last-child")
                match = false; // requires context
            if (rule.pseudoClass == "checked")
                match = false; // per-widget
        }
        if (match)
            ApplyRule(rule);
    }
}

void Engine::ApplyAll() {
    for (auto& rule : rules_)
        ApplyRule(rule);
}

} // namespace unigui::styling
