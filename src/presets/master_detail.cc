#include <unigui/presets/master_detail.h>

#include <imgui.h>

#include <algorithm>

namespace unigui::presets {

namespace {
constexpr float kDefaultSplit = 0.3f;
constexpr float kMinSplit = 0.1f; // matches Splitter's own drag clamp
constexpr float kMaxSplit = 0.9f;
} // namespace

MasterDetail::MasterDetail(std::string name)
        : FluentWidget<MasterDetail>(std::move(name))
        , list_(GetName() + "##list") {
    splitter_.emplace(GetName() + "##split", Splitter::Vertical, kDefaultSplit);
    // ListView reports items to the a11y tree but does not announce clicks, so
    // the selection announcement lives here (mirrors VirtualList's wiring).
    list_.SetOnSelect([this](int i) {
        selected_ = i;
        if (i >= 0 && i < (int) items_.size())
            a11y::Announce(items_[i] + " selected");
        if (onSelect_)
            onSelect_(i);
    });
}

// ── Fluent configuration ─────────────────────────────────────────────────────

MasterDetail& MasterDetail::WithItems(std::vector<std::string> items) {
    SetItems(std::move(items));
    return *this;
}

MasterDetail& MasterDetail::WithDetail(std::function<void(int)> detail) {
    detail_ = std::move(detail);
    return *this;
}

MasterDetail& MasterDetail::WithSplit(float ratio) {
    // Splitter exposes no split setter, so rebuild it in place at the new
    // ratio (drops any divider drag, which is fine at configuration time).
    ratio = std::clamp(ratio, kMinSplit, kMaxSplit);
    splitter_.emplace(GetName() + "##split", Splitter::Vertical, ratio);
    return *this;
}

MasterDetail& MasterDetail::WithEmptyText(std::string text) {
    emptyText_ = std::move(text);
    return *this;
}

MasterDetail& MasterDetail::WithOnSelect(std::function<void(int)> fn) {
    onSelect_ = std::move(fn);
    return *this;
}

// ── Live state ───────────────────────────────────────────────────────────────

void MasterDetail::SetItems(std::vector<std::string> items) {
    items_ = std::move(items);
    list_.SetItems(items_);
    // Clamp a now-invalid selection (clears when the list became empty).
    // Programmatic: deliberately does not fire onSelect_.
    if (selected_ >= (int) items_.size())
        selected_ = (int) items_.size() - 1;
}

int MasterDetail::GetSelected() const {
    return selected_;
}

void MasterDetail::SetSelected(int index) {
    const int count = (int) items_.size();
    if (index < 0 || count == 0)
        index = -1;
    else
        index = std::min(index, count - 1);
    if (index == selected_)
        return;
    selected_ = index;
    if (selected_ >= 0)
        a11y::Announce(items_[selected_] + " selected");
    // Programmatic: deliberately does not fire onSelect_.
}

float MasterDetail::GetSplit() const {
    if (!splitter_) // always emplaced in the ctor; guard satisfies
                    // bugprone-unchecked-optional-access
        return kDefaultSplit;
    return splitter_->GetSplit();
}

// ── Rendering ────────────────────────────────────────────────────────────────

void MasterDetail::RenderDetailPane() {
    const bool valid = selected_ >= 0 && selected_ < (int) items_.size();
    if (valid && detail_) {
        detail_(selected_);
        return;
    }
    // Centred, dimmed placeholder before a selection exists.
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 text = ImGui::CalcTextSize(emptyText_.c_str());
    ImVec2 cur = ImGui::GetCursorPos();
    cur.x += std::max(0.f, (avail.x - text.x) * 0.5f);
    cur.y += std::max(0.f, (avail.y - text.y) * 0.5f);
    ImGui::SetCursorPos(cur);
    ImGui::TextDisabled("%s", emptyText_.c_str());
}

void MasterDetail::Render() {
    if (!IsVisible())
        return;
    if (!splitter_) // always emplaced in the ctor; guard before PushID keeps the ID stack balanced
        return;
    ImGui::PushID(GetName().c_str());
    splitter_->SetContentA([this] { list_.Render(); });
    splitter_->SetContentB([this] { RenderDetailPane(); });
    splitter_->Render();
    ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(),
                     selected_ >= 0 && selected_ < (int) items_.size() ? items_[selected_]
                                                                       : std::string());
    RenderTooltip();
    ImGui::PopID();
}

} // namespace unigui::presets
