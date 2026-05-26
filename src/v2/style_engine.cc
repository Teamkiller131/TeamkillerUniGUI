#include <unigui/v2/style_engine.h>
#include <unigui/core/log.h>
#include <sstream>
#include <regex>
#include <fstream>
#include <cstdio>

namespace unigui::v2 {

StyleEngine& StyleEngine::Instance() { static StyleEngine se; return se; }

int StyleRule::priority() const {
    if (!idName.empty()) return 2;
    if (!className.empty()) return 1;
    return 0;
}

void StyleEngine::SetVar(const std::string& name, const std::string& value) { vars_[name] = value; }
std::string StyleEngine::GetVar(const std::string& name) const {
    auto it = vars_.find(name); return it != vars_.end() ? it->second : "";
}

// ── CSS Parser ──────────────────────────────────────────────────────────────

void StyleEngine::ParseSelector(StyleRule& rule, const std::string& sel) {
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

void StyleEngine::ParseRule(const std::string& block) {
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

int StyleEngine::Parse(const std::string& css) {
    int count = 0;
    // Find all rule blocks: "Selector { ... }"
    std::regex ruleRe(R"(([^{]+)\s*\{([^}]*)\})");
    std::smatch m;
    auto start = css.cbegin();
    while (std::regex_search(start, css.cend(), m, ruleRe)) {
        ParseRule(m[0].str());
        count++;
        start = m.suffix().first;
    }
    UNIGUI_LOG_INFO("CSS: {} rules parsed", count);
    return count;
}

int StyleEngine::LoadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) { UNIGUI_LOG_WARN("CSS file not found: {}", path); return 0; }
    std::stringstream ss; ss << f.rdbuf();
    return Parse(ss.str());
}

// ── Property Mapping ────────────────────────────────────────────────────────

static void ApplyProp(const std::string& key, const std::string& val) {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;
    unsigned int r=0,g=0,b=0;
    auto parseHex = [&](ImGuiCol_ c){ colors[c]=ImVec4(r/255.f,g/255.f,b/255.f,1); };
    auto parseHexAll = [&]{ sscanf(val.c_str(),"#%02x%02x%02x",&r,&g,&b); };

    if (key=="bg")            { parseHexAll(); parseHex(ImGuiCol_WindowBg); return; }
    if (key=="frame-bg")      { parseHexAll(); parseHex(ImGuiCol_FrameBg); return; }
    if (key=="text")          { parseHexAll(); parseHex(ImGuiCol_Text); return; }
    if (key=="border")        { parseHexAll(); parseHex(ImGuiCol_Border); return; }
    if (key=="button")        { parseHexAll(); parseHex(ImGuiCol_Button); return; }
    if (key=="button-hover")  { parseHexAll(); parseHex(ImGuiCol_ButtonHovered); return; }
    if (key=="button-active") { parseHexAll(); parseHex(ImGuiCol_ButtonActive); return; }
    if (key=="bg-hover")      { parseHexAll(); parseHex(ImGuiCol_ButtonHovered); return; }
    if (key=="bg-active")     { parseHexAll(); parseHex(ImGuiCol_ButtonActive); return; }
    if (key=="header")        { parseHexAll(); parseHex(ImGuiCol_Header); return; }
    if (key=="title-bg")      { parseHexAll(); parseHex(ImGuiCol_TitleBgActive); return; }
    if (key=="accent")        { parseHexAll(); colors[ImGuiCol_CheckMark]=ImVec4(r/255.f,g/255.f,b/255.f,1); colors[ImGuiCol_SliderGrab]=colors[ImGuiCol_CheckMark]; return; }
    if (key=="rounding")      { float v=std::stof(val); style.WindowRounding=v; style.FrameRounding=v; style.GrabRounding=v; return; }
    if (key=="padding")       { float v=std::stof(val); style.WindowPadding=ImVec2(v,v); style.FramePadding=ImVec2(v,v*0.75f); return; }
    if (key=="spacing")       { float v=std::stof(val); style.ItemSpacing=ImVec2(v,v*0.75f); return; }
    if (key=="scrollbar-size"){ style.ScrollbarSize=std::stof(val); return; }
    if (key=="alpha")         { style.Alpha=std::stof(val); return; }
}

void StyleEngine::ApplyRule(const StyleRule& rule) {
    for (auto& [k, v] : rule.props) ApplyProp(k, v);
}

void StyleEngine::Apply(const std::string& widgetType, const std::string& className,
                         const std::string& idName, bool hovered) {
    for (auto& rule : rules_) {
        bool match = (rule.type.empty() || rule.type == "*" || rule.type == widgetType);
        if (match && !className.empty() && rule.className != className) match = false;
        if (match && !idName.empty() && rule.idName != idName) match = false;
        if (match && !rule.pseudoClass.empty()) {
            if (rule.pseudoClass == "hover" && !hovered) match = false;
        }
        if (match) ApplyRule(rule);
    }
}

void StyleEngine::ApplyAll() {
    for (auto& rule : rules_) ApplyRule(rule);
}

} // namespace unigui::v2
