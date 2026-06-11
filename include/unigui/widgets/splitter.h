#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
namespace unigui {
class Splitter : public Widget {
public:
    enum Orientation { Horizontal, Vertical };
    Splitter(std::string name, Orientation orientation = Horizontal, float split = 0.5f);
    void Render() override;
    float GetSplit() const;
    void SetContentA(std::function<void()> cb);
    void SetContentB(std::function<void()> cb);

private:
    Orientation ori_;
    float split_;
    std::function<void()> cbA_, cbB_;
};
} // namespace unigui
