# TeamkillerUniGUI v2.8 — Component Polish & Ecosystem

## TL;DR

> **Quick Summary**: NodeEditor integration groundwork, RichText/Markdown widget, ImageButton enhancements, additional property backfills, undo/redo on text widgets, more examples.
> 
> **Test Target**: ≥175 tests (165 + ≥10 new)
> **Tasks**: ~10 tasks

---

## Phase 1: New Components (4 tasks)

- [ ] 1. NodeEditor Groundwork
  **What**: `include/unigui/ext/node_editor.h` — `NodeEditorBegin/End`, `Node`, `Link` helpers. Wraps `imgui-node-editor`. vcpkg dependency.
  **QA**: Compiles with node-editor linked

- [ ] 2. RichText Widget
  **What**: `RichText` — formatted text display. Bold/italic/color spans. Wraps ImGui text rendering with style pushes.
  **QA**: `ctest -R RichText` → 2 tests

- [ ] 3. ImageButton Widget
  **What**: `ImageButton` — image + label button. `SetImage(textureID, size)`, `SetLabel()`. Extends Button.
  **QA**: `ctest -R ImageButton` → 2 tests

- [ ] 4. Markdown Widget
  **What**: `Markdown` — renders Markdown text. Wraps `imgui_markdown`. vcpkg dependency. `SetMarkdown("# Title\n**bold**")`.
  **QA**: Compiles with imgui_markdown linked

## Phase 2: Property Backfills (4 tasks)

- [ ] 5. Undo/Redo for LineEdit/MultiLine
  **What**: Add undo stack to LineEdit and MultiLine. `Undo()`, `Redo()`, `CanUndo()`, `CanRedo()`.
  **QA**: `ctest -R "LineEdit|MultiLine"` → +4 tests

- [ ] 6. Form Validation Enhancements
  **What**: Form: regex validation, min/max for numbers, custom error messages. `SetFieldValidator(name, fn, "error msg")`.
  **QA**: `ctest -R FormTest` → +2 tests

- [ ] 7. ComboBox Icons
  **What**: ComboBox: per-item icon support. `SetItemIcon(index, textureID)`.
  **QA**: `ctest -R ComboBox` → +1 test

- [ ] 8. Table Column Width Persistence
  **What**: Table: `SaveColumnWidths()` / `RestoreColumnWidths()`. Persist user-adjusted widths.
  **QA**: `ctest -R TableTest` → +1 test

## Phase 3: Examples & Docs (2 tasks)

- [ ] 9. plot_demo Example
  **What**: `examples/plot_demo/main.cc` — demonstrates ImPlot integration: line/bar/scatter plots.
  **QA**: Compiles and renders with `--frames 10`

- [ ] 10. README + CHANGELOG v2.8
  **QA**: Updated

---

## Scope

### INCLUDE: NodeEditor, RichText, ImageButton, Markdown, undo/redo, Form enhancements
### EXCLUDE: Full NodeEditor implementation (groundwork only), hot-reload
