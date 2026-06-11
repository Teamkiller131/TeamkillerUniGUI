#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {
class CollapsingHeader : public Widget {
public:
    CollapsingHeader(std::string name, std::string label, bool default_open = false);
    void Render() override;
    bool IsOpen() const;
    void SetOpen(bool open);
    void SetContentCallback(std::function<void()> cb);
    void SetOnToggle(std::function<void(bool)> fn);
    const std::string& GetLabel() const;

private:
    std::string label_;
    bool open_;
    std::function<void()> content_callback_;
    std::function<void(bool)> onToggle_;
};
} // namespace unigui
