#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <array>
#include <unordered_map>

namespace unigui {
class Table : public Widget {
public:
    enum class Alignment {
        Left,
        Center,
        Right
    };

    Table(std::string name, std::vector<std::string> columns);
    void Render() override;
    void AddRow(std::vector<std::string> row);
    void ClearRows();
    int GetSelectedRow() const;
    void SetOnSelect(std::function<void(int)> callback);
    void SetSortable(bool on);
    void SetResizable(bool on);
    void SetColumnAlignment(int col, Alignment alignment);
    Alignment GetColumnAlignment(int col) const;
    void SetColumnUnit(int col, std::string unit);
    const std::string& GetColumnUnit(int col) const;
    void SaveColumnWidths();
    void RestoreColumnWidths();

    /// Number of data rows / columns.
    int RowCount() const { return (int)rows_.size(); }
    int ColumnCount() const { return (int)columns_.size(); }
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
    void SetColumnSortComparator(int col, SortComparator cmp) { sort_comparators_[col] = std::move(cmp); }

    /// Export table to CSV string.
    std::string ExportCSV() const;
    /// Import table from CSV string. Returns true on success.
    bool ImportCSV(const std::string& csv);
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
};
}
