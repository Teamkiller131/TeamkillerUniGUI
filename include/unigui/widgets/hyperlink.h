#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
namespace unigui {
class Hyperlink : public FluentWidget<Hyperlink> {
public:
    Hyperlink(std::string name, std::string label, std::string url = "");
    void Render() override;
    void SetURL(std::string url);
    void SetLabel(std::string label);
    bool WasClicked() const;

    // ── Fluent (chainable) helpers — return Hyperlink& via CRTP base ──────────
    Hyperlink& WithURL(std::string url) {
        SetURL(std::move(url));
        return *this;
    }
    Hyperlink& WithLabel(std::string label) {
        SetLabel(std::move(label));
        return *this;
    }

private:
    std::string label_, url_;
    bool clicked_ = false;
};
} // namespace unigui
