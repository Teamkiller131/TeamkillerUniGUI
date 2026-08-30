#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

/// Simple Markdown renderer supporting:
///   # Header1, ## Header2, ### Header3
///   **bold**, *italic*, `code`
///   - bullet list items
///   --- horizontal rule
///   regular paragraph text (word-wrapped)
class Markdown : public FluentWidget<Markdown> {
public:
    Markdown(std::string name, std::string markdown = "");

    void Render() override;

    /// Set the markdown source text.
    void SetMarkdown(std::string md);
    const std::string& GetMarkdown() const { return source_; }

    /// Set a callback for link clicks (e.g., [text](url)).
    void SetLinkCallback(std::function<void(const std::string& url)> cb);

    /// Set maximum width for text wrapping (0 = fill available).
    void SetMaxWidth(float w) { maxWidth_ = w; }

    // ── Fluent (chainable) helpers — return Markdown& via CRTP base ──────────
    Markdown& WithMarkdown(std::string md) {
        SetMarkdown(std::move(md));
        return *this;
    }
    Markdown& WithLinkCallback(std::function<void(const std::string& url)> cb) {
        SetLinkCallback(std::move(cb));
        return *this;
    }
    Markdown& WithMaxWidth(float w) {
        SetMaxWidth(w);
        return *this;
    }

private:
    void RenderLine(const std::string& line);
    void RenderInline(const std::string& text);

    std::string source_;
    std::function<void(const std::string&)> linkCallback_;
    float maxWidth_ = 0.0f;
};

} // namespace unigui
