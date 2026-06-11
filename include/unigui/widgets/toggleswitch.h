#pragma once
#include <unigui/fx/animation.h>
#include <unigui/widgets/value_widget.h>

#include <string>

namespace unigui {
class ToggleSwitch : public ValueWidget<bool> {
public:
    using ValueWidget::GetValue;
    using ValueWidget::SetValue;

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
} // namespace unigui
