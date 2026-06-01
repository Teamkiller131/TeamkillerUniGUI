#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>
namespace unigui {
class DragFloat : public ValueWidget<float> {
public:
    using ValueWidget::SetValue;
    using ValueWidget::GetValue;
    DragFloat(std::string name, std::string label, float value = 0.0f, float speed = 1.0f, float vmin = 0.0f, float vmax = 0.0f);
    void Render() override;
    bool WasChanged() const;
private: std::string label_; float speed_, min_, max_; bool changed_;
};
}
