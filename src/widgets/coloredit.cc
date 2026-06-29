#include <unigui/widgets/coloredit.h>

#include <imgui.h>

namespace unigui {

ColorEdit::ColorEdit(std::string name, std::string label, float r, float g, float b, float a)
        : Widget(std::move(name))
        , label_(std::move(label))
        , changed_(false) {
    color_[0] = r;
    color_[1] = g;
    color_[2] = b;
    color_[3] = a;
}

void ColorEdit::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    changed_ = ImGui::ColorEdit4(label_.c_str(), color_);
    ReportAccessible(a11y::Role::Input, ImGui::IsItemFocused(), "");
    if (changed_ && onChange_)
        onChange_(GetColor());
    ImGui::PopID();
}

ImVec4 ColorEdit::GetColor() const {
    return ImVec4(color_[0], color_[1], color_[2], color_[3]);
}

void ColorEdit::SetColor(float r, float g, float b, float a) {
    color_[0] = r;
    color_[1] = g;
    color_[2] = b;
    color_[3] = a;
}

bool ColorEdit::WasChanged() const {
    return changed_;
}

const std::string& ColorEdit::GetLabel() const {
    return label_;
}

void ColorEdit::SetOnChange(std::function<void(ImVec4)> fn) {
    onChange_ = std::move(fn);
}

} // namespace unigui
