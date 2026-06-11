#include <unigui/widgets/richtext.h>

#include <imgui.h>

namespace unigui {

RichText::RichText(std::string name, std::string text)
        : Widget(std::move(name))
        , plain_text_(std::move(text)) {
    if (!plain_text_.empty())
        spans_.push_back({plain_text_});
}

void RichText::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::BeginGroup();
    if (spans_.empty()) {
        ImGui::TextUnformatted(plain_text_.c_str());
    } else {
        for (auto& s : spans_) {
            bool pushedFont = false;
            if (s.bold || s.italic) {
                ImFont* font = ImGui::GetIO().FontDefault;
                ImGui::PushFont(font);
                pushedFont = true;
            }
            if (s.color.w > 0.0f)
                ImGui::PushStyleColor(ImGuiCol_Text, s.color);
            ImGui::TextUnformatted(s.text.c_str());
            if (s.color.w > 0.0f)
                ImGui::PopStyleColor();
            if (pushedFont)
                ImGui::PopFont();
            if (&s != &spans_.back())
                ImGui::SameLine(0, 0);
        }
    }
    ImGui::EndGroup();
    ImGui::PopID();
}

void RichText::SetText(std::string text) {
    plain_text_ = std::move(text);
    spans_.clear();
    if (!plain_text_.empty())
        spans_.push_back({plain_text_});
}

std::string RichText::GetText() const {
    return plain_text_;
}

void RichText::SetSpans(std::vector<RichTextSpan> spans) {
    spans_ = std::move(spans);
    plain_text_.clear();
    for (auto& s : spans_)
        plain_text_ += s.text;
}

void RichText::AddSpan(std::string text, ImVec4 color, bool bold, bool italic) {
    spans_.push_back({std::move(text), bold, italic, color});
    for (auto& s : spans_)
        plain_text_ += s.text;
}

} // namespace unigui
