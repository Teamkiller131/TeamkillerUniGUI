#include <unigui/widgets/dialog.h>

#include <imgui.h>
namespace unigui {
Dialog::Dialog(std::string name, std::string title, std::string message)
        : FluentWidget<Dialog>(std::move(name))
        , title_(std::move(title))
        , message_(std::move(message)) {}
void Dialog::Render() {
    if (!open_)
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::OpenPopup(title_.c_str());
    if (ImGui::BeginPopupModal(title_.c_str(), &open_)) {
        ImGui::TextUnformatted(message_.c_str());
        okClicked_ = false;
        if (ImGui::Button(okText_.c_str())) {
            okClicked_ = true;
            if (onOk_)
                onOk_();
            open_ = false;
            ImGui::CloseCurrentPopup();
        }
        if (!cancelText_.empty()) {
            ImGui::SameLine();
            if (ImGui::Button(cancelText_.c_str())) {
                if (onCancel_)
                    onCancel_();
                open_ = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}
void Dialog::Open() {
    open_ = true;
}
void Dialog::Close() {
    open_ = false;
}
bool Dialog::IsOpen() const {
    return open_;
}
void Dialog::SetButtons(std::string ok, std::string cancel) {
    okText_ = std::move(ok);
    cancelText_ = std::move(cancel);
}
void Dialog::SetOnOk(std::function<void()> cb) {
    onOk_ = std::move(cb);
}
void Dialog::SetOnCancel(std::function<void()> cb) {
    onCancel_ = std::move(cb);
}
bool Dialog::WasOkClicked() const {
    return okClicked_;
}
} // namespace unigui
