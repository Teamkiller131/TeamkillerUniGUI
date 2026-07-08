#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {
class GroupBox : public FluentWidget<GroupBox> {
public:
    GroupBox(std::string name, std::string title);
    void Render() override;
    void SetTitle(std::string title);
    void SetContentCallback(std::function<void()> callback);

    // ── Fluent (chainable) helpers — return GroupBox& via CRTP base ──────────
    GroupBox& WithTitle(std::string title) {
        SetTitle(std::move(title));
        return *this;
    }
    GroupBox& WithContentCallback(std::function<void()> callback) {
        SetContentCallback(std::move(callback));
        return *this;
    }

private:
    std::string title_;
    std::function<void()> content_callback_;
};
} // namespace unigui
