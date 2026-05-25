#include <unigui/widgets/widget_base.h>
#include <imgui_internal.h>

namespace unigui {

Widget::Widget(std::string name)
    : name_(std::move(name)) {
}

void Widget::Show() { visible_ = true; }
void Widget::Hide() { visible_ = false; }
bool Widget::IsVisible() const { return visible_; }

const std::string& Widget::GetName() const { return name_; }

ImGuiID Widget::GetID() const {
    return ImHashStr(name_.c_str(), name_.size(), 0);
}

void Widget::SetTooltip(std::string text) { tooltip_ = std::move(text); }
void Widget::SetFocused() { ImGui::SetKeyboardFocusHere(); focused_ = true; }
bool Widget::IsFocused() const { return focused_; }
void Widget::SetNextFocused() { ImGui::SetNextItemWidth(-1); ImGui::SetKeyboardFocusHere(); }

} // namespace unigui
