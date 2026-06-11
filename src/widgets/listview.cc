#include <unigui/widgets/listview.h>

#include <imgui.h>

#include <algorithm>
namespace unigui {
ListView::ListView(std::string name, std::vector<std::string> items)
        : Widget(std::move(name))
        , items_(std::move(items)) {}
void ListView::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    for (int i = 0; i < (int) items_.size(); i++) {
        bool sel = multiSelect_ ? (std::find(multiSelected_.begin(), multiSelected_.end(), i) !=
                                   multiSelected_.end())
                                : (i == selected_);
        if (ImGui::Selectable(items_[i].c_str(), sel)) {
            if (multiSelect_) {
                auto it = std::find(multiSelected_.begin(), multiSelected_.end(), i);
                if (it != multiSelected_.end())
                    multiSelected_.erase(it);
                else
                    multiSelected_.push_back(i);
            } else {
                selected_ = i;
            }
            if (on_select_)
                on_select_(i);
        }
    }
    ImGui::PopID();
}
int ListView::GetSelected() const {
    return selected_;
}
void ListView::SetItems(std::vector<std::string> items) {
    items_ = std::move(items);
}
void ListView::SetOnSelect(std::function<void(int)> callback) {
    on_select_ = std::move(callback);
}
void ListView::SetMultiSelect(bool on) {
    multiSelect_ = on;
}
std::vector<int> ListView::GetSelectedItems() const {
    return multiSelect_ ? multiSelected_
                        : (selected_ >= 0 ? std::vector<int>{selected_} : std::vector<int>{});
}
} // namespace unigui
