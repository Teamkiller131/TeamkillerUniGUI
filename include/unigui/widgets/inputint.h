#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>
namespace unigui {
class InputInt : public Widget {
public:
    InputInt(std::string name, std::string label, int value = 0, int min = 0, int max = 100);
    void Render() override;
    int GetValue() const; void SetValue(int v);
    void SetRange(int min, int max);
    void SetOnChange(std::function<void(int)> cb);
    void SetSuffix(std::string s);
private: std::string label_; int val_, min_, max_; std::function<void(int)> on_change_;
    std::string suffix_;
};
}
