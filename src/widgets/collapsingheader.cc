#include <unigui/widgets/collapsingheader.h>

#include <imgui.h>
namespace unigui {
CollapsingHeader::CollapsingHeader(std::string name, std::string label, bool default_open)
        : FluentWidget<CollapsingHeader>(std::move(name))
        , label_(std::move(label))
        , open_(default_open) {}
void CollapsingHeader::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    bool was_open = open_;
    // Force imgui's stored open state to ours (so SetOpen()/default_open lead), then
    // take the RETURN value as the post-interaction expand state. The two-arg
    // overload's bool* is p_visible — a close-'X' + don't-render-at-all flag, NOT the
    // open state — so passing &open_ there made default_open=false render nothing and
    // click-collapse never update open_.
    ImGui::SetNextItemOpen(open_, ImGuiCond_Always);
    open_ = ImGui::CollapsingHeader(label_.c_str());
    ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(), open_ ? "expanded" : "collapsed");
    if (open_ != was_open && onToggle_)
        onToggle_(open_);
    if (open_ && content_callback_)
        content_callback_();
    ImGui::PopID();
}
bool CollapsingHeader::IsOpen() const {
    return open_;
}
void CollapsingHeader::SetOpen(bool open) {
    open_ = open;
}
void CollapsingHeader::SetContentCallback(std::function<void()> cb) {
    content_callback_ = std::move(cb);
}
void CollapsingHeader::SetOnToggle(std::function<void(bool)> fn) {
    onToggle_ = std::move(fn);
}
const std::string& CollapsingHeader::GetLabel() const {
    return label_;
}
} // namespace unigui
