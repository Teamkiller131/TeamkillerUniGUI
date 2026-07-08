#pragma once

#include <unigui/widgets/datatable.h>
#include <unigui/widgets/editabledatagrid.h>
#include <unigui/widgets/widget_base.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// BasketTicket<T> — an editable basket / program-trading row grid: a toolbar
/// (Add / Remove / Import / optional Submit) over an `EditableDataGrid<T>` whose
/// rows the ticket owns. Invalid rows (per a caller validator) are highlighted,
/// and removal is **deferred** to after the grid renders so the row vector isn't
/// mutated mid-iteration.
///
/// Presentation-only and host-driven by design: the embedder owns CSV/TXT/XLSX
/// parsing and the file dialog (the Import button just fires `onImportRequested`,
/// and the host calls `SetRows()` with parsed rows), and order routing /
/// replenish-liquidate logic stays in the controller (`onSubmit` hands back the
/// rows). Column editors are configured through `Grid()`.
template <class T> class BasketTicket : public FluentWidget<BasketTicket<T>> {
public:
    BasketTicket(std::string name, std::vector<typename DataTable<T>::ColumnDef> columns)
            : FluentWidget<BasketTicket<T>>(std::move(name))
            , grid_(this->GetName() + "_grid", std::move(columns)) {
        grid_.SetMultiSelect(true);
    }

    /// Configure the grid's columns / cell editors directly.
    EditableDataGrid<T>& Grid() { return grid_; }

    // ── Rows ────────────────────────────────────────────────────────────
    void SetRows(std::vector<T> rows) { rows_ = std::move(rows); }
    const std::vector<T>& Rows() const { return rows_; }
    std::vector<T>& MutableRows() { return rows_; }
    void AddRow(T row) { rows_.push_back(std::move(row)); }
    /// Immediate programmatic removal (the toolbar uses deferred removal).
    void RemoveRow(int index) {
        if (index >= 0 && index < static_cast<int>(rows_.size()))
            rows_.erase(rows_.begin() + index);
    }
    void Clear() { rows_.clear(); }
    std::size_t RowCount() const { return rows_.size(); }
    /// Rows passing the validator (all rows if no validator is set).
    std::size_t ValidCount() const {
        if (!validator_)
            return rows_.size();
        return static_cast<std::size_t>(std::count_if(
            rows_.begin(), rows_.end(), [this](const T& r) { return validator_(r); }));
    }
    bool AllValid() const { return !rows_.empty() && ValidCount() == rows_.size(); }

    // ── Configuration ───────────────────────────────────────────────────
    /// Factory for the "Add" button (the new blank/default row).
    BasketTicket& SetRowFactory(std::function<T()> f) {
        rowFactory_ = std::move(f);
        return *this;
    }
    /// Rows for which this returns false are highlighted as invalid.
    BasketTicket& SetValidator(std::function<bool(const T&)> v) {
        validator_ = std::move(v);
        return *this;
    }
    /// Fired when the user clicks Import — the host owns the file dialog/parser
    /// and calls SetRows() with the result.
    BasketTicket& SetOnImportRequested(std::function<void()> cb) {
        onImport_ = std::move(cb);
        return *this;
    }
    /// Fired when the user submits a fully-valid basket.
    BasketTicket& SetOnSubmit(std::function<void(const std::vector<T>&)> cb) {
        onSubmit_ = std::move(cb);
        return *this;
    }

    // ── Fluent (chainable) helpers — return BasketTicket& via CRTP base ──
    BasketTicket& WithRows(std::vector<T> rows) {
        SetRows(std::move(rows));
        return *this;
    }

    void Render() override {
        if (!this->IsVisible())
            return;
        ImGui::PushID(this->GetName().c_str());

        // ── Toolbar ─────────────────────────────────────────────────────
        if (ImGui::Button("+ Add") && rowFactory_)
            AddRow(rowFactory_());
        ImGui::SameLine();
        if (ImGui::Button("- Remove")) {
            const auto sel = grid_.GetSelectedRows();
            pendingRemove_.insert(pendingRemove_.end(), sel.begin(), sel.end());
        }
        if (onImport_) {
            ImGui::SameLine();
            if (ImGui::Button("Import"))
                onImport_();
        }
        if (onSubmit_) {
            ImGui::SameLine();
            const bool ok = AllValid();
            if (!ok)
                ImGui::BeginDisabled();
            if (ImGui::Button("Submit"))
                onSubmit_(rows_);
            if (!ok)
                ImGui::EndDisabled();
        }

        // ── Grid ────────────────────────────────────────────────────────
        grid_.SetDataSource(&rows_);
        if (validator_) {
            grid_.SetRowColor([this](int, const T& item) -> ImU32 {
                return validator_(item) ? 0u : IM_COL32(120, 40, 40, 110);
            });
        }
        grid_.Render();

        // ── Deferred removal (after the grid iterated rows_) ────────────
        if (!pendingRemove_.empty()) {
            std::sort(pendingRemove_.begin(), pendingRemove_.end(), std::greater<int>());
            pendingRemove_.erase(std::unique(pendingRemove_.begin(), pendingRemove_.end()),
                                 pendingRemove_.end());
            for (int idx : pendingRemove_)
                if (idx >= 0 && idx < static_cast<int>(rows_.size()))
                    rows_.erase(rows_.begin() + idx);
            pendingRemove_.clear();
        }

        ImGui::PopID();
    }

private:
    EditableDataGrid<T> grid_;
    std::vector<T> rows_;
    std::vector<int> pendingRemove_;
    std::function<T()> rowFactory_;
    std::function<bool(const T&)> validator_;
    std::function<void()> onImport_;
    std::function<void(const std::vector<T>&)> onSubmit_;
};

} // namespace unigui
