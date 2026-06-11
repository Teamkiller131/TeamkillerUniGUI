#pragma once
#include <unigui/widgets/widget_base.h>

#include <array>
#include <functional>
#include <string>

namespace unigui {
class ColorPicker : public Widget {
public:
    ColorPicker(std::string name, std::string label,
                std::array<float, 3> color = {0.0f, 0.0f, 0.0f});
    void Render() override;
    std::array<float, 3> GetColor() const;
    void SetColor(std::array<float, 3> color);
    void SetOnChange(std::function<void(std::array<float, 3>)> callback);
    void SetAlpha(bool on);
    std::array<float, 4> GetColorRGBA() const;
    void SetColorRGBA(std::array<float, 4> color);

private:
    std::string label_;
    std::array<float, 3> color_;
    std::array<float, 4> color4_;
    bool has_alpha_ = false;
    std::function<void(std::array<float, 3>)> on_change_;
};
} // namespace unigui
