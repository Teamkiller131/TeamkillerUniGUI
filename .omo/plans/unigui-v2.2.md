# TeamkillerUniGUI v2.2 — File I/O + Missing Components + Property Backfills + DX11

## TL;DR

> **Quick Summary**: Add FilePath/DirPath selectors, SpinBox/ToggleSwitch/InputInt/InputFloat/Tooltip/Splitter/Separator/ScrollArea, backfill missing properties on 7 existing widgets (Button variants, LineEdit modes, ComboBox search, Table sorting, ColorPicker RGBA, Form field types, Window flags), complete DX11 backend HWND integration.
> 
> **Deliverables**:
> - 9 new widgets: FilePath, DirPath, SpinBox, ToggleSwitch, InputInt, InputFloat, Tooltip, Splitter, ScrollArea
> - Property backfills on 7 existing widgets
> - DX11 native file dialog integration
> - DX11 renderer HWND initialization
> - Metal .mm file structure (macOS groundwork)
> - Updated README + CHANGELOG
> 
> **Estimated Effort**: Medium-Large (~18 tasks)
> **Parallel Execution**: YES — 3 waves
> **Critical Path**: Task 1 → Task 2 → Task 16 → FINAL

---

## Work Objectives

### Core Objective
Add file/directory selection dialogs, 9 new standard components, backfill missing properties on existing widgets, and complete DX11 backend integration.

### Definition of Done
- [ ] `ctest --preset windows-msvc-debug` → ≥140 tests pass (120 existing + ≥20 new)
- [ ] DX11 configures + compiles on Windows
- [ ] FilePath shows native Windows file dialog (or ImGui fallback on Linux)
- [ ] All property backfills have test coverage

---

## TODOs

### Wave 1: File I/O + DX11 (4 tasks)

- [ ] 1. FilePath Widget
  **What**: `FilePath` class — file open/save selector with filter, extension, title. Uses native file dialog (Windows COM `IFileDialog`, macOS `NSOpenPanel`) or ImGui fallback. ASCII text path display with "Browse..." button.
  **Properties**: `GetPath()`, `SetPath()`, `SetFilter("*.cpp;*.h")`, `SetTitle("Open File")`, `SetMode(Open | Save)`, `OnPathChanged` callback.
  **Agent**: `unspecified-high` | **Test**: 3 tests
  **QA**: `ctest --preset windows-msvc-debug -R "FilePathTest" --output-on-failure` → exit 0, 3 tests pass: `GetPath_DefaultsToEmpty`, `SetPath_Works`, `Render_DoesNotCrash`

- [ ] 2. DirPath Widget
  **What**: `DirPath` class — directory selector. Same pattern as FilePath but for directories only.
  **Properties**: `GetPath()`, `SetPath()`, `SetTitle("Select Folder")`, `OnPathChanged` callback.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "DirPathTest" --output-on-failure` → exit 0, 2 tests pass: `GetPath_DefaultsToEmpty`, `Render_DoesNotCrash`

- [ ] 3. DX11 HWND Integration
  **What**: Update `dx11_renderer.cc` — add `CreateDeviceAndSwapChain(HWND)` helper using DXGI factory. Wire up in `CreateDX11Renderer()` for Windows. Use GLFW window handle for DX11 init.
  **Agent**: `deep`
  **QA**: `cmake --preset windows-msvc-debug && cmake --build --preset windows-msvc-debug` → exit 0, `dx11_renderer.cc` compiles and links without errors. Verify `UNIGUI_HAS_DX11` defined: `Select-String -Path "build/windows-msvc-debug/CMakeCache.txt" -Pattern "UNIGUI_HAS_DX11"` → found.

- [ ] 4. Metal Backend Groundwork
  **What**: Create `src/backend/metal_renderer.cc` with `#ifdef __APPLE__` guard for `.mm` compilation hint. Actual Metal implementation deferred but file structure and CMake detection ready.
  **Agent**: `quick`
  **QA**: `Test-Path "src/backend/metal_renderer.cc"` → True. `cmake --build --preset windows-msvc-debug` → exit 0 (file compiles as no-op on Windows).

- [ ] 5. SpinBox Widget
  **What**: `SpinBox<T>` template — wraps `ImGui::InputInt/InputFloat`. Min/max/step/value. Format string. OnChange callback.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "SpinBoxTest" --output-on-failure` → exit 0, 2 tests: `GetValue_DefaultsCorrect`, `Render_DoesNotCrash`

- [ ] 6. ToggleSwitch Widget
  **What**: `ToggleSwitch` — modern on/off toggle. Uses styled `ImGui::Checkbox` or custom draw.
  **Properties**: `IsOn()`, `SetOn()`, `SetOff()`, `OnChange` callback.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "ToggleSwitchTest" --output-on-failure` → exit 0, 2 tests: `DefaultsToOff`, `SetOn_Works`

- [ ] 7. InputInt Widget
  **What**: `InputInt` — wraps `ImGui::InputInt()`. Min/max validation. OnChange callback.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "InputIntTest" --output-on-failure` → exit 0, 2 tests: `GetValue_DefaultsToZero`, `Render_DoesNotCrash`

- [ ] 8. InputFloat Widget
  **What**: `InputFloat` — wraps `ImGui::InputFloat()`. Min/max. Format string. OnChange.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "InputFloatTest" --output-on-failure` → exit 0, 2 tests: `GetValue_DefaultsToZero`, `Render_DoesNotCrash`

- [ ] 9. Tooltip Widget
  **What**: `Tooltip` — hover-based help text. `Tooltip::Show("text")` wrapper for `ImGui::SetTooltip()`.
  **Agent**: `quick` | **Test**: 1 test
  **QA**: `ctest --preset windows-msvc-debug -R "TooltipTest" --output-on-failure` → exit 0, 1 test: `Render_DoesNotCrash`

- [ ] 10. Splitter Widget
  **What**: `Splitter` — resizable panel divider. Horizontal or vertical. Min/max pane sizes. Two child content callbacks.
  **Agent**: `unspecified-high` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "SplitterTest" --output-on-failure` → exit 0, 2 tests: `Render_DoesNotCrash`, `Orientation_DefaultsToHorizontal`

- [ ] 11. Separator Widget
  **What**: `Separator` — standalone widget wrapping `ImGui::SeparatorText()` or `ImGui::Separator()`. Optional label text.
  **Agent**: `quick` | **Test**: 1 test
  **QA**: `ctest --preset windows-msvc-debug -R "SeparatorTest" --output-on-failure` → exit 0, 1 test: `Render_DoesNotCrash`

- [ ] 12. ScrollArea Widget
  **What**: `ScrollArea` — scrollable region with content callback. Wraps `ImGui::BeginChild()` with scrolling flags. Configurable size.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "ScrollAreaTest" --output-on-failure` → exit 0, 2 tests: `Render_DoesNotCrash`, `ContentCallback_IsCalled`

- [ ] 13. Button Enhancements
  **What**: Add `SetIcon()`, `SetColorVariant(Primary|Danger|Success|Default)`, `SetSize(Small|Medium|Large)`.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "ButtonTest" --output-on-failure` → exit 0, existing 6 + new 2 = 8 tests pass, including `ColorVariant_DefaultsToDefault`, `Size_DefaultsToMedium`

- [ ] 14. LineEdit Enhancements
  **What**: Add `SetPasswordMode(bool)`, `SetMultiline(bool)`, `SetReadOnly(bool)`, `SetMaxLength(int)`.
  **Agent**: `quick` | **Test**: 3 tests
  **QA**: `ctest --preset windows-msvc-debug -R "LineEditTest" --output-on-failure` → exit 0, existing 5 + new 3 = 8 tests pass, including `PasswordMode_MasksInput`, `ReadOnly_PreventsEdit`, `MaxLength_Truncates`

- [ ] 15. ComboBox Enhancements
  **What**: Add `SetEditable(bool)`, `SetSearchable(bool)` — filter items as user types.
  **Agent**: `quick` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "ComboBoxTest" --output-on-failure` → exit 0, existing 4 + new 2 = 6 tests pass, including `Editable_AcceptsCustomText`, `Searchable_FiltersItems`

- [ ] 16. Table Enhancements
  **What**: Add `SetSortable(bool)` — click header to sort. `SetFilterable(bool)`. `SetResizable(bool)`.
  **Agent**: `unspecified-high` | **Test**: 2 tests
  **QA**: `ctest --preset windows-msvc-debug -R "TableTest" --output-on-failure` → exit 0, existing 2 + new 2 = 4 tests pass, including `Sortable_ChangesOrder`, `Resizable_AllowsColumnResize`

- [ ] 17. ColorPicker + Form + Window Backfills
  **What**: ColorPicker: `SetAlpha(bool)`, `SetPresets()`. Form: `AddComboField()`, `AddSliderField()`, `AddNumberField()`. Window: `SetPosition(x,y)`, `SetFlags(resizable|movable|collapsible)`, `SetIcon()`.
  **Agent**: `unspecified-high` | **Test**: 4 tests
  **QA**: `ctest --preset windows-msvc-debug -R "ColorPickerTest|FormTest|WindowTest" --output-on-failure` → exit 0, new tests: `ColorPicker_Alpha_Works`, `Form_AddComboField_Works`, `Form_AddNumberField_Works`, `Window_SetPosition_Works`

- [ ] 18. README v2.2 Update
  **Agent**: `writing`
  **QA**: `Select-String -Path "README.md" -Pattern "v2.2"` → found. `Select-String -Path "README.md" -Pattern "FilePath|SpinBox|ToggleSwitch|Splitter"` → all 4 found.

- [ ] 19. CHANGELOG v2.2
  **Agent**: `writing`
  **QA**: `Test-Path "CHANGELOG.md"` → True. `Select-String -Path "CHANGELOG.md" -Pattern "v2.2"` → found.

- [ ] 19. CHANGELOG v2.2
  **Agent**: `writing`

---

## Scope Boundaries

### INCLUDE (v2.2)
- 9 new widgets (FilePath, DirPath, SpinBox, ToggleSwitch, InputInt, InputFloat, Tooltip, Splitter, ScrollArea)
- Property backfills (Button, LineEdit, ComboBox, Table, ColorPicker, Form, Window)
- DX11 HWND integration
- Metal groundwork (file structure)
- ~20 new tests (total ≥140)

### EXCLUDE (v3+)
- DatePicker, Image, Breadcrumb, LoadingIndicator, Notification/Toast
- DX12 backend
- Full Metal implementation
- WebGPU/Emscripten
- Multi-viewport
- ImPlot/NodeEditor integration

## Success Criteria

```bash
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug  # ≥140 tests
```
