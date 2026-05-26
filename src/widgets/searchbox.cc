#include <unigui/widgets/searchbox.h>
#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace unigui {

SearchBox::SearchBox(std::string name, std::string hint)
    : Widget(std::move(name)), hint_(std::move(hint)) {}

void SearchBox::Render() {
    if (!IsVisible()) return;
    ImGui::SetNextItemWidth(-1);
    bool changed = ImGui::InputTextWithHint(GetName().c_str(), hint_.c_str(), buf_, sizeof(buf_));
    if (changed) {
        query_ = buf_;
        if (onChange_) onChange_(query_);
    }

    if (!query_.empty() && ImGui::IsItemActive()) {
        auto matches = GetMatches();
        if (!matches.empty()) {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
            ImGui::BeginTooltip();
            for (auto& m : matches) {
                if (ImGui::Selectable(m.c_str())) {
                    query_ = m;
                    snprintf(buf_, sizeof(buf_), "%s", m.c_str());
                    if (onSelect_) onSelect_(m);
                }
            }
            ImGui::EndTooltip();
        }
    }
}

std::vector<std::string> SearchBox::GetMatches() const {
    std::vector<std::string> result;
    std::string q = query_;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    for (auto& item : items_) {
        std::string lower = item;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find(q) != std::string::npos) result.push_back(item);
        if (result.size() >= 8) break; // max 8 suggestions
    }
    return result;
}

} // namespace unigui
