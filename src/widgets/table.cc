#include <unigui/widgets/table.h>
#include <imgui.h>
namespace unigui {
Table::Table(std::string name, std::vector<std::string> columns) : Widget(std::move(name)), columns_(std::move(columns)) {}
void Table::AddRow(std::vector<std::string> row) { rows_.push_back(std::move(row)); }
void Table::ClearRows() { rows_.clear(); }
int Table::GetSelectedRow() const { return selected_; }
void Table::SetOnSelect(std::function<void(int)> callback) { on_select_ = std::move(callback); }
void Table::SetSortable(bool on) { sortable_ = on; }
void Table::SetResizable(bool on) { resizable_ = on; }
void Table::SaveColumnWidths() {
    saved_widths_.clear();
    // Note: Must be called while a table is active (inside Render/callback).
    if (int n = ImGui::TableGetColumnCount()) {
        for (int c = 0; c < n && c < (int)columns_.size(); c++)
            saved_widths_.push_back(ImGui::GetColumnWidth(c));
    }
}
void Table::RestoreColumnWidths() {
    // Note: Must be called while a table is active (inside Render/callback).
    if (int n = ImGui::TableGetColumnCount()) {
        for (int c = 0; c < n && c < (int)saved_widths_.size(); c++)
            ImGui::SetColumnWidth(c, saved_widths_[c]);
    }
}
void Table::Render() {
    if (!IsVisible()) return;
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (sortable_) flags |= ImGuiTableFlags_Sortable;
    if (resizable_) flags |= ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable(GetName().c_str(), (int)columns_.size(), flags)) {
        for (auto& col : columns_) { ImGui::TableSetupColumn(col.c_str()); }
        ImGui::TableHeadersRow();
        for (int r = 0; r < (int)rows_.size(); r++) {
            ImGui::TableNextRow();
            for (int c = 0; c < (int)rows_[r].size() && c < (int)columns_.size(); c++) {
                ImGui::TableSetColumnIndex(c);
                if (c == 0) {
                    if (ImGui::Selectable(rows_[r][c].c_str(), r == selected_, ImGuiSelectableFlags_SpanAllColumns)) {
                        selected_ = r; if (on_select_) on_select_(r);
                    }
                } else { ImGui::TextUnformatted(rows_[r][c].c_str()); }
            }
        }
        ImGui::EndTable();
    }
}
}
