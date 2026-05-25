#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>
namespace unigui {
class ScrollArea : public Widget {
public:
    ScrollArea(std::string name, float width = 0, float height = 200);
    void Render() override;
    void SetContentCallback(std::function<void()> cb);
    void SetSize(float w, float h);
private: float w_,h_; std::function<void()> cb_;
};
}
