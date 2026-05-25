# TeamkillerUniGUI v2.6 — Advanced UI Features

## TL;DR

> **Quick Summary**: Multi-viewport support (OS-level window dragging), ContextMenu/PopupMenu, drag-drop, keyboard shortcuts, DockSpace layout, TreeView drag-drop, enhanced widget interactivity.
> 
> **Test Target**: ≥165 tests (156 + ≥9 new)
> **Tasks**: ~12 tasks

---

## Phase 1: Multi-Viewport (2 tasks)

- [ ] 1. Enable Multi-Viewport
  **What**: Set `ImGuiConfigFlags_ViewportsEnable` in App Bootstrap. Enable platform backend viewport support in GLFW/SDL3 backends. Test window dragging to OS-level windows.
  **QA**: `hello_unigui` compiled with viewports → windows can be dragged outside main window

- [ ] 2. DockSpace Widget
  **What**: `DockSpace` — full docking layout manager. Wraps `ImGui::DockSpace()`. Allows users to arrange windows by drag-docking.
  **QA**: `ctest -R DockSpace` → 2 tests pass

## Phase 2: Context Menu & Popups (3 tasks)

- [ ] 3. ContextMenu Widget
  **What**: `ContextMenu` — right-click popup menu. `AddItem(label, callback)`, `AddSeparator()`, `Show()`. Wraps `ImGui::OpenPopupOnItemClick` + `BeginPopupContextWindow`.
  **QA**: `ctest -R ContextMenu` → 2 tests

- [ ] 4. PopupMenu Enhancement
  **What**: Add `SetContextMenu` to TreeView, ListView, Table — widgets gain right-click support via a shared context menu pattern.
  **QA**: `ctest -R "TreeView|ListView"` → existing + 2 new tests

- [ ] 5. Widget Tooltip Enhancement
  **What**: Add per-widget tooltip support: `Widget::SetTooltip("hover text")` method on Widget base class.
  **QA**: `ctest -R Widget` → +1 test for tooltip

## Phase 3: Drag & Drop (3 tasks)

- [ ] 6. Drag-Drop Source/Sink API
  **What**: `DragSource` / `DropTarget` template helpers. `BeginDragSource(type, data)`, `BeginDropTarget()`, `AcceptDrag(type, callback)`.
  **QA**: `ctest -R DragDrop` → 2 tests

- [ ] 7. TreeView Drag-Drop
  **What**: Enable drag-drop reordering in TreeView. Nodes can be dragged to reorder within the tree.
  **QA**: `ctest -R TreeView` → +1 test for drag-drop

- [ ] 8. ListView Drag-Drop
  **What**: Enable drag-drop reordering in ListView. Items can be dragged to reorder.
  **QA**: `ctest -R ListView` → +1 test for drag-drop

## Phase 4: Keyboard & Input (4 tasks)

- [ ] 9. Keyboard Shortcut System
  **What**: `ShortcutManager` — register global keyboard shortcuts. `Register(ImGuiKey, callback)`, `Unregister()`, process in NewFrame.
  **QA**: `ctest -R Shortcut` → 2 tests

- [ ] 10. Widget Focus System
  **What**: `Widget::SetFocused()`, `Widget::IsFocused()`, Tab-key navigation between focusable widgets. Add to Widget base.
  **QA**: `ctest -R Focus` → 2 tests

- [ ] 11. Input Hints
  **What**: Per-widget `SetInputHint(text)` for placeholder/help text displayed when widget is hovered or focused.
  **QA**: `ctest -R InputHint` → 1 test

- [ ] 12. README + CHANGELOG v2.6
  **What**: Document new advanced UI features, multi-viewport setup, keyboard shortcuts.
  **QA**: README updated, CHANGELOG entry

---

## Scope Boundaries

### INCLUDE (v2.6)
- Multi-viewport, DockSpace, ContextMenu, PopupMenu
- Drag-drop API, TreeView/ListView drag-drop
- Keyboard shortcuts, widget focus, input hints

### EXCLUDE (v3+)
- DX12/WebGPU/Emscripten
- ImPlot enhancements, NodeEditor
- Hot-reload, plugin system
