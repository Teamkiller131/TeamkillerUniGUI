#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>
namespace unigui {
class InputFloat : public Widget {
public:
    InputFloat(std::string name, std::string label, float value = 0.0f, float min = 0.0f, float max = 100.0f);
    void Render() override;
    float GetValue() const; void SetValue(float v);
    void SetRange(float min, float max);
    void SetFormat(const char* fmt);
    void SetOnChange(std::function<void(float)> cb);
    void SetSuffix(std::string s);
private: std::string label_; float val_, min_, max_; const char* fmt_ = "%.3f"; std::function<void(float)> on_change_;
    std::string suffix_;
};
}
