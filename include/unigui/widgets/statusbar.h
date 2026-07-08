#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>

namespace unigui {
class StatusBar : public FluentWidget<StatusBar> {
public:
    StatusBar(std::string name, std::string text = "");
    void Render() override;
    void SetText(std::string text);
    const std::string& GetText() const;

    // ── Fluent (chainable) helpers — return StatusBar& via CRTP base ──────────
    StatusBar& WithText(std::string text) {
        SetText(std::move(text));
        return *this;
    }

private:
    std::string text_;
};
} // namespace unigui
