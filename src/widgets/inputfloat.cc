#include <unigui/widgets/inputfloat.h>

#include <imgui.h>
namespace unigui {
InputFloat::InputFloat(std::string n, std::string l, float v, float mn, float mx)
        : ValueWidget<float>(std::move(n), v)
        , label_(std::move(l))
        , min_(mn)
        , max_(mx) {}
void InputFloat::Render() {
    if (!IsVisible())
        return;
    float prev = value_;
    ImGui::PushID(GetName().c_str());
    const bool disabled = !IsEnabled();
    if (disabled)
        ImGui::BeginDisabled();
    ImGui::InputFloat(label_.c_str(), &value_, 0, 0, fmt_);
    if (!suffix_.empty()) {
        ImGui::SameLine();
        ImGui::TextUnformatted(suffix_.c_str());
    }
    if (disabled)
        ImGui::EndDisabled();
    ImGui::PopID();
    if (value_ < min_)
        value_ = min_;
    if (value_ > max_)
        value_ = max_;
    NotifyChange(prev);
}
void InputFloat::SetRange(float mn, float mx) {
    min_ = mn;
    max_ = mx;
}
void InputFloat::SetFormat(const char* f) {
    fmt_ = f;
}
void InputFloat::SetSuffix(std::string s) {
    suffix_ = std::move(s);
}
} // namespace unigui
