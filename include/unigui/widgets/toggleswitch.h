#pragma once
#include <unigui/widgets/value_widget.h>
#include <unigui/fx/animation.h>
#include <string>

namespace unigui {
class ToggleSwitch : public ValueWidget<bool> {
public:
    using ValueWidget::SetValue;
    using ValueWidget::GetValue;

    ToggleSwitch(std::string name, std::string label, bool on = false);
    void Render() override;

    bool IsOn() const { return GetValue(); }
    void SetOn() { SetValue(true); }
    void SetOff() { SetValue(false); }
    void Toggle() { SetValue(!GetValue()); }

    // SetOnChange inherited from ValueWidget<bool>
private:
    std::string label_;
    fx::AnimationState anim_;
};
}
