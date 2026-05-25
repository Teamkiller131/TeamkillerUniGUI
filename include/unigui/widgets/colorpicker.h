#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>
#include <array>

namespace unigui {
class ColorPicker : public Widget {
public:
    ColorPicker(std::string name, std::string label, std::array<float, 3> color = {0.0f,0.0f,0.0f});
    void Render() override;
    std::array<float, 3> GetColor() const;
    void SetColor(std::array<float, 3> color);
    void SetOnChange(std::function<void(std::array<float,3>)> callback);
private:
    std::string label_;
    std::array<float, 3> color_;
    std::function<void(std::array<float,3>)> on_change_;
};
}
