#include <unigui/widgets/markdown.h>
#include <imgui.h>
#include <sstream>
#include <regex>

namespace unigui {

Markdown::Markdown(std::string name, std::string markdown)
    : Widget(std::move(name)), source_(std::move(markdown)) {}

void Markdown::SetMarkdown(std::string md) { source_ = std::move(md); }

void Markdown::RenderInline(const std::string& text) {
    // Parse inline: **bold**, *italic*, `code`, [link](url), plain text
    std::string current;
    size_t i = 0;
    while (i < text.size()) {
        // Bold **...**
        if (i + 1 < text.size() && text[i] == '*' && text[i+1] == '*') {
            if (!current.empty()) { ImGui::TextUnformatted(current.c_str()); ImGui::SameLine(0, 0); current.clear(); }
            size_t end = text.find("**", i + 2);
            if (end != std::string::npos) {
                std::string bold = text.substr(i + 2, end - i - 2);
                ImFont* font = ImGui::GetIO().FontDefault;
                ImGui::PushFont(font);
                ImGui::TextUnformatted(bold.c_str());
                ImGui::PopFont();
                ImGui::SameLine(0, 0);
                i = end + 2;
                continue;
            }
        }
        // Italic *...* (but not **)
        if (text[i] == '*' && (i == 0 || text[i-1] != '*') && (i+1 >= text.size() || text[i+1] != '*')) {
            if (!current.empty()) { ImGui::TextUnformatted(current.c_str()); ImGui::SameLine(0, 0); current.clear(); }
            size_t end = text.find('*', i + 1);
            if (end != std::string::npos) {
                std::string italic = text.substr(i + 1, end - i - 1);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.8f);
                ImGui::TextUnformatted(italic.c_str());
                ImGui::PopStyleVar();
                ImGui::SameLine(0, 0);
                i = end + 1;
                continue;
            }
        }
        // Inline code `...`
        if (text[i] == '`') {
            if (!current.empty()) { ImGui::TextUnformatted(current.c_str()); ImGui::SameLine(0, 0); current.clear(); }
            size_t end = text.find('`', i + 1);
            if (end != std::string::npos) {
                std::string code = text.substr(i + 1, end - i - 1);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
                ImGui::TextUnformatted(code.c_str());
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 0);
                i = end + 1;
                continue;
            }
        }
        // Link [text](url)
        if (text[i] == '[') {
            if (!current.empty()) { ImGui::TextUnformatted(current.c_str()); ImGui::SameLine(0, 0); current.clear(); }
            size_t endBracket = text.find(']', i + 1);
            size_t endParen = (endBracket != std::string::npos && endBracket + 1 < text.size() && text[endBracket+1] == '(')
                ? text.find(')', endBracket + 2) : std::string::npos;
            if (endBracket != std::string::npos && endParen != std::string::npos) {
                std::string linkText = text.substr(i + 1, endBracket - i - 1);
                std::string url = text.substr(endBracket + 2, endParen - endBracket - 2);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
                ImGui::TextUnformatted(linkText.c_str());
                ImGui::PopStyleColor();
                if (ImGui::IsItemClicked() && linkCallback_) linkCallback_(url);
                ImGui::SameLine(0, 0);
                i = endParen + 1;
                continue;
            }
        }
        current += text[i];
        i++;
    }
    if (!current.empty()) ImGui::TextUnformatted(current.c_str());
}

void Markdown::RenderLine(const std::string& line) {
    if (line.empty()) {
        ImGui::Spacing();
        return;
    }

    // Headers
    if (line[0] == '#') {
        int level = 0;
        while (level < (int)line.size() && level < 3 && line[level] == '#') level++;
        std::string header = line.substr(level);
        while (!header.empty() && header[0] == ' ') header.erase(0, 1);
        float scale = 1.0f;
        if (level == 1) scale = 1.5f;
        else if (level == 2) scale = 1.3f;
        else if (level == 3) scale = 1.15f;
        ImGui::SetWindowFontScale(scale);
        ImGui::TextUnformatted(header.c_str());
        ImGui::SetWindowFontScale(1.0f);
        return;
    }

    // Horizontal rule
    if (line == "---" || line == "***" || line == "___") {
        ImGui::Separator();
        return;
    }

    // Bullet list
    if (line.size() >= 2 && line[0] == '-' && line[1] == ' ') {
        ImGui::Bullet();
        ImGui::SameLine();
        RenderInline(line.substr(2));
        return;
    }

    // Regular paragraph
    if (maxWidth_ > 0) ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + maxWidth_);
    RenderInline(line);
    if (maxWidth_ > 0) ImGui::PopTextWrapPos();
}

void Markdown::Render() {
    if (!IsVisible()) return;
    ImGui::BeginGroup();
    std::istringstream stream(source_);
    std::string line;
    while (std::getline(stream, line)) {
        // Strip trailing \r
        if (!line.empty() && line.back() == '\r') line.pop_back();
        RenderLine(line);
    }
    ImGui::EndGroup();
}

} // namespace unigui
