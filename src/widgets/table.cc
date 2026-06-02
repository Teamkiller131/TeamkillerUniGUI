#include <unigui/widgets/table.h>
#include <imgui.h>
#include <sstream>
#include <algorithm>
#include <cctype>
namespace unigui {
Table::Table(std::string name, std::vector<std::string> columns) : Widget(std::move(name)), columns_(std::move(columns)) {}
void Table::AddRow(std::vector<std::string> row) { rows_.push_back(std::move(row)); }
void Table::ClearRows() { rows_.clear(); }
int Table::GetSelectedRow() const { return selected_; }
void Table::SetOnSelect(std::function<void(int)> callback) { on_select_ = std::move(callback); }
void Table::SetSortable(bool on) { sortable_ = on; }
void Table::SetResizable(bool on) { resizable_ = on; }

const std::string& Table::CellText(int row, int col) const {
    static const std::string kEmpty;
    if (row < 0 || row >= (int)rows_.size()) return kEmpty;
    if (col < 0 || col >= (int)rows_[row].size()) return kEmpty;
    return rows_[row][col];
}

void Table::ApplySort(int col, bool ascending) {
    if (col < 0 || col >= (int)columns_.size()) return;
    static const std::string kEmpty;
    auto cell = [&](const std::vector<std::string>& r) -> const std::string& {
        return col < (int)r.size() ? r[col] : kEmpty;
    };

    auto it = sort_comparators_.find(col);
    if (it != sort_comparators_.end()) {
        // Custom comparator: sort rows directly.
        const SortComparator& cmp = it->second;
        std::stable_sort(rows_.begin(), rows_.end(),
            [&](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                bool less = cmp(cell(a), cell(b));
                bool greater = cmp(cell(b), cell(a));
                if (!less && !greater) return false; // equal — keep stable order
                return ascending ? less : greater;
            });
    } else {
        // Numeric-aware default: parse each cell once into a sort key, then
        // sort an index permutation to avoid re-parsing on every comparison.
        auto asNumber = [](const std::string& s, double& out) -> bool {
            if (s.empty()) return false;
            try {
                size_t pos = 0;
                out = std::stod(s, &pos);
                while (pos < s.size() && std::isspace((unsigned char)s[pos])) ++pos;
                return pos == s.size();
            } catch (...) { return false; }
        };
        struct Key { bool isNum; double num; };
        const int n = (int)rows_.size();
        std::vector<Key> keys(n);
        std::vector<int> order(n);
        for (int i = 0; i < n; ++i) {
            order[i] = i;
            double v = 0;
            keys[i].isNum = asNumber(cell(rows_[i]), v);
            keys[i].num = v;
        }
        std::stable_sort(order.begin(), order.end(), [&](int ia, int ib) {
            const Key& ka = keys[ia]; const Key& kb = keys[ib];
            bool less;
            if (ka.isNum && kb.isNum) less = ka.num < kb.num;
            else less = cell(rows_[ia]) < cell(rows_[ib]);
            bool greater;
            if (ka.isNum && kb.isNum) greater = kb.num < ka.num;
            else greater = cell(rows_[ib]) < cell(rows_[ia]);
            if (!less && !greater) return false;
            return ascending ? less : greater;
        });
        std::vector<std::vector<std::string>> sorted;
        sorted.reserve(n);
        for (int idx : order) sorted.push_back(std::move(rows_[idx]));
        rows_ = std::move(sorted);
    }
    // Row order changed — drop the now-stale selection index.
    selected_ = -1;
}

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

        // ── Apply user sorting requests ───────────────────────────────
        if (sortable_) {
            if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs()) {
                if (specs->SpecsDirty && specs->SpecsCount > 0) {
                    const ImGuiTableColumnSortSpecs& s = specs->Specs[0];
                    ApplySort(s.ColumnIndex, s.SortDirection != ImGuiSortDirection_Descending);
                    specs->SpecsDirty = false;
                }
            }
        }

        for (int r = 0; r < (int)rows_.size(); r++) {
            ImGui::TableNextRow();
            for (int c = 0; c < (int)rows_[r].size() && c < (int)columns_.size(); c++) {
                ImGui::TableSetColumnIndex(c);
                ImGui::PushID(r * 1000 + c);
                bool handled = false;
                if (cell_renderer_) handled = cell_renderer_(r, c);
                if (!handled) {
                    if (c == 0) {
                        if (ImGui::Selectable(rows_[r][c].c_str(), r == selected_, ImGuiSelectableFlags_SpanAllColumns)) {
                            selected_ = r; if (on_select_) on_select_(r);
                        }
                    } else { ImGui::TextUnformatted(rows_[r][c].c_str()); }
                }
                ImGui::PopID();
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
