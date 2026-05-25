#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>
namespace unigui {
class ToggleSwitch : public Widget {
public:
    ToggleSwitch(std::string name, std::string label, bool on = false);
    void Render() override;
    bool IsOn() const; void SetOn(); void SetOff(); void Toggle();
    void SetOnChange(std::function<void(bool)> cb);
private: std::string label_; bool on_; std::function<void(bool)> on_change_;
};
}
