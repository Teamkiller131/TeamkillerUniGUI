#pragma once

#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class Panel : public FluentWidget<Panel> {
public:
    Panel(std::string name, std::string title);
    void Render() override;
    void SetTitle(std::string title);
    const std::string& GetTitle() const;
    bool IsCollapsed() const;
    void SetContentCallback(std::function<void()> callback);
    void SetWrapEnabled(bool on) { wrap_ = on; }

    // ── Fluent (chainable) helpers — return Panel& via CRTP base ──────────
    Panel& WithTitle(std::string title) {
        SetTitle(std::move(title));
        return *this;
    }
    Panel& WithContentCallback(std::function<void()> callback) {
        SetContentCallback(std::move(callback));
        return *this;
    }
    Panel& WithWrapEnabled(bool on) {
        SetWrapEnabled(on);
        return *this;
    }

private:
    std::string title_;
    bool collapsed_ = false;
    bool wrap_ = true;
    std::function<void()> content_callback_;
};

} // namespace unigui
