#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {
class CollapsingHeader : public FluentWidget<CollapsingHeader> {
public:
    CollapsingHeader(std::string name, std::string label, bool default_open = false);
    void Render() override;
    bool IsOpen() const;
    void SetOpen(bool open);
    void SetContentCallback(std::function<void()> cb);
    void SetOnToggle(std::function<void(bool)> fn);
    const std::string& GetLabel() const;

    // ── Fluent (chainable) helpers — return CollapsingHeader& via CRTP base ──
    CollapsingHeader& WithOpen(bool open) {
        SetOpen(open);
        return *this;
    }
    CollapsingHeader& WithContentCallback(std::function<void()> cb) {
        SetContentCallback(std::move(cb));
        return *this;
    }
    CollapsingHeader& WithOnToggle(std::function<void(bool)> fn) {
        SetOnToggle(std::move(fn));
        return *this;
    }

private:
    std::string label_;
    bool open_;
    std::function<void()> content_callback_;
    std::function<void(bool)> onToggle_;
};
} // namespace unigui
