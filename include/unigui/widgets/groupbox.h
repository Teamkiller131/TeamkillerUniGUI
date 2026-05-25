#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {
class GroupBox : public Widget {
public:
    GroupBox(std::string name, std::string title);
    void Render() override;
    void SetTitle(std::string title);
    void SetContentCallback(std::function<void()> callback);
private:
    std::string title_;
    std::function<void()> content_callback_;
};
}
