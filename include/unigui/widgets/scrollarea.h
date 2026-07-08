#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
namespace unigui {
class ScrollArea : public FluentWidget<ScrollArea> {
public:
    ScrollArea(std::string name, float width = 0, float height = 200);
    void Render() override;
    void SetContentCallback(std::function<void()> cb);
    void SetSize(float w, float h);

    // ── Fluent (chainable) helpers — return ScrollArea& via CRTP base ──────────
    ScrollArea& WithContentCallback(std::function<void()> cb) {
        SetContentCallback(std::move(cb));
        return *this;
    }
    ScrollArea& WithSize(float w, float h) {
        SetSize(w, h);
        return *this;
    }

private:
    float w_, h_;
    std::function<void()> cb_;
};
} // namespace unigui
