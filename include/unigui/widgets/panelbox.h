#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {

/// PanelBox — dark panel container with title bar + tinted content area.
/// Uses PushID/PopID for ID safety.
class PanelBox : public Widget {
public:
    PanelBox(std::string name, std::string title);
    void Render() override;

    void SetTintColor(ImU32 color) { tintColor_ = color; }
    void SetContentCallback(std::function<void()> cb) { contentCb_ = std::move(cb); }

    const std::string& GetTitle() const { return title_; }
    void SetTitle(std::string title) { title_ = std::move(title); }

private:
    std::string title_;
    ImU32 tintColor_ = 0;
    std::function<void()> contentCb_;
};

} // namespace unigui
