#include <unigui/widgets/colorpicker.h>
#include <imgui.h>
namespace unigui {
ColorPicker::ColorPicker(std::string name, std::string label, std::array<float, 3> color) : Widget(std::move(name)), label_(std::move(label)), color_(color) {}
void ColorPicker::Render() {
    if (!IsVisible()) return;
    auto prev = color_;
    ImGui::ColorEdit3(label_.c_str(), color_.data());
    if (color_ != prev && on_change_) on_change_(color_);
}
std::array<float, 3> ColorPicker::GetColor() const { return color_; }
void ColorPicker::SetColor(std::array<float, 3> color) { color_ = color; }
void ColorPicker::SetOnChange(std::function<void(std::array<float,3>)> callback) { on_change_ = std::move(callback); }
}
