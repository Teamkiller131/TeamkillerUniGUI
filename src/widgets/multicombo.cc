#include <unigui/widgets/multicombo.h>
#include <imgui.h>
#include <sstream>
#include <algorithm>

namespace unigui {

namespace {

float CalcComboWidth(const std::vector<std::string>& items, const std::string& preview) {
    float maxTextWidth = ImGui::CalcTextSize(preview.c_str()).x;
    for (const auto& item : items)
        maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(item.c_str()).x);
    const ImGuiStyle& style = ImGui::GetStyle();
    return maxTextWidth + style.FramePadding.x * 2.0f + ImGui::GetFrameHeight() + style.ItemInnerSpacing.x + 4.0f;
}

} // namespace

MultiCombo::MultiCombo(std::string name, std::string label, std::vector<std::string> items)
    : Widget(std::move(name)), label_(std::move(label)), items_(std::move(items)) {}

void MultiCombo::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    std::string preview = GetPreview();
    ImGui::SetNextItemWidth(CalcComboWidth(items_, preview));
    if (ImGui::BeginCombo(label_.c_str(), preview.c_str())) {
        for (int i = 0; i < (int)items_.size(); i++) {
            bool sel = selected_.count(i) > 0;
            if (ImGui::Checkbox(items_[i].c_str(), &sel)) {
                if (sel) selected_.insert(i); else selected_.erase(i);
                if (onChange_) onChange_();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
}

void MultiCombo::SetItems(std::vector<std::string> items) { items_ = std::move(items); selected_.clear(); }
bool MultiCombo::IsSelected(int index) const { return selected_.count(index) > 0; }
void MultiCombo::SetSelected(int index, bool sel) { if (sel) selected_.insert(index); else selected_.erase(index); }

std::vector<int> MultiCombo::GetSelectedIndices() const {
    return std::vector<int>(selected_.begin(), selected_.end());
}
void MultiCombo::SetSelectedIndices(const std::vector<int>& indices) {
    selected_.clear();
    for (int i : indices) selected_.insert(i);
}

std::string MultiCombo::GetPreview() const {
    if (selected_.empty()) return "";
    std::ostringstream ss;
    int count = 0;
    for (int i : selected_) {
        if (count > 0) ss << ", ";
        if (count >= 3) { ss << "+" << (selected_.size() - 3) << " more"; break; }
        if (i < (int)items_.size()) ss << items_[i];
        count++;
    }
    return ss.str();
}

} // namespace unigui
