#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <map>
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

    struct GroupInfo {
        std::string label;
        int startRow = 0, endRow = -1;
        bool expanded = true;
        int sortCol = -1;      // -1 = unsorted, 0..N = sorted by column
        bool sortAsc = true;   // sort direction
    };

    using CellFormatter = std::function<std::string(int row, int col, const T&)>;
    using RowColorFn     = std::function<ImU32(int row, const T&)>;
    using CellColorFn    = std::function<ImU32(int row, int col, const T&)>;
    using CellBoldFn     = std::function<bool(int row, int col, const T&)>;
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
    void SetCellColor(CellColorFn fn) { cellColorFn_ = std::move(fn); }
    void SetCellBold(CellBoldFn fn)   { cellBoldFn_ = std::move(fn); }

    // ── Sorting ───────────────────────────────────────────────────────────
    void SetSortCompare(int col, SortCompare cmp) { sortComps_[col] = std::move(cmp); }
    int  GetSortColumn() const  { return sortColumn_; }
    bool GetSortAscending() const { return sortAscending_; }

    // ── Selection ─────────────────────────────────────────────────────────
    void SetMultiSelect(bool on)        { multiSelect_ = on; }
    int  GetSelectedRow() const         { return selectedRows_.empty() ? -1 : selectedRows_[0]; }
    std::vector<int> GetSelectedRows() const { return selectedRows_; }
    void SetOnSelect(SelectFn cb)       { onSelect_ = std::move(cb); }
    void SetOnDoubleClick(DoubleClickFn cb) { onDblClick_ = std::move(cb); }
    void SetOnSelectionChanged(std::function<void()> cb) { onSelChanged_ = std::move(cb); }
    void SetContextMenu(std::function<void(int row)> fn) { ctxMenuFn_ = std::move(fn); }

    // ── Column auto-width ─────────────────────────────────────────────────
    void SetColumnAutoWidth(int col, bool on) {
        if (on) autoWidthCols_.insert(col); else autoWidthCols_.erase(col); }
    void SetColumnReorderable(bool on) { columnReorder_ = on; }
    void FlashRow(int row, ImU32 color, float duration) { flashRow_=row;flashColor_=color;flashDuration_=duration;flashElapsed_=0.f; }
    void SetGroups(const std::vector<GroupInfo>& groups) { groups_ = groups; }
    void ToggleGroup(int idx) { if(idx>=0&&idx<(int)groups_.size()) groups_[idx].expanded=!groups_[idx].expanded; }

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
                    ImGuiTableFlags_SizingFixedFit
                    | (columnReorder_ ? ImGuiTableFlags_Reorderable : 0)
                    | (groups_.empty() ? ImGuiTableFlags_Sortable : 0);

        float tableH = virtualScroll_ ? ImGui::GetContentRegionAvail().y : 0.f;
        if (!ImGui::BeginTable(GetName().c_str(), (int)columns_.size(), flags,
                               ImVec2(0, tableH))) return;

        for (size_t ci = 0; ci < columns_.size(); ++ci) {
            auto& col = columns_[ci];
            ImGui::TableSetupColumn(col.name.c_str(),
                (col.resizable ? ImGuiTableColumnFlags_None : ImGuiTableColumnFlags_NoResize),
                autoWidthCols_.count((int)ci) ? 0.f : col.width, (ImGuiID)ci);
        }

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
                } else if (cellFmt_) {
                    // Default sort: compare string values of the sorted column
                    std::vector<int> indices(data_->size());
                    for (size_t i = 0; i < data_->size(); ++i) indices[i] = (int)i;
                    std::sort(indices.begin(), indices.end(),
                        [&](int a, int b) {
                            int cmp = cellFmt_(a, sortColumn_, (*data_)[a])
                                      .compare(cellFmt_(b, sortColumn_, (*data_)[b]));
                            return sortAscending_ ? (cmp < 0) : (cmp > 0);
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

        auto renderRow = [&](int idx) {
            if (!rowPasses(idx)) return;
            ImGui::PushID(idx);

            if (flashRow_ == idx && flashElapsed_ < flashDuration_) {
                flashElapsed_ += ImGui::GetIO().DeltaTime;
                float a = 1.f - flashElapsed_ / flashDuration_;
                ImU32 c = (flashColor_ & 0x00FFFFFF) | ((ImU32)((flashColor_ >> 24) * a) << 24);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, c);
                if (flashElapsed_ >= flashDuration_) flashRow_ = -1;
            }

            ImGui::TableNextRow();
            if (rowColorFn_) {
                ImU32 bg = rowColorFn_(idx, (*data_)[idx]);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg);
            }

            bool isSelected = std::find(selectedRows_.begin(), selectedRows_.end(), idx) != selectedRows_.end();
            for (int col = 0; col < (int)columns_.size(); ++col) {
                ImGui::TableSetColumnIndex(col);
                std::string text = cellFmt_ ? cellFmt_(idx, col, (*data_)[idx])
                                            : std::to_string(idx);

                bool isEditing = (editRow_ == idx && editCol_ == col);
                if (isEditing) {
                    ImGui::SetKeyboardFocusHere();
                    char editLabel[64];
                    snprintf(editLabel, sizeof(editLabel), "##edit_%d_%d", idx, col);
                    if (ImGui::InputText(editLabel, editBuf_, sizeof(editBuf_),
                                         ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (onCellCommit_ && strlen(editBuf_) > 0)
                            onCellCommit_(idx, col, std::string(editBuf_));
                        editRow_ = editCol_ = -1;
                    }
                    if (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape))
                        editRow_ = editCol_ = -1;
                } else if (col == 0) {
                    ImGuiSelectableFlags sflags = ImGuiSelectableFlags_SpanAllColumns;
                    if (editableCols_.count(col)) sflags |= ImGuiSelectableFlags_AllowDoubleClick;
                    if (ImGui::Selectable(text.c_str(), isSelected, sflags)) {
                        if (multiSelect_ && ImGui::GetIO().KeyCtrl) {
                            auto it = std::find(selectedRows_.begin(), selectedRows_.end(), idx);
                            if (it != selectedRows_.end()) selectedRows_.erase(it);
                            else selectedRows_.push_back(idx);
                        } else {
                            selectedRows_.clear();
                            selectedRows_.push_back(idx);
                        }
                        if (onSelect_) onSelect_(idx);
                        if (onSelChanged_) onSelChanged_();
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            if (editableCols_.count(col)) {
                                editRow_ = idx; editCol_ = col;
                                strncpy(editBuf_, text.c_str(), sizeof(editBuf_) - 1);
                                editBuf_[sizeof(editBuf_) - 1] = 0;
                            }
                            if (onDblClick_) onDblClick_(idx);
                        }
                        if (ctxMenuFn_ && ImGui::BeginPopupContextItem()) {
                            ctxMenuFn_(idx);
                            ImGui::EndPopup();
                        }
                    }
                } else {
                    // Cell-level styling
                    if (cellColorFn_) {
                        ImU32 c = cellColorFn_(idx, col, (*data_)[idx]);
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(c&0xFF, (c>>8)&0xFF, (c>>16)&0xFF, (c>>24)&0xFF));
                    }
                    if (cellBoldFn_ && cellBoldFn_(idx, col, (*data_)[idx]))
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    ImGui::TextUnformatted(text.c_str());
                    int pops = (cellColorFn_ ? 1 : 0) + (cellBoldFn_ && cellBoldFn_(idx,col,(*data_)[idx]) ? 1 : 0);
                    ImGui::PopStyleColor(pops);
                }
            }

            if (ctxMenuFn_ && ImGui::BeginPopupContextItem()) {
                ctxMenuFn_(idx);
                ImGui::EndPopup();
            }
            ImGui::PopID();
        };

        // ── Render rows ──────────────────────────────────────────────────
        if (groups_.empty()) {
            for (int row = firstRow; row < lastRow; ++row) {
                int idx = (sortColumn_ >= 0 && !sortIndices_.empty())
                            ? sortIndices_[row] : row;
                renderRow(idx);
            }
        } else {
            // Group-aware rendering: disable global sort, each group self-sorts
            ImU32 groupBg = IM_COL32(45, 48, 62, 255);
            ImU32 ungroupedBg = IM_COL32(55, 50, 40, 255);
            int trailingRow = 0;

            for (size_t gi = 0; gi < groups_.size(); ++gi) {
                auto& g = groups_[gi];

                // ── Group header (full-width SpanAllColumns row) ──────
                ImGui::TableNextRow();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, groupBg);
                ImGui::TableSetColumnIndex(0);
                char hdr[256];
                int childCount = g.endRow < 0 ? (int)data_->size() - g.startRow
                               : std::min((int)data_->size(), g.endRow) - g.startRow;
                snprintf(hdr, sizeof(hdr), "%s  %s  (%d rows)",
                         g.expanded ? "▼" : "▶", g.label.c_str(), childCount);
                if (ImGui::Selectable(hdr, false, ImGuiSelectableFlags_SpanAllColumns))
                    g.expanded = !g.expanded;

                // ── Sort mini-header ─────────────────────────────────
                if (g.expanded) {
                    ImGui::TableNextRow();
                    for (int c = 0; c < (int)columns_.size(); ++c) {
                        ImGui::TableSetColumnIndex(c);
                        auto& col = columns_[c];
                        ImGui::PushID((int)(gi * 100 + c + 5000));

                        // Sort indicator label
                        const char* arrow = "";
                        if (g.sortCol == c) arrow = g.sortAsc ? " ▲" : " ▼";
                        char slabel[128];
                        snprintf(slabel, sizeof(slabel), "%s%s", col.name.c_str(), arrow);

                        if (ImGui::Selectable(slabel, false, ImGuiSelectableFlags_AllowDoubleClick)) {
                            int gStart = std::max(0, g.startRow);
                            int gEnd = g.endRow < 0 ? (int)data_->size()
                                      : std::min((int)data_->size(), g.endRow);
                            if (cellFmt_) {
                                // 3-state: none→asc→desc→none
                                if (g.sortCol != c) { g.sortCol = c; g.sortAsc = true; }
                                else if (g.sortAsc)  { g.sortAsc = false; }
                                else                  { g.sortCol = -1; }

                                // Do sort if any
                                if (g.sortCol >= 0) {
                                    std::vector<std::pair<std::string,int>> sv;
                                    for (int r = gStart; r < gEnd; ++r)
                                        sv.push_back({cellFmt_(r, g.sortCol, (*data_)[r]), r});
                                    if (g.sortAsc)
                                        std::sort(sv.begin(), sv.end(), [](auto&a,auto&b){return a.first<b.first;});
                                    else
                                        std::sort(sv.begin(), sv.end(), [](auto&a,auto&b){return a.first>b.first;});
                                    groupSortMap_.clear();
                                    int pos = gStart;
                                    for (auto& [s, row] : sv) groupSortMap_[row] = pos++;
                                } else {
                                    // Clear sort for this group
                                    for (int r = gStart; r < gEnd; ++r) groupSortMap_.erase(r);
                                }
                            }
                        }
                        ImGui::PopID();
                    }
                }

                // ── Child rows (sorted if group sort active) ─────────
                int groupStart = std::max(0, g.startRow);
                int groupEnd = g.endRow < 0 ? (int)data_->size() : std::min((int)data_->size(), g.endRow);
                trailingRow = std::max(trailingRow, groupEnd);

                if (!g.expanded) continue;

                if (g.sortCol >= 0 && !groupSortMap_.empty()) {
                    // Render in sorted order: build position→original index map
                    std::vector<int> sortedIdx(groupEnd - groupStart);
                    for (int r = groupStart; r < groupEnd; ++r)
                        sortedIdx[groupSortMap_[r] - groupStart] = r;
                    for (int pos = 0; pos < (int)sortedIdx.size(); ++pos) {
                        int idx = sortedIdx[pos];
                        idx = (sortColumn_ >= 0 && !sortIndices_.empty()) ? sortIndices_[idx] : idx;
                        renderRow(idx);
                    }
                } else {
                    for (int row = groupStart; row < groupEnd; ++row) {
                        int idx = (sortColumn_ >= 0 && !sortIndices_.empty()) ? sortIndices_[row] : row;
                        renderRow(idx);
                    }
                }
            }

            // ── Ungrouped rows ────────────────────────────────────────
            if (trailingRow < totalRows) {
                ImGui::TableNextRow();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ungroupedBg);
                ImGui::TableSetColumnIndex(0);
                char ughdr[128];
                snprintf(ughdr, sizeof(ughdr), "▸  Ungrouped (%d rows)", totalRows - trailingRow);
                ImGui::TextUnformatted(ughdr);

                for (int row = trailingRow; row < totalRows; ++row) {
                    int idx = (sortColumn_ >= 0 && !sortIndices_.empty())
                                ? sortIndices_[row] : row;
                    renderRow(idx);
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
    CellColorFn cellColorFn_;
    CellBoldFn cellBoldFn_;
    std::unordered_map<int, SortCompare> sortComps_;
    std::vector<int> sortIndices_;
    int sortColumn_ = -1;
    bool sortAscending_ = true;
    bool virtualScroll_ = true;
    int scrollToRow_ = -1;
    SelectFn onSelect_;
    DoubleClickFn onDblClick_;
    std::function<void()> onSelChanged_;
    CellCommitFn onCellCommit_;
    FilterFn filterFn_;
    std::string filterText_;
    std::unordered_set<int> editableCols_;
    std::unordered_set<int> autoWidthCols_;
    int editRow_ = -1, editCol_ = -1;
    char editBuf_[256] = {};
    bool multiSelect_ = false;
    std::vector<int> selectedRows_;
    std::function<void(int)> ctxMenuFn_;
    int flashRow_ = -1; ImU32 flashColor_ = 0; float flashDuration_ = 0, flashElapsed_ = 0;
    bool columnReorder_ = false;
    std::vector<GroupInfo> groups_;
    std::map<int, int> groupSortMap_;
};

} // namespace unigui
