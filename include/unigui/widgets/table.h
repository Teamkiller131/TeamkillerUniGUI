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
    std::unordered_map<int, SortComparator> sort_comparators_;
    std::vector<float> col_widths_;
    std::vector<float> col_stretches_;
    bool scrollX_ = false;
};
} // namespace unigui
