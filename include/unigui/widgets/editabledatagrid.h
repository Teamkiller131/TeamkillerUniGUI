#pragma once

#include <unigui/im/im.h>
#include <unigui/widgets/datatable.h>

#include <imgui.h>

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// EditableDataGrid<T> — a `DataTable<T>` with typed, per-column cell editors.
///
/// Data-driven grids that host stateful editors (a combo / int / float / button
/// bound to a row) are otherwise hand-rolled on raw `ImGui::BeginTable` with a
/// per-row `static std::map<int, Widget>` cache keyed by visible row index —
/// fragile (index reuse after sort/delete) and leaky. EditableDataGrid renders
/// each editor through the **stateless** `unigui::im` layer inside the table's
/// per-row `PushID`, so there is no widget cache at all: the value is read via a
/// getter and written via an on-change callback, keeping all strategy/routing
/// logic in the caller (presentation-only).
///
/// A per-row `SetRowReadOnly` predicate makes every editor in that row collapse
/// to static text — the pervasive "frozen once the pod is running" pattern.
template <class T> class EditableDataGrid : public DataTable<T> {
public:
    using DataTable<T>::DataTable;

    using ItemsFn = std::function<std::vector<std::string>(int row, const T&)>;
    using GetIntFn = std::function<int(int row, const T&)>;
    using SetIntFn = std::function<void(int row, int value)>;
    using GetFloatFn = std::function<float(int row, const T&)>;
    using SetFloatFn = std::function<void(int row, float value)>;
    using LabelFn = std::function<std::string(int row, const T&)>;
    using ClickFn = std::function<void(int row)>;
    using ReadOnlyFn = std::function<bool(int row, const T&)>;

    /// Rows for which this returns true render every editor as static text.
    EditableDataGrid& SetRowReadOnly(ReadOnlyFn fn) {
        readOnly_ = std::move(fn);
        return *this;
    }

    /// A dropdown column: `items`/`getSel` read the row, `onChange(row, sel)` writes.
    EditableDataGrid& SetComboColumn(int col, ItemsFn items, GetIntFn getSel, SetIntFn onChange) {
        const std::string id = "##egc" + std::to_string(col);
        this->SetCellRenderer(col, [=, this](int row, const T& item) {
            const std::vector<std::string> opts = items(row, item);
            int sel = getSel(row, item);
            if (IsFrozen(row, item)) {
                const char* txt = (sel >= 0 && sel < (int) opts.size()) ? opts[sel].c_str() : "";
                ImGui::TextUnformatted(txt);
                return;
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (im::Combo(id, &sel, opts))
                onChange(row, sel);
        });
        return *this;
    }

    /// An integer input column.
    EditableDataGrid& SetIntColumn(int col, GetIntFn getVal, SetIntFn onChange, int step = 1) {
        const std::string id = "##egi" + std::to_string(col);
        this->SetCellRenderer(col, [=, this](int row, const T& item) {
            int v = getVal(row, item);
            if (IsFrozen(row, item)) {
                ImGui::Text("%d", v);
                return;
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (im::InputInt(id, &v, step))
                onChange(row, v);
        });
        return *this;
    }

    /// A float input column. `fmt` is a printf-style display format (e.g. "%.2f").
    EditableDataGrid& SetFloatColumn(int col, GetFloatFn getVal, SetFloatFn onChange,
                                     const char* fmt = "%.2f") {
        const std::string id = "##egf" + std::to_string(col);
        const std::string fmtStr = fmt;
        this->SetCellRenderer(col, [=, this](int row, const T& item) {
            float v = getVal(row, item);
            if (IsFrozen(row, item)) {
                char buf[64];
                std::snprintf(buf, sizeof(buf), fmtStr.c_str(), v);
                ImGui::TextUnformatted(buf);
                return;
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (im::InputFloat(id, &v, 0.0f, 0.0f, fmtStr))
                onChange(row, v);
        });
        return *this;
    }

    /// An action button column. `label` is per-row; `onClick(row)` fires on press.
    /// Frozen rows render the label as disabled text.
    EditableDataGrid& SetButtonColumn(int col, LabelFn label, ClickFn onClick) {
        const std::string id = "##egb" + std::to_string(col);
        this->SetCellRenderer(col, [=, this](int row, const T& item) {
            const std::string text = label(row, item);
            if (IsFrozen(row, item)) {
                ImGui::TextDisabled("%s", text.c_str());
                return;
            }
            if (im::Button(text + id))
                onClick(row);
        });
        return *this;
    }

private:
    bool IsFrozen(int row, const T& item) const { return readOnly_ && readOnly_(row, item); }
    ReadOnlyFn readOnly_;
};

} // namespace unigui
