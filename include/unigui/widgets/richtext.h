#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <string>
#include <vector>

namespace unigui {

/// A span of styled text within a RichText widget.
struct RichTextSpan {
    std::string text;
    bool bold = false;
    bool italic = false;
    ImVec4 color = ImVec4(1, 1, 1, 1);
};

/// Rich text widget: renders formatted text with bold/italic/color spans.
class RichText : public Widget {
public:
    RichText(std::string name, std::string text = "");
    void Render() override;

    /// Set plain text (single color, no formatting).
    void SetText(std::string text);
    std::string GetText() const;

    /// Clear all spans and set from formatted spans list.
    void SetSpans(std::vector<RichTextSpan> spans);
    /// Add a styled span to the end.
    void AddSpan(std::string text, ImVec4 color, bool bold = false, bool italic = false);

    const std::vector<RichTextSpan>& GetSpans() const { return spans_; }

private:
    std::string plain_text_;
    std::vector<RichTextSpan> spans_;
};

} // namespace unigui
