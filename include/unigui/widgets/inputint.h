#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>
namespace unigui {
class InputInt : public ValueWidget<int> {
public:
    using ValueWidget::SetValue;
    using ValueWidget::GetValue;
    InputInt(std::string name, std::string label, int value = 0, int min = 0, int max = 100);
    void Render() override;
    void SetRange(int min, int max);
    void SetSuffix(std::string s);
private: std::string label_; int min_, max_;
    std::string suffix_;
};
}
