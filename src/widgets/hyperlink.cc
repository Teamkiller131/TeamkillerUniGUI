#include <unigui/widgets/hyperlink.h>

#include <imgui.h>
#ifdef _WIN32
#include <windows.h>

#include <shellapi.h>
#endif
namespace unigui {
Hyperlink::Hyperlink(std::string n, std::string l, std::string u)
        : FluentWidget<Hyperlink>(std::move(n))
        , label_(std::move(l))
        , url_(std::move(u)) {}
void Hyperlink::SetURL(std::string u) {
    url_ = std::move(u);
}
void Hyperlink::SetLabel(std::string l) {
    label_ = std::move(l);
}
bool Hyperlink::WasClicked() const {
    return clicked_;
}
void Hyperlink::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.5f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    clicked_ = ImGui::SmallButton(label_.c_str());
    ReportAccessible(a11y::Role::Link, ImGui::IsItemFocused(), url_);
    ImGui::PopStyleColor(4);
    if (clicked_ && !url_.empty()) {
#ifdef _WIN32
        ShellExecuteA(nullptr, "open", url_.c_str(), nullptr, nullptr, SW_SHOW);
#endif
    }
    ImGui::PopID();
}
} // namespace unigui
