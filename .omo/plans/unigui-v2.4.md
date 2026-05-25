# TeamkillerUniGUI v2.4 — Completion + New Widgets + Third-Party + Polish

## TL;DR

> **Quick Summary**: Finish v2.3 remaining work (vulkan_triangle, Metal groundwork), add 4 new widgets (Tag, Breadcrumb, MultiLine, IconButton), backfill missing properties (TreeView icons/multi-select, ListView multi-select, Image sizing modes), integrate ImPlot for plotting, do engineering polish (Doxygen generation, widget_gallery, form_demo, vcpkg port prep).
> 
> **Test Target**: ≥160 tests (148 + ≥12 new)
> **Tasks**: ~16 tasks

---

## Phase 1: Completion (3 tasks)

- [ ] 1. vulkan_triangle Example
  **What**: `examples/vulkan_triangle/main.cc` — minimal Vulkan demo: SDL3 window, clear, triangle via ImDrawList, present N frames, `--frames N` flag.
  **QA**: `cmake --preset windows-msvc-sdl3-vulkan-debug && cmake --build` → example compiles

- [ ] 2. Metal Backend Groundwork
  **What**: `src/backend/metal_renderer.cc` — add `#ifdef __APPLE__` guard with comment for `.mm` compilation. On macOS, stub MTKView skeleton.
  **QA**: `cmake --build` compiles metal_renderer.cc on Windows (no-op stub)

- [ ] 3. BackendType Documentation
  **What**: README section "Backend Selection Guide" with platform requirements for all 4 backends.
  **QA**: `Select-String -Path "README.md" -Pattern "Backend Selection"` → found

## Phase 2: New Widgets (4 tasks, parallel)

- [ ] 4. Tag / Badge Widget
  **What**: `Tag` — colored label chip. `SetText()`, `SetColor(r,g,b)`, `SetRemovable(bool)`. Wraps styled ImGui::SmallButton/Text.
  **QA**: `ctest -R TagTest` → 2 tests pass

- [ ] 5. Breadcrumb Widget
  **What**: `Breadcrumb` — path navigation: "Home > Settings > Profile". `SetItems(vector<string>)`, `GetSelected()`, `OnSelect` callback.
  **QA**: `ctest -R Breadcrumb` → 2 tests

- [ ] 6. MultiLine Widget
  **What**: `MultiLine` — multi-line read-only text display. `SetText()`, `SetMaxLines()`, scrolling. Wraps ImGui::InputTextMultiline with ReadOnly.
  **QA**: `ctest -R MultiLine` → 2 tests

- [ ] 7. IconButton Widget
  **What**: `IconButton` — button with icon text (Unicode/Font Awesome). `SetIcon("★")`, `SetLabel()`, `SetSize()`. Extends Button widget.
  **QA**: `ctest -R IconButton` → 2 tests

## Phase 3: Property Backfills (3 tasks)

- [ ] 8. TreeView Enhancements
  **What**: Add `SetMultiSelect(bool)`, `SetShowIcons(bool)`, `GetSelectedNodes()` returning vector of indices.
  **QA**: `ctest -R TreeViewTest` → +2 tests

- [ ] 9. ListView Enhancements
  **What**: Add `SetMultiSelect(bool)`, `GetSelectedItems()` returning vector of indices.
  **QA**: `ctest -R ListViewTest` → +2 tests

- [ ] 10. Image Enhancements
  **What**: Add `SetScaleMode(Fit|Stretch|Crop)`, `SetAspectRatio(float)`.
  **QA**: `ctest -R ImageTest` → +1 test

## Phase 4: Third-Party Integration (2 tasks)

- [ ] 11. ImPlot Integration
  **What**: Add `vcpkg.json` dependency on `implot`. Create `include/unigui/ext/plot.h` — thin wrapper for `ImPlot::BeginPlot/EndPlot` with RAII scoping. `LinePlot`, `BarPlot`, `ScatterPlot` helpers.
  **QA**: `cmake --build` compiles with implot linked

- [ ] 12. Node Editor Groundwork
  **What**: Add `vcpkg.json` dependency on `imgui-node-editor` (optional). Create `include/unigui/ext/node_editor.h` skeleton.
  **QA**: Build compiles with node-editor linked

## Phase 5: Engineering (3 tasks)

- [ ] 13. widget_gallery Example
  **What**: `examples/widget_gallery/main.cc` — showcases all 37+ widgets in organized tabs.
  **QA**: Compiles and renders with `--frames 10`

- [ ] 14. Doxygen Generation
  **What**: Run `doxygen Doxyfile`, add `docs/` to .gitignore, add `docs/index.html` link to README.
  **QA**: `docs/index.html` exists

- [ ] 15. vcpkg Port Preparation
  **What**: Create `ports/unigui/vcpkg.json` and `portfile.cmake` for vcpkg registry submission.
  **QA**: `vcpkg install unigui` from local overlay works

## Phase FINAL

- [ ] F1-F4: Review agents

---

## Scope Boundaries

### INCLUDE (v2.4)
- vulkan_triangle example, Metal groundwork, 4 new widgets, 3 property backfills
- ImPlot integration (vcpkg), Doxygen generation, widget_gallery, form_demo
- vcpkg port prep

### EXCLUDE (v3+)
- DX12, WebGPU, Emscripten, full Metal implementation (macOS-only), multi-viewport
- imgui-node-editor full integration (groundwork only)
