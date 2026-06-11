#include <unigui/widgets/listbox.h>

#include <imgui.h>

namespace unigui {

ListBox::ListBox(std::string name, std::string label, std::vector<std::string> items, int selected)
        : Widget(std::move(name))
        , label_(std::move(label))
        , items_(std::move(items))
        , selected_(selected)
        , prev_selected_(selected) {}

void ListBox::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    // Captureless lambda -> function pointer for ImGui::ListBox getter overload
    // Signature: const char* (*)(void* data, int idx)
    auto getter = [](void* data, int idx) -> const char* {
        auto& v = *static_cast<std::vector<std::string>*>(data);
        if (idx < 0 || idx >= static_cast<int>(v.size()))
            return nullptr;
        return v[idx].c_str();
    };

    ImGui::ListBox(label_.c_str(), &selected_, getter, &items_, static_cast<int>(items_.size()));

    if (selected_ != prev_selected_) {
        prev_selected_ = selected_;
        if (on_change_)
            on_change_(selected_);
    }

    ImGui::PopID();
}

int ListBox::GetSelectedIndex() const {
    return selected_;
}

void ListBox::SetSelectedIndex(int idx) {
    selected_ = idx;
    prev_selected_ = idx;
}

std::string ListBox::GetSelectedValue() const {
    if (selected_ >= 0 && selected_ < static_cast<int>(items_.size()))
        return items_[selected_];
    return {};
}

const std::vector<std::string>& ListBox::GetItems() const {
    return items_;
}

void ListBox::SetItems(std::vector<std::string> items) {
    items_ = std::move(items);
}

void ListBox::SetOnChange(std::function<void(int)> cb) {
    on_change_ = std::move(cb);
}

} // namespace unigui
