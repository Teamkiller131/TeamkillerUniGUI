#include <unigui/widgets/datepicker.h>

#include <imgui.h>
namespace unigui {
DatePicker::DatePicker(std::string n, std::string l)
        : Widget(std::move(n))
        , label_(std::move(l)) {}
std::array<int, 3> DatePicker::GetDate() const {
    return {year_, month_, day_};
}
void DatePicker::SetDate(int y, int m, int d) {
    year_ = y;
    month_ = m;
    day_ = d;
}
void DatePicker::SetOnChange(std::function<void(int, int, int)> cb) {
    on_change_ = std::move(cb);
}
void DatePicker::Render() {
    if (!IsVisible())
        return;
    int prev[] = {year_, month_, day_};
    ImGui::PushID(GetName().c_str());
    ImGui::InputInt("Y", &year_);
    ImGui::SameLine();
    ImGui::InputInt("M", &month_);
    ImGui::SameLine();
    ImGui::InputInt("D", &day_);
    if (month_ < 1)
        month_ = 1;
    if (month_ > 12)
        month_ = 12;
    if (day_ < 1)
        day_ = 1;
    if (day_ > 31)
        day_ = 31;
    if ((year_ != prev[0] || month_ != prev[1] || day_ != prev[2]) && on_change_)
        on_change_(year_, month_, day_);
    ReportAccessible(a11y::Role::Input, ImGui::IsItemFocused(),
                     std::to_string(year_) + "-" + std::to_string(month_) + "-" + std::to_string(day_));
    ImGui::PopID();
}
} // namespace unigui
