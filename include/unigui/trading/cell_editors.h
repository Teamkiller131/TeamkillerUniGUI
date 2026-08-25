#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// In-cell editors for DataTable<T>  (namespace unigui::trading)
//
// Interactive cell types built on the stateless unigui::im wrappers — unlike
// the read-only renderers in cell_renderers.h, these can modify the row's
// model data via a setter callback. They handle the widget-ID naming that
// hand-written `static std::map` caches get wrong after sort/reorder.
//
// All editors are CellRenderFn values: draw from the cell's cursor position
// and end with the editor's natural extent so the row reserves the correct
// height. Header-only; the lambdas only borrow their captures — the caller
// owns the value sources.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/im/im.h>
#include <unigui/trading/blotters.h> // theme::Polarity
#include <unigui/widgets/button.h>
#include <unigui/widgets/datatable.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace unigui::trading {

// ─────────────────────────────────────────────────────────────────────────────
// ComboCell — in-cell dropdown with model binding.
//
// `optionsOf(row)` returns the option list for that row (can be dynamic).
// `selectedOf(row)` returns the current selection index (0-based).
// `onSelect(row, newIdx)` is called when the user changes the selection —
// write the model and trigger any side effects there.
//
// When `editableWhen` is provided and returns false for a row, the cell
// degrades to a read-only text display of the currently selected option.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
typename DataTable<T>::CellRenderFn ComboCell(
        std::function<std::vector<std::string>(int row, const T&)> optionsOf,
        std::function<int(int row, const T&)> selectedOf,
        std::function<void(int row, int newIdx, T&)> onSelect,
        std::function<bool(int row, const T&)> editableWhen = nullptr) {
    return [optionsOf = std::move(optionsOf), selectedOf = std::move(selectedOf),
            onSelect = std::move(onSelect), editableWhen = std::move(editableWhen)](
                   int row, const T& item) {
        const std::vector<std::string> opts = optionsOf(row, item);
        const int sel = selectedOf(row, item);

        if (editableWhen && !editableWhen(row, item)) {
            if (sel >= 0 && sel < (int) opts.size()) {
                ImGui::TextUnformatted(opts[sel].c_str());
            } else {
                ImGui::TextDisabled("-");
            }
            return;
        }

        int cur = sel;
        if (unigui::im::Combo("##cc", &cur, opts)) {
            if (cur != sel && onSelect)
                onSelect(row, cur, const_cast<T&>(item));
        }
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// ButtonCell — in-cell action button.
//
// `labelOf(row)` returns the button text (can vary by state, e.g. "Start" vs
// "Stop"). `onClick(row)` fires when pressed. `variantOf` optionally returns
// a per-row colour variant; `enabledWhen` controls the disabled state.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
typename DataTable<T>::CellRenderFn ButtonCell(
        std::function<std::string(int row, const T&)> labelOf,
        std::function<void(int row, T&)> onClick,
        std::function<unigui::Button::ColorVariant(int row, const T&)> variantOf = nullptr,
        std::function<bool(int row, const T&)> enabledWhen = nullptr) {
    return [labelOf = std::move(labelOf), onClick = std::move(onClick),
            variantOf = std::move(variantOf), enabledWhen = std::move(enabledWhen)](
                   int row, const T& item) {
        const std::string label = labelOf(row, item);
        const bool enabled = !enabledWhen || enabledWhen(row, item);

        if (!enabled)
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

        unigui::Button btn("##bc", label);
        if (variantOf)
            btn.SetColorVariant(variantOf(row, item));
        if (btn.Render() && onClick && enabled)
            onClick(row, const_cast<T&>(item));

        if (!enabled)
            ImGui::PopStyleVar();
    };
}

} // namespace unigui::trading
