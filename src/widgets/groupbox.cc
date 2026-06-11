#include <unigui/widgets/groupbox.h>

#include <imgui.h>
namespace unigui {
GroupBox::GroupBox(std::string name, std::string title)
        : Widget(std::move(name))
        , title_(std::move(title)) {}
void GroupBox::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::BeginGroup();
    if (!title_.empty())
        ImGui::TextUnformatted(title_.c_str());
    if (content_callback_)
        content_callback_();
    ImGui::EndGroup();
    ImGui::PopID();
}
void GroupBox::SetTitle(std::string title) {
    title_ = std::move(title);
}
void GroupBox::SetContentCallback(std::function<void()> callback) {
    content_callback_ = std::move(callback);
}
} // namespace unigui
