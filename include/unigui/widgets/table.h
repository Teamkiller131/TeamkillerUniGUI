#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <array>

namespace unigui {
class Table : public Widget {
public:
    Table(std::string name, std::vector<std::string> columns);
    void Render() override;
    void AddRow(std::vector<std::string> row);
    void ClearRows();
    int GetSelectedRow() const;
    void SetOnSelect(std::function<void(int)> callback);
    void SetSortable(bool on);
    void SetResizable(bool on);
    void SaveColumnWidths();
    void RestoreColumnWidths();
private:
    std::vector<std::string> columns_;
    std::vector<std::vector<std::string>> rows_;
    int selected_ = -1;
    bool sortable_ = false;
    bool resizable_ = false;
    std::function<void(int)> on_select_;
    std::vector<float> saved_widths_;
};
}
