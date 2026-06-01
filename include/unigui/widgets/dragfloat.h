#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
namespace unigui {
class DragFloat : public Widget {
public:
    DragFloat(std::string name, std::string label, float value = 0.0f, float speed = 1.0f, float vmin = 0.0f, float vmax = 0.0f);
    void Render() override;
    float GetValue() const; void SetValue(float v);
    bool WasChanged() const;
private: std::string label_; float value_, speed_, min_, max_; bool changed_;
};
}
