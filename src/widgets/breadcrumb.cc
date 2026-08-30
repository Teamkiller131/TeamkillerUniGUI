#include <unigui/widgets/breadcrumb.h>

#include <imgui.h>
namespace unigui {
Breadcrumb::Breadcrumb(std::string n)
        : FluentWidget<Breadcrumb>(std::move(n)) {}
void Breadcrumb::SetItems(std::vector<std::string> i) {
    items_ = std::move(i);
}
int Breadcrumb::GetSelected() const {
    return selected_;
}
void Breadcrumb::SetOnSelect(std::function<void(int)> cb) {
    on_select_ = std::move(cb);
}
void Breadcrumb::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    for (int i = 0; i < (int) items_.size(); i++) {
        if (i > 0) {
            ImGui::SameLine();
            ImGui::TextUnformatted(">");
            ImGui::SameLine();
        }
        if (ImGui::SmallButton(items_[i].c_str())) {
            selected_ = i;
            if (on_select_)
                on_select_(i);
        }
    }
    ImGui::PopID();
}
} // namespace unigui
