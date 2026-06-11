#pragma once

#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class Panel : public Widget {
public:
    Panel(std::string name, std::string title);
    void Render() override;
    void SetTitle(std::string title);
    const std::string& GetTitle() const;
    bool IsCollapsed() const;
    void SetContentCallback(std::function<void()> callback);
    void SetWrapEnabled(bool on) { wrap_ = on; }

private:
    std::string title_;
    bool collapsed_ = false;
    bool wrap_ = true;
    std::function<void()> content_callback_;
};

} // namespace unigui
