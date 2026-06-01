#include <unigui/widgets/confirmdialog.h>
#include <imgui.h>

namespace unigui {

ConfirmDialog::ConfirmDialog(std::string name) : Widget(std::move(name)) {}

void ConfirmDialog::Open() {
    open_ = true;
    confirmed_ = false;
    justOpened_ = true;
}

bool ConfirmDialog::WasConfirmed() const {
    return confirmed_;
}

void ConfirmDialog::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    if (!open_) { ImGui::PopID(); return; }
    if (justOpened_) { ImGui::OpenPopup("##confirm"); justOpened_ = false; }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("##confirm", &open_, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (dangerStyle_) { ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(233,69,96,255)); ImGui::BeginChild("##topbar", ImVec2(0,4)); ImGui::EndChild(); ImGui::PopStyleColor(); }
        ImGui::TextUnformatted(icon_.c_str()); ImGui::SameLine();
        ImGui::TextUnformatted(title_.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("%s", message_.c_str());
        ImGui::Spacing();
        float w = ImGui::GetWindowWidth();
        ImGui::SetCursorPosX(w - 180);
        if (ImGui::Button(cancelLabel_.c_str(), ImVec2(80, 0))) { open_ = false; }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.91f, 0.27f, 0.38f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.35f, 0.45f, 1.0f));
        if (ImGui::Button(confirmLabel_.c_str(), ImVec2(80, 0))) { confirmed_ = true; open_ = false; }
        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

} // namespace unigui
