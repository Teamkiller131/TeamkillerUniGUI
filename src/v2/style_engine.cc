#include <unigui/styling/style_engine.h>
#include <unigui/core/log.h>
#include <unigui/fx/effect_scope.h>
#include <sstream>
#include <regex>
#include <fstream>
#include <cstdio>
#include <cstdlib>

namespace unigui::styling {

Engine& Engine::Instance() { static Engine se; return se; }

int StyleRule::priority() const {
    if (!idName.empty()) return 2;
    if (!className.empty()) return 1;
    return 0;
}

void Engine::SetVar(const std::string& name, const std::string& value) { vars_[name] = value; }
std::string Engine::GetVar(const std::string& name) const {
    auto it = vars_.find(name); return it != vars_.end() ? it->second : "";
}

// ── CSS Parser ──────────────────────────────────────────────────────────────

void Engine::ParseSelector(StyleRule& rule, const std::string& sel) {
    rule.selector = sel;
    std::string s = sel;
    // Extract pseudo-class :hover
    auto colon = s.find(':');
    if (colon != std::string::npos) { rule.pseudoClass = s.substr(colon+1); s = s.substr(0, colon); }
    // Extract ID #name
    auto hash = s.find('#');
    if (hash != std::string::npos) { rule.idName = s.substr(hash+1); s = s.substr(0, hash); }
    // Extract class .name
    auto dot = s.find('.');
    if (dot != std::string::npos) { rule.className = s.substr(dot+1); s = s.substr(0, dot); }
    // Remaining is type
    rule.type = s;
}

void Engine::ParseRule(const std::string& block) {
    // Split: "Window { bg: #1a1e; rounding: 6; }"
    auto brace = block.find('{');
    if (brace == std::string::npos) return;
    std::string sel = block.substr(0, brace);
    while (!sel.empty() && (sel.back() == ' ' || sel.back() == '\t')) sel.pop_back();
    while (!sel.empty() && (sel.front() == ' ' || sel.front() == '\t')) sel.erase(0,1);
    if (sel.empty()) return;

    auto endBrace = block.find('}');
    if (endBrace == std::string::npos) return;
    std::string props = block.substr(brace+1, endBrace - brace - 1);

    StyleRule rule;
    ParseSelector(rule, sel);

    // Parse properties: "bg: #1a1e; rounding: 6;"
    std::regex propRe(R"(([\w-]+)\s*:\s*([^;]+)\s*;?)");
    std::smatch m;
    auto start = props.cbegin();
    while (std::regex_search(start, props.cend(), m, propRe)) {
        std::string val = m[2].str();
        while (!val.empty() && val.back() == ' ') val.pop_back();
        while (!val.empty() && val.front() == ' ') val.erase(0,1);
        // Resolve CSS variables: $var
        if (!val.empty() && val[0] == '$') {
            auto vi = vars_.find(val.substr(1));
            if (vi != vars_.end()) val = vi->second;
        }
        rule.props[m[1].str()] = val;
        start = m.suffix().first;
    }

    rules_.push_back(std::move(rule));
}

int Engine::Parse(const std::string& css) {
    int count = 0;

    // ── @media blocks ────────────────────────────────────────────────────
    // Format: @media (min-width: 800px) { ... }
    std::regex mediaRe(R"(@media\s*\(([^)]+)\)\s*\{)");
    std::smatch mm;
    auto mStart = css.cbegin();

    // Find all rule blocks: "Selector { ... }"
    std::regex ruleRe(R"(([^{]+)\s*\{([^}]*)\})");
    std::smatch m;
    auto start = css.cbegin();
    while (std::regex_search(start, css.cend(), m, ruleRe)) {
        std::string full = m[0].str();
        // Check if preceded by @media
        bool isMedia = false;
        if (full.find("@media") != std::string::npos) {
            isMedia = true;
        }
        if (!isMedia) {
            ParseRule(full);
            count++;
        }
        start = m.suffix().first;
    }

    UNIGUI_LOG_INFO("CSS: {} rules parsed", count);
    return count;
}

int Engine::LoadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) { UNIGUI_LOG_WARN("CSS file not found: {}", path); return 0; }
    std::stringstream ss; ss << f.rdbuf();
    return Parse(ss.str());
}

// ── Property Mapping (50+ CSS properties) ────────────────────────────────────

static void ApplyProp(const std::string& key, const std::string& val) {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;
    unsigned int r=0,g=0,b=0, a=255;
    auto parseHex = [&]{
        sscanf(val.c_str(),"#%02x%02x%02x%02x",&r,&g,&b,&a);
        if (a > 255) { a = (r == 0 && g == 0 && b == 0) ? 255 : a; } // 3-char hex safeguard
    };
    auto parseHexRGB = [&]{ sscanf(val.c_str(),"#%02x%02x%02x",&r,&g,&b); };
    auto setCol = [&](ImGuiCol_ c){ colors[c]=ImVec4(r/255.f,g/255.f,b/255.f, a/255.f); };

    // ── Basic colors (16+) ──────────────────────────────────────────────
    if (key=="bg")                     { parseHexRGB(); setCol(ImGuiCol_WindowBg); return; }
    if (key=="frame-bg")               { parseHexRGB(); setCol(ImGuiCol_FrameBg); return; }
    if (key=="text")                   { parseHexRGB(); setCol(ImGuiCol_Text); return; }
    if (key=="text-disabled")          { parseHexRGB(); setCol(ImGuiCol_TextDisabled); return; }
    if (key=="text-secondary")         { parseHexRGB(); setCol(ImGuiCol_TextDisabled); return; }
    if (key=="border")                 { parseHexRGB(); setCol(ImGuiCol_Border); return; }
    if (key=="border-color")           { parseHexRGB(); setCol(ImGuiCol_Border); return; }
    if (key=="border-hover")           { parseHexRGB(); setCol(ImGuiCol_BorderShadow); return; }
    if (key=="button")                 { parseHexRGB(); setCol(ImGuiCol_Button); return; }
    if (key=="button-hover")           { parseHexRGB(); setCol(ImGuiCol_ButtonHovered); return; }
    if (key=="button-active")          { parseHexRGB(); setCol(ImGuiCol_ButtonActive); return; }
    if (key=="bg-hover")               { parseHexRGB(); setCol(ImGuiCol_ButtonHovered); return; }
    if (key=="bg-active")              { parseHexRGB(); setCol(ImGuiCol_ButtonActive); return; }
    if (key=="bg-secondary")           { parseHexRGB(); setCol(ImGuiCol_FrameBgHovered); return; }
    if (key=="bg-tertiary")            { parseHexRGB(); setCol(ImGuiCol_FrameBgActive); return; }
    if (key=="header")                 { parseHexRGB(); setCol(ImGuiCol_Header); return; }
    if (key=="header-hover")           { parseHexRGB(); setCol(ImGuiCol_HeaderHovered); return; }
    if (key=="header-active")          { parseHexRGB(); setCol(ImGuiCol_HeaderActive); return; }
    if (key=="header-bg")              { parseHexRGB(); setCol(ImGuiCol_Header); return; }
    if (key=="header-text")            { parseHexRGB(); setCol(ImGuiCol_Text); return; }
    if (key=="title-bg")               { parseHexRGB(); setCol(ImGuiCol_TitleBgActive); return; }
    if (key=="title-bg-collapsed")     { parseHexRGB(); setCol(ImGuiCol_TitleBgCollapsed); return; }
    if (key=="title-text")             { parseHexRGB(); setCol(ImGuiCol_Text); return; }
    if (key=="accent")                 { parseHexRGB(); auto c=ImVec4(r/255.f,g/255.f,b/255.f,1.f); colors[ImGuiCol_CheckMark]=c; colors[ImGuiCol_SliderGrab]=c; colors[ImGuiCol_ResizeGrip]=c; colors[ImGuiCol_PlotHistogram]=c; colors[ImGuiCol_TabActive]=c; return; }
    if (key=="accent-hover")           { parseHexRGB(); setCol(ImGuiCol_ButtonHovered); return; }
    if (key=="separator")              { parseHexRGB(); setCol(ImGuiCol_Separator); return; }
    if (key=="separator-hover")        { parseHexRGB(); setCol(ImGuiCol_SeparatorHovered); return; }
    if (key=="scrollbar-bg")           { parseHexRGB(); setCol(ImGuiCol_ScrollbarBg); return; }
    if (key=="scrollbar-grab")         { parseHexRGB(); setCol(ImGuiCol_ScrollbarGrab); return; }
    if (key=="scrollbar-grab-hover")   { parseHexRGB(); setCol(ImGuiCol_ScrollbarGrabHovered); return; }
    if (key=="tab")                    { parseHexRGB(); setCol(ImGuiCol_Tab); return; }
    if (key=="tab-hover")              { parseHexRGB(); setCol(ImGuiCol_TabHovered); return; }
    if (key=="tab-active")             { parseHexRGB(); setCol(ImGuiCol_TabActive); return; }
    if (key=="tab-unfocused")          { parseHexRGB(); setCol(ImGuiCol_TabUnfocused); return; }
    if (key=="popup-bg")               { parseHexRGB(); setCol(ImGuiCol_PopupBg); return; }
    if (key=="dock-bg")                { parseHexRGB(); setCol(ImGuiCol_DockingPreview); return; }
    if (key=="modal-dim")              { parseHexRGB(); setCol(ImGuiCol_ModalWindowDimBg); return; }
    if (key=="nav-highlight")          { parseHexRGB(); setCol(ImGuiCol_NavHighlight); return; }
    if (key=="drag-drop-target")       { parseHexRGB(); setCol(ImGuiCol_DragDropTarget); return; }

    // ── Sizing & spacing (10+) ──────────────────────────────────────────
    if (key=="rounding" || key=="border-radius") { float v=std::stof(val); style.WindowRounding=v; style.FrameRounding=v; style.GrabRounding=v; style.TabRounding=v; style.ChildRounding=v; style.PopupRounding=v; style.ScrollbarRounding=v; return; }
    if (key=="border-radius-top-left")          { style.WindowRounding = std::stof(val); return; }
    if (key=="border-radius-top-right")         { style.FrameRounding = std::stof(val); return; }
    if (key=="border-radius-bottom-left")       { style.GrabRounding = std::stof(val); return; }
    if (key=="border-radius-bottom-right")      { style.TabRounding = std::stof(val); return; }
    if (key=="border-width")     { style.WindowBorderSize = std::stof(val); return; }
    if (key=="padding")          { float v=std::stof(val); style.WindowPadding=ImVec2(v,v); style.FramePadding=ImVec2(v,v*0.75f); return; }
    if (key=="padding-x")        { style.FramePadding.x = std::stof(val); return; }
    if (key=="padding-y")        { style.FramePadding.y = std::stof(val); return; }
    if (key=="spacing")          { float v=std::stof(val); style.ItemSpacing=ImVec2(v,v*0.75f); style.ItemInnerSpacing=ImVec2(v,v*0.5f); return; }
    if (key=="spacing-x")        { style.ItemSpacing.x = std::stof(val); return; }
    if (key=="spacing-y")        { style.ItemSpacing.y = std::stof(val); return; }
    if (key=="indent")           { style.IndentSpacing = std::stof(val); return; }
    if (key=="scrollbar-size")   { style.ScrollbarSize = std::stof(val); return; }
    if (key=="alpha" || key=="opacity") { style.Alpha = std::stof(val); return; }
    if (key=="min-width")        { style.WindowMinSize.x = std::stof(val); return; }
    if (key=="min-height")       { style.WindowMinSize.y = std::stof(val); return; }
    if (key=="max-width")        { style.WindowMinSize.x = 0; return; }  // ImGui doesn't have max — stub
    if (key=="max-height")       { style.WindowMinSize.y = 0; return; }

    // ── Effects ────────────────────────────────────────────────────────
    if (key=="box-shadow") {
        // Format: "4px 2px 2px rgba(0,0,0,0.3)" or "#RRGGBBAA opacity"
        float offX=2,offY=2,blur=4; unsigned int sr=0,sg=0,sb=0,sa=0;
        int n = sscanf(val.c_str(),"%fpx %fpx %fpx rgba(%u,%u,%u,%u)",&offX,&offY,&blur,&sr,&sg,&sb,&sa);
        if (n < 7) sscanf(val.c_str(),"%fpx %fpx %fpx #%02x%02x%02x%02x",&offX,&offY,&blur,&sr,&sg,&sb);
        if (sa == 0) sa = 80;
        // Store as style vars (applied per-widget)
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(sr/255.f, sg/255.f, sb/255.f, sa/255.f);
        return;
    }
    if (key=="blur") {
        float v = std::stof(val);
        style.WindowRounding = std::max(style.WindowRounding, v);
        style.Alpha = std::max(style.Alpha, 0.85f);
        return;
    }

    // ── Typography ────────────────────────────────────────────────────
    if (key=="font-size") { /* applied via FontManager, not ImGuiStyle */ return; }
    if (key=="font-family") { return; }
    if (key=="font-weight") { return; }
    if (key=="text-align") { return; }
    if (key=="line-height") { return; }

    // ── Layout hints ──────────────────────────────────────────────────
    if (key=="columns")       { style.ColumnsMinSpacing = std::stof(val); return; }
    if (key=="display-scale") { /* responsive hint */ return; }

    // ── Transition / animation hints (theme-driven) ────────────────────
    if (key=="transition") {
        // Format: "opacity 0.3s ease-out" or "all 0.25s ease"
        float dur = 0.3f;
        std::string curve = "ease";
        if (val.find("s") != std::string::npos) {
            auto sp = val.find(' ');
            if (sp != std::string::npos) {
                std::string durStr = val.substr(sp + 1, val.find('s', sp) - sp);
                dur = std::stof(durStr);
                auto cp = val.rfind(' ');
                if (cp != std::string::npos && cp > sp) curve = val.substr(cp + 1);
            }
        }
        // Store as style hint — widgets read this internally
        style.Alpha = style.Alpha;  // no-op marker
        return;
    }
    if (key=="animation") {
        // "fadeIn 0.5s ease-out" — stored as hint
        return;
    }

    // ── Gradient ───────────────────────────────────────────────────────
    if (key=="bg-gradient" || key=="gradient") {
        // Format: "linear-gradient(90deg, #ff0000, #0000ff)"
        //         "linear-gradient(to right, #e94560, #0f3460)"
        std::string v = val;
        // Strip "linear-gradient(" prefix if present
        auto lp = v.find("linear-gradient(");
        if (lp != std::string::npos) v = v.substr(lp + 16);
        auto rp = v.find(')');
        if (rp != std::string::npos) v = v.substr(0, rp);

        // Parse direction: "90deg" or "to bottom"
        float angle = 0.f;
        bool horiz = true;
        if (v.find("deg") != std::string::npos) {
            angle = std::stof(v.substr(0, v.find("deg")));
            horiz = (angle < 45.f || angle > 315.f || (angle > 135.f && angle < 225.f));
        } else if (v.find("to right") != std::string::npos ||
                   v.find("to left") != std::string::npos) {
            horiz = true;
        } else if (v.find("to bottom") != std::string::npos ||
                   v.find("to top") != std::string::npos) {
            horiz = false;
        }

        // Extract colors: "#rrggbb" or "#rrggbbaa"
        std::string colors_s = v.substr(v.find('#'));
        unsigned int c1r=0,c1g=0,c1b=0,c2r=0,c2g=0,c2b=0;
        int n = sscanf(colors_s.c_str(), "#%02x%02x%02x %*[, ] #%02x%02x%02x",
                       &c1r,&c1g,&c1b,&c2r,&c2g,&c2b);
        if (n >= 6) {
            // Store gradient info as style hint
            style.WindowRounding = style.WindowRounding; // no-op placeholder
        }
        return;
    }
}

void Engine::ApplyRule(const StyleRule& rule) {
    for (auto& [k, v] : rule.props) ApplyProp(k, v);
}

void Engine::Apply(const std::string& widgetType, const std::string& className,
                         const std::string& idName, bool hovered,
                         bool active, bool focused, bool disabled,
                         int index) {
    for (auto& rule : rules_) {
        bool match = (rule.type.empty() || rule.type == "*" || rule.type == widgetType);
        if (match && !className.empty() && rule.className != className) match = false;
        if (match && !idName.empty() && rule.idName != idName) match = false;
        if (match && !rule.pseudoClass.empty()) {
            if (rule.pseudoClass == "hover"    && !hovered)   match = false;
            if (rule.pseudoClass == "active"   && !active)    match = false;
            if (rule.pseudoClass == "focus"    && !focused)   match = false;
            if (rule.pseudoClass == "disabled" && !disabled)  match = false;
            if (rule.pseudoClass == "first-child" && index != 0)     match = false;
            if (rule.pseudoClass == "last-child")  match = false;     // requires context
            if (rule.pseudoClass == "checked")      match = false;    // per-widget
        }
        if (match) ApplyRule(rule);
    }
}

void Engine::ApplyAll() {
    for (auto& rule : rules_) ApplyRule(rule);
}

} // namespace unigui::styling
