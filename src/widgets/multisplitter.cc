#include <unigui/widgets/multisplitter.h>
#include <imgui.h>
#include <algorithm>

namespace unigui {

MultiSplitter::MultiSplitter(std::string name, Orientation ori)
    : Widget(std::move(name)), ori_(ori) {}

void MultiSplitter::AddPanel(float ratio, std::function<void()> content) {
    panels_.push_back({ratio, std::move(content)});
    // Normalize ratios to sum 1.0
    float total = 0.f;
    for (auto& p : panels_) total += p.ratio;
    if (total > 0.f) for (auto& p : panels_) p.ratio /= total;
}

std::vector<float> MultiSplitter::GetRatios() const {
    std::vector<float> ratios;
    ratios.reserve(panels_.size());
    for (auto& p : panels_) ratios.push_back(p.ratio);
    return ratios;
}

void MultiSplitter::SetRatios(const std::vector<float>& ratios) {
    int count = (int)std::min(panels_.size(), ratios.size());
    for (int i = 0; i < count; ++i) panels_[i].ratio = ratios[i];
    float total = 0.f;
    for (auto& p : panels_) total += p.ratio;
    if (total > 0.f) for (auto& p : panels_) p.ratio /= total;
}

void MultiSplitter::Render() {
    if (!IsVisible() || panels_.empty()) return;

    auto avail = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    float totalLen = (ori_ == Horizontal) ? avail.y : avail.x;

    for (int i = 0; i < (int)panels_.size(); ++i) {
        float panelLen = totalLen * panels_[i].ratio;

        // Panel with unique PushID
        ImGui::PushID(i);
        ImGui::BeginChild("##panel", ori_ == Horizontal ? ImVec2(avail.x, panelLen) : ImVec2(panelLen, avail.y), ImGuiChildFlags_Borders);
        if (panels_[i].content) panels_[i].content();
        ImGui::EndChild();
        ImGui::PopID();

        // Drag handle (unique ID per index)
        if (i < (int)panels_.size() - 1) {
            char handleID[64];
            snprintf(handleID, sizeof(handleID), "##ms_%s_%d", GetName().c_str(), i);
            if (ori_ == Horizontal) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1));
                ImGui::Button(handleID, ImVec2(avail.x, 5));
                ImGui::PopStyleColor();
                if (ImGui::IsItemActive()) {
                    float delta = ImGui::GetIO().MouseDelta.y;
                    panels_[i].ratio += delta / totalLen;
                    panels_[i+1].ratio -= delta / totalLen;
                    if (panels_[i].ratio < 0.05f) { panels_[i+1].ratio += panels_[i].ratio - 0.05f; panels_[i].ratio = 0.05f; }
                    if (panels_[i+1].ratio < 0.05f) { panels_[i].ratio += panels_[i+1].ratio - 0.05f; panels_[i+1].ratio = 0.05f; }
                }
            } else {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1));
                ImGui::Button(handleID, ImVec2(5, avail.y));
                ImGui::PopStyleColor();
                if (ImGui::IsItemActive()) {
                    float delta = ImGui::GetIO().MouseDelta.x;
                    panels_[i].ratio += delta / totalLen;
                    panels_[i+1].ratio -= delta / totalLen;
                    if (panels_[i].ratio < 0.05f) { panels_[i+1].ratio += panels_[i].ratio - 0.05f; panels_[i+1].ratio = 0.05f; }
                    if (panels_[i+1].ratio < 0.05f) { panels_[i].ratio += panels_[i+1].ratio - 0.05f; panels_[i+1].ratio = 0.05f; }
                }
                ImGui::SameLine();
            }
        }
    }

    ImGui::PopStyleVar();
}

} // namespace unigui
