#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

/// PanelBox — dark panel container with title bar + tinted content area.
/// Uses PushID/PopID for ID safety.
class PanelBox : public FluentWidget<PanelBox> {
public:
    PanelBox(std::string name, std::string title);
    void Render() override;

    void SetTintColor(ImU32 color) { tintColor_ = color; }
    void SetContentCallback(std::function<void()> cb) { contentCb_ = std::move(cb); }
    /// When true, the bordered content area shrink-wraps to its children (capped
    /// by the parent splitter height) instead of always filling the slot.
    void SetShrinkWrapContent(bool on) { shrinkWrapContent_ = on; }

    const std::string& GetTitle() const { return title_; }
    void SetTitle(std::string title) { title_ = std::move(title); }

    // ── Fluent (chainable) helpers — return PanelBox& via CRTP base ──────────
    PanelBox& WithTintColor(ImU32 color) {
        SetTintColor(color);
        return *this;
    }
    PanelBox& WithContentCallback(std::function<void()> cb) {
        SetContentCallback(std::move(cb));
        return *this;
    }
    PanelBox& WithShrinkWrapContent(bool on) {
        SetShrinkWrapContent(on);
        return *this;
    }
    PanelBox& WithTitle(std::string title) {
        SetTitle(std::move(title));
        return *this;
    }

private:
    std::string title_;
    ImU32 tintColor_ = 0;
    bool shrinkWrapContent_ = false;
    std::function<void()> contentCb_;
};

} // namespace unigui
