#pragma once

#include <unigui/widgets/widget_base.h>
#include <unigui/fx/animation.h>
#include <string>
#include <functional>

namespace unigui {

class Button : public Widget {
public:
    enum ColorVariant { Default, Primary, Danger, Success };
    enum Size { Small, Medium, Large };

    Button(std::string name, std::string label);
    void Render() override;
    bool WasClicked() const;
    const std::string& GetLabel() const;
    void SetLabel(std::string label);
    void SetColorVariant(ColorVariant variant);
    void SetSize(Size size);
    void SetOnClick(std::function<void()> fn);

private:
    std::string label_;
    bool clicked_ = false;
    ColorVariant variant_ = Default;
    Size sz_ = Medium;
    fx::AnimationState anim_;
    std::function<void()> onClick_;
};

} // namespace unigui
