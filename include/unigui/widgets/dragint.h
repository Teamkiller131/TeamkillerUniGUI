#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
namespace unigui {
class DragInt : public Widget {
public:
    DragInt(std::string name, std::string label, int value = 0, float speed = 1.0f, int vmin = 0, int vmax = 0);
    void Render() override;
    int GetValue() const; void SetValue(int v);
    bool WasChanged() const;
private: std::string label_; int value_, min_, max_; float speed_; bool changed_;
};
}
