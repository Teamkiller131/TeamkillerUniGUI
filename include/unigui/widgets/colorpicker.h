#pragma once
#include <unigui/widgets/widget_base.h>

#include <array>
#include <functional>
#include <string>

namespace unigui {
class ColorPicker : public FluentWidget<ColorPicker> {
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

    // ── Fluent (chainable) helpers — return ColorPicker& via CRTP base ─────
    ColorPicker& WithColor(std::array<float, 3> color) {
        SetColor(color);
        return *this;
    }
    ColorPicker& WithOnChange(std::function<void(std::array<float, 3>)> callback) {
        SetOnChange(std::move(callback));
        return *this;
    }
    ColorPicker& WithAlpha(bool on) {
        SetAlpha(on);
        return *this;
    }
    ColorPicker& WithColorRGBA(std::array<float, 4> color) {
        SetColorRGBA(color);
        return *this;
    }

private:
    std::string label_;
    std::array<float, 3> color_;
    std::array<float, 4> color4_;
    bool has_alpha_ = false;
    std::function<void(std::array<float, 3>)> on_change_;
};
} // namespace unigui
