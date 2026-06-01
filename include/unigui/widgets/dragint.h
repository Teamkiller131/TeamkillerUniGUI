#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>
namespace unigui {
class DragInt : public ValueWidget<int> {
public:
    using ValueWidget::SetValue;
    using ValueWidget::GetValue;
    DragInt(std::string name, std::string label, int value = 0, float speed = 1.0f, int vmin = 0, int vmax = 0);
    void Render() override;
    bool WasChanged() const;
private: std::string label_; float speed_; int min_, max_; bool changed_;
};
}
