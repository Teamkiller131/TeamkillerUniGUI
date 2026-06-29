#include <unigui/widgets/dragfloat.h>

#include <imgui.h>
namespace unigui {
DragFloat::DragFloat(std::string n, std::string l, float v, float s, float mn, float mx)
        : ValueWidget<float>(std::move(n), v)
        , label_(std::move(l))
        , speed_(s)
        , min_(mn)
        , max_(mx)
        , changed_(false) {}
void DragFloat::Render() {
    if (!IsVisible())
        return;
    float prev = value_;
    ImGui::PushID(GetName().c_str());
    changed_ = ImGui::DragFloat(label_.c_str(), &value_, speed_, min_, max_, "%.3f");
    ImGui::PopID();
    if (value_ < min_)
        value_ = min_;
    if (value_ > max_)
        value_ = max_;
    ReportAccessible(a11y::Role::Slider, ImGui::IsItemFocused(), std::to_string(value_));
    NotifyChange(prev);
}
bool DragFloat::WasChanged() const {
    return changed_;
}
} // namespace unigui
