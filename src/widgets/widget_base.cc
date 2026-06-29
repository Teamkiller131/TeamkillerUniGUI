#include <unigui/fx/elevation.h>
#include <unigui/widgets/widget_base.h>

#include <imgui_internal.h>

namespace unigui {

Widget::Widget(std::string name)
        : name_(std::move(name))
        , enabled_(true)
        , userData_(nullptr) {}

void Widget::Show() {
    visible_ = true;
}
void Widget::Hide() {
    visible_ = false;
}
bool Widget::IsVisible() const {
    return visible_;
}

const std::string& Widget::GetName() const {
    return name_;
}

ImGuiID Widget::GetID() const {
    return ImHashStr(name_.c_str(), name_.size(), 0);
}

void Widget::SetTooltip(std::string text) {
    tooltip_ = std::move(text);
}
void Widget::SetFocused() {
    ImGui::SetKeyboardFocusHere();
    focused_ = true;
}
bool Widget::IsFocused() const {
    return focused_;
}
void Widget::SetNextFocused() {
    ImGui::SetNextItemWidth(-1);
    ImGui::SetKeyboardFocusHere();
}
void Widget::SetAccessibleName(std::string name) {
    accessibleName_ = std::move(name);
}
void Widget::SetAccessibleDescription(std::string desc) {
    accessibleDesc_ = std::move(desc);
}
void Widget::AnnounceAccessible(a11y::Role role, const std::string& value) {
    if (!a11y::IsEnabled())
        return; // zero cost when a11y is off
    a11y::SetFocused(
        {accessibleName_.empty() ? name_ : accessibleName_, accessibleDesc_, value, role});
}
void Widget::ReportAccessible(a11y::Role role, bool focused, const std::string& value,
                              bool disabled) {
    if (!a11y::IsEnabled())
        return; // zero cost when a11y is off
    // AddNode registers the element in this frame's tree and, when focused, forwards it
    // to SetFocused() (which fires the focus-changed announcement only on change).
    a11y::AddNode({accessibleName_.empty() ? name_ : accessibleName_, accessibleDesc_, value, role,
                   focused, disabled});
}
void Widget::SetMinSize(float w, float h) {
    minSize_ = ImVec2(w, h);
}
void Widget::SetMaxSize(float w, float h) {
    maxSize_ = ImVec2(w, h);
}
void Widget::SetShadow(bool enable, float radius, float offX, float offY) {
    shadow_.enabled = enable;
    if (radius > 0)
        shadow_.radius = radius;
    shadow_.offX = offX;
    shadow_.offY = offY;
}

void Widget::SetElevation(fx::Elevation level) {
    const fx::ElevationTokens t = fx::ElevationPreset(level);
    if (level == fx::Elevation::None) {
        shadow_.enabled = false;
        return;
    }
    shadow_.enabled = true;
    shadow_.radius = t.shadow_radius;
    shadow_.offX = t.shadow_offset_x;
    shadow_.offY = t.shadow_offset_y;
    shadow_.samples = t.shadow_samples;
    shadow_.color = IM_COL32(0, 0, 0, t.shadow_alpha);
}

} // namespace unigui
