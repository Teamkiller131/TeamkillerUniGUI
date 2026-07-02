#include <unigui/presets/settings_page.h>
#include <unigui/theme/theme.h>

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <utility>

namespace unigui::presets {

namespace {
constexpr float kSectionListWidth = 160.f;  // narrow section list, px
constexpr float kControlColumnRatio = 0.4f; // label column share of the row...
constexpr float kControlColumnMin = 140.f;  // ...but never narrower than this
constexpr size_t kTextBufSize = 512;        // bounded text edit buffer

// Register one settings row in the per-frame a11y tree (no-op when a11y is
// off). Called right after the row's control was submitted, so
// ImGui::IsItemFocused() refers to that control — mirrors the per-item wiring
// in virtuallist.cc.
void ReportRow(const std::string& label, a11y::Role role, std::string value) {
    if (!a11y::IsEnabled())
        return;
    a11y::AddNode({label, "", std::move(value), role, ImGui::IsItemFocused(), false});
}
} // namespace

SettingsPage::SettingsPage(std::string name)
        : FluentWidget<SettingsPage>(std::move(name)) {}

// ── Schema building ──────────────────────────────────────────────────────────

int SettingsPage::EnsureSection() {
    if (sections_.empty())
        sections_.emplace_back("General");
    return static_cast<int>(sections_.size()) - 1;
}

SettingsPage& SettingsPage::AddSection(std::string label) {
    sections_.push_back(std::move(label));
    return *this;
}

SettingsPage& SettingsPage::AddToggle(std::string label, std::function<bool()> get,
                                      std::function<void(bool)> set) {
    Row row;
    row.kind = RowKind::Toggle;
    row.section = EnsureSection();
    row.label = std::move(label);
    row.getBool = std::move(get);
    row.setBool = std::move(set);
    rows_.push_back(std::move(row));
    return *this;
}

SettingsPage& SettingsPage::AddInt(std::string label, std::function<int()> get,
                                   std::function<void(int)> set, int min, int max) {
    Row row;
    row.kind = RowKind::Int;
    row.section = EnsureSection();
    row.label = std::move(label);
    row.getInt = std::move(get);
    row.setInt = std::move(set);
    row.intMin = min;
    row.intMax = max;
    rows_.push_back(std::move(row));
    return *this;
}

SettingsPage& SettingsPage::AddFloat(std::string label, std::function<float()> get,
                                     std::function<void(float)> set, float min, float max) {
    Row row;
    row.kind = RowKind::Float;
    row.section = EnsureSection();
    row.label = std::move(label);
    row.getFloat = std::move(get);
    row.setFloat = std::move(set);
    row.floatMin = min;
    row.floatMax = max;
    rows_.push_back(std::move(row));
    return *this;
}

SettingsPage& SettingsPage::AddCombo(std::string label, std::vector<std::string> options,
                                     std::function<int()> get, std::function<void(int)> set) {
    Row row;
    row.kind = RowKind::Combo;
    row.section = EnsureSection();
    row.label = std::move(label);
    row.options = std::move(options);
    row.getInt = std::move(get); // Combo reuses the int pair for the index
    row.setInt = std::move(set);
    rows_.push_back(std::move(row));
    return *this;
}

SettingsPage& SettingsPage::AddText(std::string label, std::function<std::string()> get,
                                    std::function<void(const std::string&)> set) {
    Row row;
    row.kind = RowKind::Text;
    row.section = EnsureSection();
    row.label = std::move(label);
    row.getText = std::move(get);
    row.setText = std::move(set);
    row.buf.resize(kTextBufSize, '\0');
    rows_.push_back(std::move(row));
    return *this;
}

SettingsPage& SettingsPage::AddAction(std::string label, std::function<void()> fn) {
    Row row;
    row.kind = RowKind::Action;
    row.section = EnsureSection();
    row.label = std::move(label);
    row.action = std::move(fn);
    rows_.push_back(std::move(row));
    return *this;
}

// ── Section selection ────────────────────────────────────────────────────────

void SettingsPage::SetActiveSection(int index) {
    if (index < 0 || index >= GetSectionCount() || index == active_)
        return;
    active_ = index;
    a11y::Announce(sections_[static_cast<size_t>(active_)] + " settings");
}

// ── Render ───────────────────────────────────────────────────────────────────

void SettingsPage::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    const bool disabled = !IsEnabled();
    if (disabled)
        BeginDisabled();
    if (GetSectionCount() > 1) {
        RenderSectionList();
        ImGui::SameLine();
    }
    RenderRows();
    if (disabled)
        EndDisabled();
    // The whole page registers as one logical group; the value carries the
    // active section so an AT user knows which page of settings is shown.
    ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(),
                     active_ < GetSectionCount() ? sections_[static_cast<size_t>(active_)] : "",
                     disabled);
    RenderTooltip();
    ImGui::PopID();
}

void SettingsPage::RenderSectionList() {
    ImGui::BeginChild("##sections", ImVec2(kSectionListWidth, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < GetSectionCount(); ++i) {
        const std::string& label = sections_[static_cast<size_t>(i)];
        ImGui::PushID(i);
        if (ImGui::Selectable(label.c_str(), i == active_))
            SetActiveSection(i); // announces the newly-shown section
        ReportRow(label, a11y::Role::ListItem, i == active_ ? "selected" : "");
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void SettingsPage::RenderRows() {
    ImGui::BeginChild("##rows", ImVec2(0, 0));
    if (active_ < GetSectionCount()) {
        // ── Themed section header ───────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, theme::GetSemanticColor(theme::Semantic::Accent));
        ImGui::SeparatorText(sections_[static_cast<size_t>(active_)].c_str());
        ImGui::PopStyleColor();
    }
    const float controlX =
        std::max(kControlColumnMin, ImGui::GetContentRegionAvail().x * kControlColumnRatio);
    for (int r = 0; r < GetRowCount(); ++r)
        if (rows_[static_cast<size_t>(r)].section == active_)
            RenderRow(r, controlX);
    ImGui::EndChild();
}

void SettingsPage::RenderRow(int rowIndex, float controlColumnX) {
    Row& row = rows_[static_cast<size_t>(rowIndex)];
    // Stable ##ids: every "##…" label below is seeded by the row index, so two
    // rows with identical user labels can never collide.
    ImGui::PushID(rowIndex);
    if (row.kind != RowKind::Action) {
        // ── Label column (left, aligned) + control column (right) ───────
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(row.label.c_str());
        ImGui::SameLine(controlColumnX);
        ImGui::SetNextItemWidth(-FLT_MIN);
    }
    switch (row.kind) {
    case RowKind::Toggle: {
        bool v = row.getBool ? row.getBool() : false;
        if (ImGui::Checkbox("##toggle", &v) && row.setBool)
            row.setBool(v);
        ReportRow(row.label, a11y::Role::Toggle, v ? "on" : "off");
        break;
    }
    case RowKind::Int: {
        int v = row.getInt ? row.getInt() : row.intMin;
        if (ImGui::SliderInt("##int", &v, row.intMin, row.intMax, "%d",
                             ImGuiSliderFlags_AlwaysClamp) &&
            row.setInt)
            row.setInt(v);
        ReportRow(row.label, a11y::Role::Slider, std::to_string(v));
        break;
    }
    case RowKind::Float: {
        float v = row.getFloat ? row.getFloat() : row.floatMin;
        if (ImGui::SliderFloat("##float", &v, row.floatMin, row.floatMax, "%.3f",
                               ImGuiSliderFlags_AlwaysClamp) &&
            row.setFloat)
            row.setFloat(v);
        char value[32];
        std::snprintf(value, sizeof(value), "%.3f", static_cast<double>(v));
        ReportRow(row.label, a11y::Role::Slider, value);
        break;
    }
    case RowKind::Combo: {
        const int count = static_cast<int>(row.options.size());
        int cur = row.getInt ? row.getInt() : 0;
        cur = std::clamp(cur, 0, count > 0 ? count - 1 : 0);
        const char* preview = count > 0 ? row.options[static_cast<size_t>(cur)].c_str() : "";
        if (ImGui::BeginCombo("##combo", preview)) {
            for (int i = 0; i < count; ++i) {
                ImGui::PushID(i);
                if (ImGui::Selectable(row.options[static_cast<size_t>(i)].c_str(), i == cur) &&
                    row.setInt)
                    row.setInt(i);
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ReportRow(row.label, a11y::Role::Combo, preview);
        break;
    }
    case RowKind::Text: {
        if (row.buf.size() < kTextBufSize)
            row.buf.resize(kTextBufSize, '\0');
        if (!row.editing && row.getText) {
            // Sync from the model only while the user isn't typing, so a live
            // edit is never clobbered; the Enter commit below writes back.
            const std::string v = row.getText();
            std::fill(row.buf.begin(), row.buf.end(), '\0');
            std::memcpy(row.buf.data(), v.data(), std::min(v.size(), row.buf.size() - 1));
        }
        const bool commit = ImGui::InputText("##text", row.buf.data(), row.buf.size(),
                                             ImGuiInputTextFlags_EnterReturnsTrue);
        row.editing = ImGui::IsItemActive();
        ReportRow(row.label, a11y::Role::Input, row.buf.data());
        if (commit && row.setText)
            row.setText(std::string(row.buf.data()));
        break;
    }
    case RowKind::Action: {
        if (ImGui::Button(row.label.c_str()) && row.action)
            row.action();
        ReportRow(row.label, a11y::Role::Button, "");
        break;
    }
    }
    ImGui::PopID();
}

} // namespace unigui::presets
