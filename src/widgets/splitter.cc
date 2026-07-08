#include <unigui/widgets/splitter.h>

#include <imgui.h>
namespace unigui {
Splitter::Splitter(std::string n, Orientation o, float s)
        : FluentWidget<Splitter>(std::move(n))
        , ori_(o)
        , split_(s) {}
float Splitter::GetSplit() const {
    return split_;
}
void Splitter::SetContentA(std::function<void()> cb) {
    cbA_ = std::move(cb);
}
void Splitter::SetContentB(std::function<void()> cb) {
    cbB_ = std::move(cb);
}
void Splitter::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    auto avail = ImGui::GetContentRegionAvail();
    float sizeA = (ori_ == Horizontal ? avail.y : avail.x) * split_;
    ImGuiID id = ImGui::GetID(GetName().c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    if (ori_ == Horizontal) {
        ImGui::BeginChild(id + 1, ImVec2(avail.x, sizeA), ImGuiChildFlags_Borders);
        if (cbA_)
            cbA_();
        ImGui::EndChild();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1));
        ImGui::Button("##split", ImVec2(avail.x, 4));
        ImGui::PopStyleColor();
        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.y;
            split_ += (delta / avail.y);
            if (split_ < 0.1f)
                split_ = 0.1f;
            if (split_ > 0.9f)
                split_ = 0.9f;
        }
        ImGui::BeginChild(id + 2, ImVec2(avail.x, avail.y - sizeA - 8), ImGuiChildFlags_Borders);
        if (cbB_)
            cbB_();
        ImGui::EndChild();
    } else {
        float sizeX = avail.x * split_;
        ImGui::BeginChild(id + 1, ImVec2(sizeX, avail.y), ImGuiChildFlags_Borders);
        if (cbA_)
            cbA_();
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1));
        ImGui::Button("##vsplit", ImVec2(4, avail.y));
        ImGui::PopStyleColor();
        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.x;
            split_ += (delta / avail.x);
            if (split_ < 0.1f)
                split_ = 0.1f;
            if (split_ > 0.9f)
                split_ = 0.9f;
        }
        ImGui::SameLine();
        ImGui::BeginChild(id + 2, ImVec2(avail.x - sizeX - 12, avail.y), ImGuiChildFlags_Borders);
        if (cbB_)
            cbB_();
        ImGui::EndChild();
    }
    ImGui::PopStyleVar();
    ImGui::PopID();
}
} // namespace unigui
