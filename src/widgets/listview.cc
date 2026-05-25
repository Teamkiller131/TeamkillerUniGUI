#include <unigui/widgets/listview.h>
#include <imgui.h>
namespace unigui {
ListView::ListView(std::string name, std::vector<std::string> items) : Widget(std::move(name)), items_(std::move(items)) {}
void ListView::Render() {
    if (!IsVisible()) return;
    for (int i = 0; i < (int)items_.size(); i++) {
        if (ImGui::Selectable(items_[i].c_str(), i == selected_)) {
            selected_ = i; if (on_select_) on_select_(i);
        }
    }
}
int ListView::GetSelected() const { return selected_; }
void ListView::SetItems(std::vector<std::string> items) { items_ = std::move(items); }
void ListView::SetOnSelect(std::function<void(int)> callback) { on_select_ = std::move(callback); }
}
