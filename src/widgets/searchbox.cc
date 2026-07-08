#include <unigui/widgets/searchbox.h>

#include <imgui.h>

#include <algorithm>
#include <cctype>

namespace unigui {

SearchBox::SearchBox(std::string name, std::string hint)
        : FluentWidget<SearchBox>(std::move(name))
        , hint_(std::move(hint)) {}

void SearchBox::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::SetNextItemWidth(-1);
    bool changed = ImGui::InputTextWithHint(GetName().c_str(), hint_.c_str(), buf_, sizeof(buf_));
    const bool itemFocused = ImGui::IsItemFocused();
    ReportAccessible(a11y::Role::Input, itemFocused, query_);
    if (changed) {
        query_ = buf_;
        selIdx_ = -1; // new query -> new match list, drop the old highlight
        if (onChange_)
            onChange_(query_);
    }

    // Suggestion list. A real window (not a tooltip: tooltips carry NoInputs, so
    // clicks never reached the Selectables) driven from the InputText's keyboard
    // state: Down/Up move a highlight while typing (a single-line InputText does
    // not consume the vertical arrows), Enter accepts the highlighted match on
    // the deactivation frame, Esc just closes (deactivate without Enter).
    const bool active = ImGui::IsItemActive();
    const bool deactivated = ImGui::IsItemDeactivated();
    if (!query_.empty() && (active || deactivated)) {
        const auto matches = GetMatches();
        if (!matches.empty()) {
            const int n = (int) matches.size();
            if (selIdx_ >= n)
                selIdx_ = n - 1;
            if (active) {
                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                    selIdx_ = (selIdx_ + 1) % n;
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                    selIdx_ = selIdx_ <= 0 ? n - 1 : selIdx_ - 1;

                ImGui::SetNextWindowPos(
                    ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
                ImGui::Begin("##sbsuggest", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoFocusOnAppearing);
                for (int i = 0; i < n; ++i) {
                    if (ImGui::Selectable(matches[i].c_str(), i == selIdx_))
                        Accept(matches[i]);
                }
                ImGui::End();
            }
            if (deactivated && ImGui::IsKeyPressed(ImGuiKey_Enter) && selIdx_ >= 0 && selIdx_ < n) {
                Accept(matches[selIdx_]);
            }
        }
    }
    if (!active && !deactivated)
        selIdx_ = -1; // list closed
    ImGui::PopID();
}

void SearchBox::Accept(const std::string& m) {
    query_ = m;
    snprintf(buf_, sizeof(buf_), "%s", m.c_str());
    selIdx_ = -1;
    if (onSelect_)
        onSelect_(m);
}

std::vector<std::string> SearchBox::GetMatches() const {
    std::vector<std::string> result;
    std::string q = query_;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    for (auto& item : items_) {
        std::string lower = item;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find(q) != std::string::npos)
            result.push_back(item);
        if (result.size() >= 8)
            break; // max 8 suggestions
    }
    return result;
}

} // namespace unigui
