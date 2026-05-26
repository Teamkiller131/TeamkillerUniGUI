#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <imgui.h>

namespace unigui {

/// DataTable<T> — high-performance data table with virtual scrolling, sorting,
/// row coloring, cell formatting, inline editing, and text filtering.
/// References external data via pointer.
template<typename T>
class DataTable : public Widget {
public:
    struct ColumnDef {
        std::string name;
        float width = 100.f;
        bool sortable = true;
        bool resizable = true;
    };

    using CellFormatter = std::function<std::string(int row, int col, const T&)>;
    using RowColorFn     = std::function<ImU32(int row, const T&)>;
    using SortCompare    = std::function<bool(const T& a, const T& b)>;
    using SelectFn       = std::function<void(int row)>;
    using DoubleClickFn  = std::function<void(int row)>;
    using CellCommitFn   = std::function<void(int row, int col, const std::string& newValue)>;
    using FilterFn       = std::function<bool(int row, const T&)>;

    DataTable(std::string name, std::vector<ColumnDef> columns)
        : Widget(std::move(name)), columns_(std::move(columns)) {}

    // ── Data source (zero-copy pointer) ───────────────────────────────────
    void SetDataSource(const std::vector<T>* data) { data_ = data; }
    const std::vector<T>* GetDataSource() const    { return data_; }

    // ── Cell rendering ────────────────────────────────────────────────────
    void SetCellFormatter(CellFormatter fmt) { cellFmt_ = std::move(fmt); }

    // ── Row color (profit/loss, etc.) ────────────────────────────────────
    void SetRowColor(RowColorFn fn) { rowColorFn_ = std::move(fn); }

    // ── Sorting ───────────────────────────────────────────────────────────
    void SetSortCompare(int col, SortCompare cmp) { sortComps_[col] = std::move(cmp); }
    int  GetSortColumn() const  { return sortColumn_; }
    bool GetSortAscending() const { return sortAscending_; }

    // ── Selection ─────────────────────────────────────────────────────────
    int  GetSelectedRow() const { return selectedRow_; }
    void SetOnSelect(SelectFn cb) { onSelect_ = std::move(cb); }
    void SetOnDoubleClick(DoubleClickFn cb) { onDblClick_ = std::move(cb); }

    // ── Inline editing ───────────────────────────────────────────────────
    /// Enable inline editing on a column. Double-click to edit.
    void SetCellEditable(int col, bool editable) { editableCols_.insert(col); }
    void SetOnCellCommit(CellCommitFn fn) { onCellCommit_ = std::move(fn); }

    // ── Filtering (text search) ──────────────────────────────────────────
    /// Set text filter string — rows not matching are hidden.
    void SetFilterText(const std::string& text) { filterText_ = text; }
    const std::string& GetFilterText() const { return filterText_; }
    /// Custom filter predicate (optional, overrides text filter).
    void SetFilterFn(FilterFn fn) { filterFn_ = std::move(fn); }

    // ── Virtual scrolling ─────────────────────────────────────────────────
    void SetVirtualScroll(bool on) { virtualScroll_ = on; }
    void ScrollToRow(int row) { scrollToRow_ = row; }

    // ── Render ────────────────────────────────────────────────────────────
    void Render() override {
        if (!IsVisible() || !data_) return;

        // ── Header ──────────────────────────────────────────────────────
        int flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                    ImGuiTableFlags_Sortable;

        float tableH = virtualScroll_ ? ImGui::GetContentRegionAvail().y : 0.f;
        if (!ImGui::BeginTable(GetName().c_str(), (int)columns_.size(), flags,
                               ImVec2(0, tableH))) return;

        for (auto& col : columns_)
            ImGui::TableSetupColumn(col.name.c_str(),
                (col.resizable ? ImGuiTableColumnFlags_None : ImGuiTableColumnFlags_NoResize)
                | (col.sortable ? ImGuiTableColumnFlags_DefaultSort : 0),
                col.width);

        ImGui::TableHeadersRow();

        // ── Sort handling ───────────────────────────────────────────────
        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
            if (sortSpecs->SpecsDirty) {
                sortColumn_    = sortSpecs->Specs->ColumnUserID;
                sortAscending_ = sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending;
                if (sortComps_.count(sortColumn_)) {
                    std::vector<int> indices(data_->size());
                    for (size_t i = 0; i < data_->size(); ++i) indices[i] = (int)i;
                    std::sort(indices.begin(), indices.end(),
                        [&](int a, int b) {
                            return sortAscending_
                                ? sortComps_[sortColumn_]((*data_)[a], (*data_)[b])
                                : sortComps_[sortColumn_]((*data_)[b], (*data_)[a]);
                        });
                    sortIndices_ = std::move(indices);
                }
                sortSpecs->SpecsDirty = false;
            }
        }
        if (sortColumn_ >= 0) {
            ImGui::TableSetColumnSortDirection(sortColumn_,
                sortAscending_ ? ImGuiSortDirection_Ascending : ImGuiSortDirection_Descending, true);
        }

        // ── Virtual scroll: clamp visible rows ───────────────────────────
        int totalRows = (int)data_->size();
        int firstRow = 0;
        int lastRow = totalRows;
        if (virtualScroll_ && totalRows > 0) {
            float rowHeight = ImGui::GetTextLineHeightWithSpacing();
            float scrollY   = ImGui::GetScrollY();
            firstRow = std::max(0, (int)(scrollY / rowHeight));
            lastRow  = std::min(totalRows, firstRow + (int)(tableH / rowHeight) + 2);
            ImGuiListClipper clipper;
            clipper.Begin(totalRows, rowHeight);
            firstRow = clipper.DisplayStart;
            lastRow  = clipper.DisplayEnd;
        }

        // ── Scroll-to-row gesture ────────────────────────────────────────
        if (scrollToRow_ >= 0 && scrollToRow_ < totalRows) {
            float rowH = ImGui::GetTextLineHeightWithSpacing();
            ImGui::SetScrollY(scrollToRow_ * rowH);
            scrollToRow_ = -1;
        }

        // ── Text filter ─────────────────────────────────────────────────
        auto rowPasses = [&](int idx) -> bool {
            const T& item = (*data_)[idx];
            if (filterFn_ && !filterFn_(idx, item)) return false;
            if (!filterText_.empty() && cellFmt_) {
                for (int c = 0; c < (int)columns_.size(); ++c) {
                    std::string cell = cellFmt_(idx, c, item);
                    if (cell.find(filterText_) != std::string::npos) return true;
                }
                return false;
            }
            return true;
        };

        // ── Render rows ──────────────────────────────────────────────────
        for (int row = firstRow; row < lastRow; ++row) {
            int idx = (sortColumn_ >= 0 && !sortIndices_.empty())
                        ? sortIndices_[row] : row;
            if (!rowPasses(idx)) continue;

            ImGui::TableNextRow();
            if (rowColorFn_) {
                ImU32 bg = rowColorFn_(idx, (*data_)[idx]);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg);
            }

            // Selectable row
            bool isSelected = (selectedRow_ == idx);
            for (int col = 0; col < (int)columns_.size(); ++col) {
                ImGui::TableSetColumnIndex(col);
                std::string text = cellFmt_ ? cellFmt_(idx, col, (*data_)[idx])
                                            : std::to_string(idx);

                // ── Inline editing: InputText popup ────────────────────
                bool isEditing = (editRow_ == idx && editCol_ == col);
                if (isEditing) {
                    ImGui::SetKeyboardFocusHere();
                    if (ImGui::InputText("##edit", editBuf_, sizeof(editBuf_),
                                         ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (onCellCommit_ && strlen(editBuf_) > 0)
                            onCellCommit_(idx, col, std::string(editBuf_));
                        editRow_ = editCol_ = -1;
                    }
                    if (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape))
                        editRow_ = editCol_ = -1;
                } else {
                    ImGuiSelectableFlags sflags = ImGuiSelectableFlags_SpanAllColumns;
                    if (editableCols_.count(col)) sflags |= ImGuiSelectableFlags_AllowDoubleClick;
                    if (ImGui::Selectable(text.c_str(), isSelected, sflags)) {
                        selectedRow_ = idx;
                        if (onSelect_) onSelect_(idx);
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            if (editableCols_.count(col)) {
                                editRow_ = idx; editCol_ = col;
                                strncpy(editBuf_, text.c_str(), sizeof(editBuf_) - 1);
                                editBuf_[sizeof(editBuf_) - 1] = 0;
                            }
                            if (onDblClick_) onDblClick_(idx);
                        }
                    }
                }
            }
        }

        ImGui::EndTable();
    }

private:
    std::vector<ColumnDef> columns_;
    const std::vector<T>* data_ = nullptr;
    CellFormatter cellFmt_;
    RowColorFn rowColorFn_;
    std::unordered_map<int, SortCompare> sortComps_;
    std::vector<int> sortIndices_;
    int sortColumn_ = -1;
    bool sortAscending_ = true;
    int selectedRow_ = -1;
    bool virtualScroll_ = true;
    int scrollToRow_ = -1;
    SelectFn onSelect_;
    DoubleClickFn onDblClick_;
    CellCommitFn onCellCommit_;
    FilterFn filterFn_;
    std::string filterText_;
    std::unordered_set<int> editableCols_;    // columns with inline edit enabled
    int editRow_ = -1, editCol_ = -1;          // currently editing cell
    char editBuf_[256] = {};
};

} // namespace unigui
