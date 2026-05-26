#include <unigui/widgets/wizard.h>
#include <imgui.h>

namespace unigui {

Wizard::Wizard(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {}

void Wizard::AddStep(std::string name, std::string title, std::function<void()> renderFn) {
    steps_.push_back({std::move(name), std::move(title), std::move(renderFn)});
}

void Wizard::Next() { if (curStep_ + 1 < (int)steps_.size()) curStep_++; else if (onFinish_) onFinish_(); }
void Wizard::Previous() { if (curStep_ > 0) curStep_--; }
void Wizard::GoTo(int step) { if (step >= 0 && step < (int)steps_.size()) curStep_ = step; }

void Wizard::Render() {
    if (!IsVisible() || steps_.empty()) return;
    ImGui::Begin(title_.c_str(), nullptr, ImGuiWindowFlags_NoDocking);

    // Step header
    for (int i = 0; i < (int)steps_.size(); i++) {
        if (i > 0) { ImGui::SameLine(); ImGui::Text("->"); ImGui::SameLine(); }
        bool isCur = (i == curStep_);
        if (isCur) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
        ImGui::TextUnformatted(steps_[i].title.c_str());
        if (isCur) ImGui::PopStyleColor();
    }
    ImGui::Separator();

    // Current step content
    ImGui::BeginChild("##wizcontent");
    if (curStep_ < (int)steps_.size()) steps_[curStep_].render();
    ImGui::EndChild();

    // Navigation
    ImGui::Separator();
    if (curStep_ > 0) { if (ImGui::Button("Previous")) Previous(); ImGui::SameLine(); }
    if (curStep_ < (int)steps_.size() - 1) {
        if (ImGui::Button("Next")) Next();
    } else {
        if (ImGui::Button("Finish")) { if (onFinish_) onFinish_(); }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) { if (onCancel_) onCancel_(); }

    ImGui::End();
}

} // namespace unigui
