#include <unigui/widgets/table.h>
#include <imgui.h>
#include <sstream>
#include <algorithm>
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
    if (int n = ImGui::TableGetColumnCount()) {
        for (int c = 0; c < n && c < (int)columns_.size(); c++)
            saved_widths_.push_back(ImGui::GetColumnWidth(c));
    }
}
void Table::RestoreColumnWidths() {
    if (int n = ImGui::TableGetColumnCount()) {
        for (int c = 0; c < n && c < (int)saved_widths_.size(); c++)
            ImGui::SetColumnWidth(c, saved_widths_[c]);
    }
}
void Table::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
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
    ImGui::PopID();
}

std::string Table::ExportCSV() const {
    std::ostringstream ss;
    for (size_t c = 0; c < columns_.size(); c++) {
        if (c > 0) ss << ",";
        ss << columns_[c];
    }
    ss << "\n";
    for (auto& row : rows_) {
        for (size_t c = 0; c < row.size(); c++) {
            if (c > 0) ss << ",";
            std::string cell = row[c];
            bool needQuote = cell.find(',') != std::string::npos || cell.find('"') != std::string::npos;
            if (needQuote) { ss << '"'; for (char ch : cell) { if (ch == '"') ss << "\"\""; else ss << ch; } ss << '"'; }
            else ss << cell;
        }
        ss << "\n";
    }
    return ss.str();
}

bool Table::ImportCSV(const std::string& csv) {
    rows_.clear();
    std::istringstream ss(csv);
    std::string line;
    bool first = true;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::vector<std::string> cells;
        size_t pos = 0;
        while (pos < line.size()) {
            std::string cell;
            if (line[pos] == '"') {
                pos++; while (pos < line.size()) {
                    if (line[pos] == '"') {
                        if (pos+1 < line.size() && line[pos+1] == '"') { cell += '"'; pos+=2; }
                        else { pos++; break; }
                    } else { cell += line[pos]; pos++; }
                }
            } else {
                size_t comma = line.find(',', pos);
                if (comma == std::string::npos) { cell = line.substr(pos); pos = line.size(); }
                else { cell = line.substr(pos, comma - pos); pos = comma + 1; }
            }
            cells.push_back(cell);
            if (pos < line.size() && line[pos] == ',') pos++;
        }
        if (first) { first = false; continue; }
        rows_.push_back(std::move(cells));
    }
    return true;
}

} // namespace unigui
