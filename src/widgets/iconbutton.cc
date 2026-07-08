#include <unigui/widgets/iconbutton.h>

#include <imgui.h>
namespace unigui {
IconButton::IconButton(std::string n, std::string i, std::string l)
        : FluentWidget<IconButton>(std::move(n))
        , icon_(std::move(i))
        , label_(std::move(l)) {}
void IconButton::SetIcon(std::string i) {
    icon_ = std::move(i);
}
void IconButton::SetLabel(std::string l) {
    label_ = std::move(l);
}
void IconButton::SetEnabled(bool e) {
    enabled_ = e;
}
bool IconButton::WasClicked() const {
    return clicked_;
}
void IconButton::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::BeginDisabled(!enabled_);
    std::string display = icon_;
    if (!label_.empty())
        display += " " + label_;
    clicked_ = ImGui::Button(display.c_str());
    ImGui::EndDisabled();
    ReportAccessible(a11y::Role::Button, ImGui::IsItemFocused(), "");
    ImGui::PopID();
}
} // namespace unigui
