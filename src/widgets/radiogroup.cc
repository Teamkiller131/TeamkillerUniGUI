#include <unigui/widgets/radiogroup.h>

#include <imgui.h>
namespace unigui {
RadioGroup::RadioGroup(std::string name, std::vector<std::string> options, int selected)
        : Widget(std::move(name))
        , options_(std::move(options))
        , selected_(selected) {}
void RadioGroup::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    bool grpFocused = false;
    for (int i = 0; i < (int) options_.size(); i++) {
        if (ImGui::RadioButton(options_[i].c_str(), &selected_, i)) {
            if (on_change_)
                on_change_(selected_);
        }
        grpFocused |= ImGui::IsItemFocused();
    }
    ReportAccessible(a11y::Role::Radio, grpFocused,
                     (selected_ >= 0 && selected_ < (int) options_.size()) ? options_[selected_]
                                                                           : std::string{});
    ImGui::PopID();
}
int RadioGroup::GetSelected() const {
    return selected_;
}
void RadioGroup::SetSelected(int index) {
    selected_ = index;
}
const std::vector<std::string>& RadioGroup::GetOptions() const {
    return options_;
}
void RadioGroup::SetOnChange(std::function<void(int)> callback) {
    on_change_ = std::move(callback);
}
} // namespace unigui
