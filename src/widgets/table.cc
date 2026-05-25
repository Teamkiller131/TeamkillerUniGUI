#include <unigui/widgets/table.h>
#include <imgui.h>
namespace unigui {
Table::Table(std::string name, std::vector<std::string> columns) : Widget(std::move(name)), columns_(std::move(columns)) {}
void Table::AddRow(std::vector<std::string> row) { rows_.push_back(std::move(row)); }
void Table::ClearRows() { rows_.clear(); }
int Table::GetSelectedRow() const { return selected_; }
void Table::SetOnSelect(std::function<void(int)> callback) { on_select_ = std::move(callback); }
void Table::Render() {
    if (!IsVisible()) return;
    if (ImGui::BeginTable(GetName().c_str(), (int)columns_.size(), ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
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
