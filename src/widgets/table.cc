#include <unigui/widgets/table.h>

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string_view>

namespace unigui {

namespace {

std::string Trim(std::string_view value) {
    size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])))
        ++first;
    size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])))
        --last;
    return std::string(value.substr(first, last - first));
}

// Non-allocating trim: a view into `value` with leading/trailing ASCII whitespace
// removed. Used on the per-cell render path so the common case allocates nothing.
std::string_view TrimView(std::string_view value) {
    size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])))
        ++first;
    size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])))
        --last;
    return value.substr(first, last - first);
}

bool EndsWith(std::string_view value, std::string_view suffix) {
    return value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix;
}

// Returns the cell text to draw as a view. On the common path (no unit, or the value
// already carries the unit) it is a view into `raw` with NO allocation; only when the
// unit must be appended is `scratch` filled and returned. `scratch` must outlive the
// returned view.
std::string_view FormatCellText(std::string_view raw, std::string_view unit, std::string& scratch) {
    if (raw.empty() || unit.empty())
        return raw;
    const std::string_view trimmed = TrimView(raw);
    if (trimmed.empty() || EndsWith(trimmed, unit))
        return raw;
    scratch.assign(trimmed.data(), trimmed.size());
    scratch.append(unit.data(), unit.size());
    return scratch;
}

bool ParseNumericCell(std::string_view text, std::string_view unit, double& out) {
    std::string trimmed = Trim(text);
    if (trimmed.empty())
        return false;
    // std::strtod is non-throwing — unlike std::stod it never allocates an
    // exception per non-numeric cell, which matters because this runs once per
    // row on every sort (and keeps the parser within the project's "no throwing
    // parsers" rule). We use strtod rather than std::from_chars because libc++
    // (macOS) does not implement the floating-point from_chars overload — it is
    // a deleted function there. strtod accepts a leading '+'/'-' and whitespace,
    // and reports the first unparsed character via its end pointer, which we use
    // to validate the trailing unit. `trimmed` is NUL-terminated (std::string).
    const char* begin = trimmed.c_str();
    char* parseEnd = nullptr;
    out = std::strtod(begin, &parseEnd);
    if (parseEnd == begin)
        return false; // not a number
    std::string rest = Trim(std::string_view(parseEnd));
    if (!unit.empty()) {
        return rest.empty() || rest == unit;
    }
    return std::none_of(rest.begin(), rest.end(),
                        [](unsigned char ch) { return std::isdigit(ch); });
}

float AlignedOffset(float availableWidth, float contentWidth, Table::Alignment alignment) {
    const float slack = std::max(0.0f, availableWidth - contentWidth);
    switch (alignment) {
    case Table::Alignment::Center:
        return slack * 0.5f;
    case Table::Alignment::Right:
        return slack;
    case Table::Alignment::Left:
    default:
        return 0.0f;
    }
}

// Take std::string_view (not const std::string&) and use ImGui's begin/end text
// overloads, so the per-cell display string never has to be materialised/copied.
void DrawAlignedText(std::string_view text, float width, Table::Alignment alignment) {
    const char* tb = text.data();
    const char* te = text.data() + text.size();
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const ImVec2 size = ImGui::CalcTextSize(tb, te);
    const float lineHeight = std::max(ImGui::GetTextLineHeight(), size.y);
    ImGui::Dummy(ImVec2(width, lineHeight));
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(start, ImVec2(start.x + width, start.y + lineHeight), true);
    const float x = start.x + AlignedOffset(width, size.x, alignment);
    const float y = start.y + std::max(0.0f, (lineHeight - size.y) * 0.5f);
    draw->AddText(ImVec2(x, y), ImGui::GetColorU32(ImGuiCol_Text), tb, te);
    draw->PopClipRect();
}

bool DrawSelectableAlignedText(std::string_view text, bool selected, float width,
                               Table::Alignment alignment) {
    const char* tb = text.data();
    const char* te = text.data() + text.size();
    ImGui::SetNextItemAllowOverlap();
    const bool clicked =
        ImGui::Selectable("##cell", selected,
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
    const ImVec2 start = ImGui::GetItemRectMin();
    const ImVec2 end = ImGui::GetItemRectMax();
    const ImVec2 size = ImGui::CalcTextSize(tb, te);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(start, ImVec2(start.x + width, end.y), true);
    const float x = start.x + AlignedOffset(width, size.x, alignment);
    const float y = start.y + std::max(0.0f, ((end.y - start.y) - size.y) * 0.5f);
    draw->AddText(ImVec2(x, y), ImGui::GetColorU32(ImGuiCol_Text), tb, te);
    draw->PopClipRect();
    return clicked;
}

} // namespace

Table::Table(std::string name, std::vector<std::string> columns)
        : Widget(std::move(name))
        , columns_(std::move(columns))
        , alignments_(columns_.size(), Alignment::Left)
        , units_(columns_.size()) {}
void Table::AddRow(std::vector<std::string> row) {
    rows_.push_back(std::move(row));
}
void Table::ClearRows() {
    rows_.clear();
}
int Table::GetSelectedRow() const {
    return selected_;
}
void Table::SetOnSelect(std::function<void(int)> callback) {
    on_select_ = std::move(callback);
}
void Table::SetSortable(bool on) {
    sortable_ = on;
}
void Table::SetResizable(bool on) {
    resizable_ = on;
}
void Table::SortByColumn(int col, bool ascending) {
    ApplySort(col, ascending);
}
void Table::SetColumnAlignment(int col, Alignment alignment) {
    if (col < 0 || col >= (int) alignments_.size())
        return;
    alignments_[col] = alignment;
}
Table::Alignment Table::GetColumnAlignment(int col) const {
    return (col >= 0 && col < (int) alignments_.size()) ? alignments_[col] : Alignment::Left;
}
void Table::SetColumnUnit(int col, std::string unit) {
    if (col < 0 || col >= (int) units_.size())
        return;
    units_[col] = std::move(unit);
}
const std::string& Table::GetColumnUnit(int col) const {
    static const std::string kEmpty;
    return (col >= 0 && col < (int) units_.size()) ? units_[col] : kEmpty;
}
void Table::SetColumnWidth(int col, float width) {
    if (col < 0)
        return;
    if (col >= (int) col_widths_.size())
        col_widths_.resize(col + 1, 0.0f);
    col_widths_[col] = width;
}
float Table::GetColumnWidth(int col) const {
    if (col < 0 || col >= (int) col_widths_.size())
        return 0.0f;
    return col_widths_[col];
}
void Table::SetColumnStretch(int col, float weight) {
    if (col < 0)
        return;
    if (col >= (int) col_stretches_.size())
        col_stretches_.resize(col + 1, 0.0f);
    col_stretches_[col] = weight;
}
float Table::GetColumnStretch(int col) const {
    if (col < 0 || col >= (int) col_stretches_.size())
        return 0.0f;
    return col_stretches_[col];
}

const std::string& Table::CellText(int row, int col) const {
    static const std::string kEmpty;
    if (row < 0 || row >= (int) rows_.size())
        return kEmpty;
    if (col < 0 || col >= (int) rows_[row].size())
        return kEmpty;
    return rows_[row][col];
}

void Table::ApplySort(int col, bool ascending) {
    if (col < 0 || col >= (int) columns_.size())
        return;
    static const std::string kEmpty;
    auto cell = [&](const std::vector<std::string>& r) -> const std::string& {
        return col < (int) r.size() ? r[col] : kEmpty;
    };

    auto it = sort_comparators_.find(col);
    if (it != sort_comparators_.end()) {
        // Custom comparator: sort rows directly.
        const SortComparator& cmp = it->second;
        std::stable_sort(rows_.begin(), rows_.end(),
                         [&](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                             bool less = cmp(cell(a), cell(b));
                             bool greater = cmp(cell(b), cell(a));
                             if (!less && !greater)
                                 return false; // equal — keep stable order
                             return ascending ? less : greater;
                         });
    } else {
        // Numeric-aware default: parse each cell once into a sort key, then
        // sort an index permutation to avoid re-parsing on every comparison.
        const std::string& unit = GetColumnUnit(col);
        struct Key {
            bool isNum;
            double num;
        };
        const int n = (int) rows_.size();
        std::vector<Key> keys(n);
        std::vector<int> order(n);
        for (int i = 0; i < n; ++i) {
            order[i] = i;
            double v = 0;
            keys[i].isNum = ParseNumericCell(cell(rows_[i]), unit, v);
            keys[i].num = v;
        }
        std::stable_sort(order.begin(), order.end(), [&](int ia, int ib) {
            const Key& ka = keys[ia];
            const Key& kb = keys[ib];
            bool less;
            if (ka.isNum && kb.isNum)
                less = ka.num < kb.num;
            else
                less = cell(rows_[ia]) < cell(rows_[ib]);
            bool greater;
            if (ka.isNum && kb.isNum)
                greater = kb.num < ka.num;
            else
                greater = cell(rows_[ib]) < cell(rows_[ia]);
            if (!less && !greater)
                return false;
            return ascending ? less : greater;
        });
        std::vector<std::vector<std::string>> sorted;
        sorted.reserve(n);
        for (int idx : order)
            sorted.push_back(std::move(rows_[idx]));
        rows_ = std::move(sorted);
    }
    // Row order changed — drop the now-stale selection index.
    selected_ = -1;
}

void Table::SaveColumnWidths() {
    saved_widths_.clear();
    if (int n = ImGui::TableGetColumnCount()) {
        for (int c = 0; c < n && c < (int) columns_.size(); c++)
            saved_widths_.push_back(ImGui::GetColumnWidth(c));
    }
}
void Table::RestoreColumnWidths() {
    if (int n = ImGui::TableGetColumnCount()) {
        for (int c = 0; c < n && c < (int) saved_widths_.size(); c++)
            ImGui::SetColumnWidth(c, saved_widths_[c]);
    }
}
void Table::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (scrollX_)
        flags |= ImGuiTableFlags_ScrollX;
    if (sortable_)
        flags |= ImGuiTableFlags_Sortable;
    if (resizable_)
        flags |= ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable(GetName().c_str(), (int) columns_.size(), flags)) {
        for (int ci = 0; ci < (int) columns_.size(); ++ci) {
            float cs = (ci < (int) col_stretches_.size()) ? col_stretches_[ci] : 0.0f;
            float cw = (ci < (int) col_widths_.size()) ? col_widths_[ci] : 0.0f;
            ImGuiTableColumnFlags colFlags;
            float initWidth;
            if (cs > 0.0f) {
                colFlags = ImGuiTableColumnFlags_WidthStretch;
                initWidth = cs;
            } else if (cw > 0.0f) {
                colFlags = ImGuiTableColumnFlags_WidthFixed;
                initWidth = cw;
            } else {
                colFlags = ImGuiTableColumnFlags_WidthStretch;
                initWidth = 0.0f;
            }
            ImGui::TableSetupColumn(columns_[ci].c_str(), colFlags, initWidth);
        }
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

        // Virtualize the row loop with ImGuiListClipper: only the rows inside the
        // current scroll viewport are laid out and drawn, so a Table with 100k
        // rows costs about the same per frame as one with a screenful. This
        // assumes uniform row heights (single-line cells or frame-height custom
        // renderers), which is how Table lays its cells out.
        ImGuiListClipper clipper;
        clipper.Begin((int) rows_.size());
        while (clipper.Step()) {
            for (int r = clipper.DisplayStart; r < clipper.DisplayEnd; r++) {
                ImGui::TableNextRow();
                for (int c = 0; c < (int) columns_.size(); c++) {
                    ImGui::TableSetColumnIndex(c);
                    ImGui::PushID(r * 1000 + c);
                    bool handled = false;
                    if (cell_renderer_)
                        handled = cell_renderer_(r, c);
                    if (!handled) {
                        std::string scratch; // owns only the rare unit-append case
                        const std::string_view display =
                            FormatCellText(CellText(r, c), GetColumnUnit(c), scratch);
                        const float width = std::max(0.0f, ImGui::GetContentRegionAvail().x);
                        if (c == 0) {
                            if (DrawSelectableAlignedText(display, r == selected_, width,
                                                          GetColumnAlignment(c))) {
                                selected_ = r;
                                if (on_select_)
                                    on_select_(r);
                            }
                        } else {
                            DrawAlignedText(display, width, GetColumnAlignment(c));
                        }
                    }
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopID();
}

std::string Table::ExportCSV() const {
    std::ostringstream ss;
    for (size_t c = 0; c < columns_.size(); c++) {
        if (c > 0)
            ss << ",";
        ss << columns_[c];
    }
    ss << "\n";
    for (auto& row : rows_) {
        for (size_t c = 0; c < row.size(); c++) {
            if (c > 0)
                ss << ",";
            std::string cell = row[c];
            bool needQuote =
                cell.find(',') != std::string::npos || cell.find('"') != std::string::npos;
            if (needQuote) {
                ss << '"';
                for (char ch : cell) {
                    if (ch == '"')
                        ss << "\"\"";
                    else
                        ss << ch;
                }
                ss << '"';
            } else
                ss << cell;
        }
        ss << "\n";
    }
    return ss.str();
}

bool Table::ImportCSV(const std::string& csv) {
    rows_.clear();
    std::string_view sv(csv);
    bool first = true;
    size_t lineStart = 0;

    while (lineStart < sv.size()) {
        size_t lineEnd = sv.find('\n', lineStart);
        if (lineEnd == std::string_view::npos)
            lineEnd = sv.size();
        size_t realEnd = lineEnd;
        if (realEnd > lineStart && sv[realEnd - 1] == '\r')
            realEnd--;
        std::string_view line = sv.substr(lineStart, realEnd - lineStart);
        lineStart = lineEnd + 1;

        if (line.empty())
            continue;

        std::vector<std::string> cells;
        cells.reserve(8);
        size_t pos = 0;
        while (pos < line.size()) {
            if (line[pos] == '"') {
                // Quoted field: accumulate into a single string
                std::string cell;
                cell.reserve(64);
                pos++;
                while (pos < line.size()) {
                    if (line[pos] == '"') {
                        if (pos + 1 < line.size() && line[pos + 1] == '"') {
                            cell += '"';
                            pos += 2;
                        } else {
                            pos++;
                            break;
                        }
                    } else {
                        cell += line[pos];
                        pos++;
                    }
                }
                cells.push_back(std::move(cell));
            } else {
                // Unquoted field: find comma, assign as substring (zero-copy)
                size_t comma = line.find(',', pos);
                if (comma == std::string_view::npos) {
                    cells.emplace_back(line.data() + pos, line.size() - pos);
                    pos = line.size();
                } else {
                    cells.emplace_back(line.data() + pos, comma - pos);
                    pos = comma + 1;
                }
            }
            if (pos < line.size() && line[pos] == ',')
                pos++;
        }
        if (first) {
            first = false;
            continue;
        }
        rows_.push_back(std::move(cells));
    }
    return true;
}

} // namespace unigui
