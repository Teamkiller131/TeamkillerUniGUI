#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {
class CollapsingHeader : public Widget {
public:
    CollapsingHeader(std::string name, std::string label, bool default_open = false);
    void Render() override;
    bool IsOpen() const;
    void SetOpen(bool open);
    void SetContentCallback(std::function<void()> cb);
    const std::string& GetLabel() const;
private:
    std::string label_;
    bool open_;
    std::function<void()> content_callback_;
};
}
