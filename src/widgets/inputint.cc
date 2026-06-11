#include <unigui/widgets/inputint.h>

#include <imgui.h>
namespace unigui {
InputInt::InputInt(std::string n, std::string l, int v, int mn, int mx)
        : ValueWidget<int>(std::move(n), v)
        , label_(std::move(l))
        , min_(mn)
        , max_(mx) {}
void InputInt::Render() {
    if (!IsVisible())
        return;
    int prev = value_;
    ImGui::PushID(GetName().c_str());
    const bool disabled = !IsEnabled();
    if (disabled)
        ImGui::BeginDisabled();
    ImGui::InputInt(label_.c_str(), &value_, 0, 0, ImGuiInputTextFlags_None);
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
void InputInt::SetRange(int mn, int mx) {
    min_ = mn;
    max_ = mx;
}
void InputInt::SetSuffix(std::string s) {
    suffix_ = std::move(s);
}
} // namespace unigui
