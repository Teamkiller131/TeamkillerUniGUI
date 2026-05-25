#pragma once

#include <unigui/widgets/widget_base.h>
#include <string>

namespace unigui {

class Button : public Widget {
public:
    enum ColorVariant { Default, Primary, Danger, Success };
    enum Size { Small, Medium, Large };

    Button(std::string name, std::string label);
    void Render() override;
    bool WasClicked() const;
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    const std::string& GetLabel() const;
    void SetLabel(std::string label);
    void SetColorVariant(ColorVariant variant);
    void SetSize(Size size);

private:
    std::string label_;
    bool enabled_ = true;
    bool clicked_ = false;
    ColorVariant variant_ = Default;
    Size sz_ = Medium;
};

} // namespace unigui
