#include <unigui/widgets/selectable.h>
#include <imgui.h>

namespace unigui {

Selectable::Selectable(std::string name, std::string label, bool selected)
    : Widget(std::move(name)), label_(std::move(label)), selected_(selected), clicked_(false) {}

void Selectable::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    clicked_ = ImGui::Selectable(label_.c_str(), &selected_);
    if (clicked_ && onClick_) onClick_();
    ImGui::PopID();
}

bool Selectable::IsSelected() const { return selected_; }
void Selectable::SetSelected(bool selected) { selected_ = selected; }
bool Selectable::WasClicked() const { return clicked_; }
void Selectable::SetOnClick(std::function<void()> fn) { onClick_ = std::move(fn); }
const std::string& Selectable::GetLabel() const { return label_; }

} // namespace unigui
