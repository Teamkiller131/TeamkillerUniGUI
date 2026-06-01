#include <unigui/widgets/collapsingheader.h>
#include <imgui.h>
namespace unigui {
CollapsingHeader::CollapsingHeader(std::string name, std::string label, bool default_open)
    : Widget(std::move(name)), label_(std::move(label)), open_(default_open) {}
void CollapsingHeader::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    ImGui::CollapsingHeader(label_.c_str(), &open_);
    if (open_ && content_callback_) content_callback_();
    ImGui::PopID();
}
bool CollapsingHeader::IsOpen() const { return open_; }
void CollapsingHeader::SetOpen(bool open) { open_ = open; }
void CollapsingHeader::SetContentCallback(std::function<void()> cb) { content_callback_ = std::move(cb); }
const std::string& CollapsingHeader::GetLabel() const { return label_; }
}
