#pragma once
#include <unigui/widgets/widget_base.h>

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace unigui {
class Table : public FluentWidget<Table> {
public:
    enum class Alignment { Left, Center, Right };

    Table(std::string name, std::vector<std::string> columns);
    void Render() override;
    void AddRow(std::vector<std::string> row);
    void ClearRows();
    int GetSelectedRow() const;
    void SetOnSelect(std::function<void(int)> callback);
    void SetSortable(bool on);
    void SetResizable(bool on);
    void SortByColumn(int col, bool ascending = true);
    void SetColumnAlignment(int col, Alignment alignment);
    Alignment GetColumnAlignment(int col) const;
    void SetColumnUnit(int col, std::string unit);
    const std::string& GetColumnUnit(int col) const;
    /// Set initial fixed pixel width for a column. 0 = default stretch.
    void SetColumnWidth(int col, float width);
    float GetColumnWidth(int col) const;
    /// Set stretch weight for a column (WidthStretch). Overrides SetColumnWidth when > 0.
    void SetColumnStretch(int col, float weight);
    float GetColumnStretch(int col) const;
    /// Enable horizontal scrolling for the table.
    void SetScrollX(bool on) { scrollX_ = on; }
    bool GetScrollX() const { return scrollX_; }
    /// Body scrolling. Default true: fixed-height viewport with an internal
    /// scroll child. Set false to grow to content height and let the enclosing
    /// panel scroll instead — row geometry then lives in the panel window, so
    /// GetRowRect / GetRowAtScreenY stay aligned with what the user clicks.
    void SetBodyScroll(bool on) { bodyScroll_ = on; }
    bool GetBodyScroll() const { return bodyScroll_; }
    /// 是否绘制列头行。多表拼接（分组区块）时，只有顶部那张表需要列头。
    void SetHeaderVisible(bool on) { headerVisible_ = on; }
    bool GetHeaderVisible() const { return headerVisible_; }
    void SaveColumnWidths();
    void RestoreColumnWidths();

    /// Number of data rows / columns.
    int RowCount() const { return (int) rows_.size(); }
    int ColumnCount() const { return (int) columns_.size(); }
    /// Read the raw string value of a cell (empty if out of range).
    const std::string& CellText(int row, int col) const;

    /// Per-cell custom renderer — lets you embed ANY ImGui widget inside a
    /// cell (ProgressBar, Button, icon, Checkbox, ...). The cursor is already
    /// positioned in the target cell when invoked. Return true if the cell was
    /// drawn, or false to fall back to the default text/selectable rendering.
    using CellRenderer = std::function<bool(int row, int col)>;
    void SetCellRenderer(CellRenderer fn) { cell_renderer_ = std::move(fn); }

    /// Per-row height provider (pixels). Return <= 0 to keep the default
    /// single-line row height. Setting one disables list clipping
    /// (ImGuiListClipper assumes uniform row heights), so use it for tables with
    /// a modest row count — e.g. grouping/separator rows that are taller than
    /// the data rows around them.
    using RowHeightProvider = std::function<float(int row)>;
    void SetRowHeightProvider(RowHeightProvider fn) { row_height_provider_ = std::move(fn); }
    bool HasRowHeightProvider() const { return row_height_provider_ != nullptr; }

    /// Per-row background painter. Return true to override this row's band
    /// colour (ImGuiTableBgTarget_RowBg0) with `out`. Used to paint grouping /
    /// separator bands without giving up the table's default zebra striping on
    /// the rows that do not opt in.
    using RowBgProvider = std::function<bool(int row, ImVec4& out)>;
    void SetRowBgProvider(RowBgProvider fn) { row_bg_provider_ = std::move(fn); }

    /// Screen-space rect of a row laid out during the last rendered frame.
    /// Returns false when the row was not laid out (clipped, or out of range).
    /// Lets callers draw drag/drop indicators and hit-test rows by mouse Y.
    bool GetRowRect(int row, ImVec2& out_min, ImVec2& out_max) const;
    /// Row whose band contains screen-space Y; -1 when none does.
    int GetRowAtScreenY(float y) const;

    /// Custom comparator for sorting a column. Receives the raw cell strings of
    /// two rows for that column and returns true if 'a' should sort before 'b'
    /// (ascending). If none is set, a numeric-aware comparison is used.
    using SortComparator = std::function<bool(const std::string& a, const std::string& b)>;
    void SetColumnSortComparator(int col, SortComparator cmp) {
        sort_comparators_[col] = std::move(cmp);
    }

    /// Export table to CSV string.
    std::string ExportCSV() const;
    /// Import table from CSV string. Returns true on success.
    bool ImportCSV(const std::string& csv);

    // ── Fluent (chainable) helpers — return Table& via CRTP base ──────────
    Table& WithOnSelect(std::function<void(int)> callback) {
        SetOnSelect(std::move(callback));
        return *this;
    }
    Table& WithSortable(bool on) {
        SetSortable(on);
        return *this;
    }
    Table& WithResizable(bool on) {
        SetResizable(on);
        return *this;
    }
    Table& WithColumnAlignment(int col, Alignment alignment) {
        SetColumnAlignment(col, alignment);
        return *this;
    }
    Table& WithColumnUnit(int col, std::string unit) {
        SetColumnUnit(col, std::move(unit));
        return *this;
    }
    Table& WithColumnWidth(int col, float width) {
        SetColumnWidth(col, width);
        return *this;
    }
    Table& WithColumnStretch(int col, float weight) {
        SetColumnStretch(col, weight);
        return *this;
    }
    Table& WithScrollX(bool on) {
        SetScrollX(on);
        return *this;
    }
    Table& WithCellRenderer(CellRenderer fn) {
        SetCellRenderer(std::move(fn));
        return *this;
    }
    Table& WithColumnSortComparator(int col, SortComparator cmp) {
        SetColumnSortComparator(col, std::move(cmp));
        return *this;
    }

private:
    void ApplySort(int col, bool ascending);

    std::vector<std::string> columns_;
    std::vector<std::vector<std::string>> rows_;
    std::vector<Alignment> alignments_;
    std::vector<std::string> units_;
    int selected_ = -1;
    bool sortable_ = false;
    bool resizable_ = false;
    std::function<void(int)> on_select_;
    std::vector<float> saved_widths_;
    CellRenderer cell_renderer_;
    RowHeightProvider row_height_provider_;
    RowBgProvider row_bg_provider_;
    /// Per-row screen rect laid out in the last frame: (x0, y0, x1, y1).
    /// Rows clipped away keep a zero rect so GetRowRect() reports "not laid out".
    std::vector<ImVec4> row_rects_;
    std::unordered_map<int, SortComparator> sort_comparators_;
    std::vector<float> col_widths_;
    std::vector<float> col_stretches_;
    bool scrollX_ = false;
    bool bodyScroll_ = true;
    bool headerVisible_ = true;
};
} // namespace unigui
