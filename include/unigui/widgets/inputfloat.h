#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>
namespace unigui {
class InputFloat : public ValueWidget<float> {
public:
    using ValueWidget::SetValue;
    using ValueWidget::GetValue;
    InputFloat(std::string name, std::string label, float value = 0.0f, float min = 0.0f, float max = 100.0f);
    void Render() override;
    void SetRange(float min, float max);
    void SetFormat(const char* fmt);
    void SetSuffix(std::string s);
private: std::string label_; float min_, max_; const char* fmt_ = "%.3f";
    std::string suffix_;
};
}
