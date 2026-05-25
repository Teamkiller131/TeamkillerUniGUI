#include <unigui/widgets/colorpicker.h>
#include <imgui.h>
namespace unigui {
ColorPicker::ColorPicker(std::string name, std::string label, std::array<float, 3> color) : Widget(std::move(name)), label_(std::move(label)), color_(color) {}
void ColorPicker::Render() {
    if (!IsVisible()) return;
    auto prev = color_;
    if (has_alpha_) {
        float c4[4] = {color4_[0], color4_[1], color4_[2], color4_[3]};
        if (ImGui::ColorEdit4(label_.c_str(), c4)) {
            color_ = {c4[0],c4[1],c4[2]}; color4_ = {c4[0],c4[1],c4[2],c4[3]};
            if (color_ != prev && on_change_) on_change_(color_);
        }
    } else {
        ImGui::ColorEdit3(label_.c_str(), color_.data());
        if (color_ != prev && on_change_) on_change_(color_);
    }
}
std::array<float, 3> ColorPicker::GetColor() const { return color_; }
void ColorPicker::SetColor(std::array<float, 3> color) { color_ = color; }
void ColorPicker::SetOnChange(std::function<void(std::array<float,3>)> callback) { on_change_ = std::move(callback); }
void ColorPicker::SetAlpha(bool on) { has_alpha_ = on; }
std::array<float, 4> ColorPicker::GetColorRGBA() const { return color4_; }
void ColorPicker::SetColorRGBA(std::array<float, 4> color) { color4_ = color; color_ = {color[0],color[1],color[2]}; }
}
