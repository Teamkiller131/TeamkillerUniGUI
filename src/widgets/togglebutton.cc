#include <unigui/widgets/togglebutton.h>

#include <imgui.h>

namespace unigui {

ToggleButton::ToggleButton(std::string name, std::string offLabel, std::string onLabel)
        : FluentWidget<ToggleButton>(std::move(name))
        , offLabel_(std::move(offLabel))
        , onLabel_(std::move(onLabel)) {}

void ToggleButton::Render() {
    toggled_ = false;
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const bool enabled = IsEnabled() && (!enabledPred_ || enabledPred_());
    const std::string label = on_ ? onLabel_ : offLabel_;

    const ImVec4 base = theme::GetSemanticColor(on_ ? onColor_ : offColor_);
    auto scale = [](ImVec4 c, float f) {
        return ImVec4(c.x * f, c.y * f, c.z * f, c.w);
    };
    ImGui::PushStyleColor(ImGuiCol_Button, base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, scale(base, 1.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, scale(base, 0.85f));

    if (!enabled)
        ImGui::BeginDisabled();
    // ## keeps the ImGui id stable as the visible label flips off/on.
    const bool pressed = ImGui::Button((label + "##tb").c_str(), size_);
    const bool itemFocused = ImGui::IsItemFocused();
    if (!enabled)
        ImGui::EndDisabled();

    ImGui::PopStyleColor(3);

    if (!enabled && !disabledTooltip_.empty() &&
        ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%s", disabledTooltip_.c_str());
    }

    if (pressed && enabled) {
        on_ = !on_;
        toggled_ = true;
        if (onToggle_)
            onToggle_(on_);
    }

    ReportAccessible(a11y::Role::Toggle, itemFocused, on_ ? "on" : "off");

    ImGui::PopID();
}

} // namespace unigui
