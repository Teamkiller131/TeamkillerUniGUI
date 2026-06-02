# TeamkillerUniGUI v3.5.0 Release Notes

**Release Date:** 2026-06-02 | **Version:** 3.5.0 | **Widgets:** 82 | **Tests:** 637

---

## Highlights

### Better UX for CascadingCombo

The cascading dropdown is no longer locked to a vertical stack and is much easier to size:

- `SetLayout(Layout::Horizontal | Vertical)` — arrange combos side-by-side or stacked.
- `SetItemWidth(float)` — set one width for all combos; `SetItemWidth(int level, float)` or `Level::width` to override a single level.
- `SetSpacing(float)` — control the gap between combos in horizontal layout.
- Fluent `WithLayout(...).WithItemWidth(...).WithSpacing(...)`; `SetHorizontal(bool)` retained for compatibility.

See the new guide: [docs/CASCADINGCOMBO.md](docs/CASCADINGCOMBO.md).

### Table & DataTable improvements

- **Table**: interactive column sorting (`SetSortable`, numeric-aware default + custom `SortComparator`) and cell embedding (`SetCellRenderer`) to host any ImGui content inside a cell. Sorting now parses each cell key once for better performance.
- **DataTable**: inline checkbox columns via `SetCellCheckbox(col, fn)`, with `SpanAllColumns` auto-disabled when checkbox columns are present so clicks register correctly.

### Display polish

- **StatusLamp** gains a glow effect (`SetGlowEnabled`), with the glow's vertical padding included in the widget bounds so it lays out correctly in tables/rows.
- **Charts** follow the active theme background (`SetThemeBackground`).
- Broader embedded font glyph coverage.

### Documentation overhaul

- [docs/WIDGET_API.md](docs/WIDGET_API.md) was rewritten as a verified, categorized reference for **all 82 widgets** — constructors, methods, and copy-paste examples.
- New in-depth guides: [docs/TREEVIEW.md](docs/TREEVIEW.md) and [docs/CASCADINGCOMBO.md](docs/CASCADINGCOMBO.md).
- `README.md` / `README_zh.md` synced: correct widget (82) and test (637) counts, version, new components, and guide links.

---

## Upgrade Guide

### From v3.4.x

1. **Pull latest**: `git pull origin master`
2. **Rebuild** with your usual preset (e.g. `cmake-msvc.cmd --build --preset windows-msvc-release`).
3. **No breaking changes** — existing code compiles unchanged. `CascadingCombo::SetHorizontal(bool)` still works and now maps onto the new layout system.

### New APIs at a glance

```cpp
// CascadingCombo: horizontal layout + widths
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal)
   .WithItemWidth(120.f)
   .WithSpacing(8.f);
cc->SetItemWidth(0, 150.f);              // widen the first level only

// Table: sortable + embedded cell content
table->SetSortable(true);
table->SetCellRenderer([](int row, int col){ /* draw + return true */ return false; });

// DataTable: inline checkbox column
dt->SetCellCheckbox(0, [](int row){ return &rowChecked[row]; });

// StatusLamp glow
lamp->SetGlowEnabled(true);
```

---

**Full Changelog:** [CHANGELOG.md](CHANGELOG.md) · **API Docs:** [docs/WIDGET_API.md](docs/WIDGET_API.md)

---

# TeamkillerUniGUI v3.3.1 Release Notes

**Release Date:** 2026-06-01 | **Version:** 3.3.1 | **Widgets:** 83 | **Tests:** 579

---

## Highlights

### 9 New Widgets for Production Use

| Widget | Category | Purpose |
|--------|----------|---------|
| PanelBox | Container | Dark panel with title bar + tinted content |
| RiskBar | Display | Colored progress bar with warn/danger thresholds |
| StatusLamp | Display | Circular status indicator with draft blink |
| AlertBar | Feedback | Persistent top alert banner with animation |
| ConfirmDialog | Feedback | Modal confirmation popup with danger style |
| CascadingCombo | Input | N-level cascading dropdown (BeginCombo/EndCombo) |
| SliderBar | Input | Multi-tick draggable slider bar |
| FuturesRiskBar | Display | Multi-marker progress bar (actual/estimated/overnight) |

### Enhancements

- **TreeView**: RowRenderer callback + 6 new TreeNode fields (icon, suffix, progress, bgColor, labelColor, progressColor)
- **DataTable**: RowClickCallback, sort indicators, SetColumnMinWidth
- **ThemeRegistry**: GetCurrentThemeName()
- **FontManager**: LoadSystemEmoji() for auto emoji fallback
- **Theme**: SetFontScale() / GetFontScale()

### Bug Fixes

- **DataTable StickyHeader**: Added TableSetupScrollFreeze(0,1) — header now stays fixed
- **Emoji rendering**: Auto-loads Segoe UI Emoji on Windows

---

# TeamkillerUniGUI v3.3.0 Release Notes

**Release Date:** 2026-06-01  
**Version:** 3.3.0  
**C++ Standard:** C++23  
**Platforms:** Windows (MSVC + Clang), Linux (GCC), macOS (Clang)

---

## Highlights

### 🛡️ 100% ID Safety — Zero Widget ID Collisions

Every one of the **74 widgets** now automatically scopes ImGui IDs via `PushID(name)/PopID()`. Same widget label? No problem.

```cpp
auto ok    = std::make_shared<unigui::Button>("btn_ok",    "OK");
auto ok2   = std::make_shared<unigui::Button>("btn_ok2",   "OK"); // same label, NO conflict!
```

### 🆕 6 New Widgets

| Widget | Description |
|--------|-------------|
| `CollapsingHeader` | Expandable section with content callback |
| `Selectable` | Highlightable list/menu item |
| `ColorEdit` | Hex color editor with preview (`#RRGGBBAA`) |
| `DragFloat` | Drag-adjustable float input |
| `DragInt` | Drag-adjustable integer input |
| `ListBox` | Scrollable item list with selection callback |

### 🔬 Developer Tooling

| Tool | How |
|------|-----|
| **Coverage** | `cmake --build build/windows-clang-coverage --target coverage` → HTML report |
| **clang-tidy** | `.clang-tidy` config + `windows-clang-tidy` preset |
| **clang-format** | `.clang-format` (4-space K&R style) |
| **Sanitizers** | ASAN presets for Windows + Linux |
| **compile_commands.json** | Auto-generated for all builds (IDE + clang-tidy) |

### 📚 Complete API Documentation

[docs/WIDGET_API.md](docs/WIDGET_API.md) — 1746 lines, all 74+ widgets with C++23 code examples.

---

## Changelog

### Added
- **6 new widgets**: `CollapsingHeader`, `Selectable`, `ColorEdit`, `DragFloat`, `DragInt`, `ListBox`
- **ID Safety**: 100% PushID/PopID coverage across all 62 widget Render() methods
- **`.clang-format`** + **`.clang-tidy`** configs
- **Coverage pipeline**: `windows-clang-coverage` preset + `--target coverage`
- **clang-tidy preset**: `windows-clang-tidy` — lint on every compile
- **`cmake-msvc.cmd`**: Portable MSVC build wrapper (vswhere-based)
- **`compile_commands.json`**: Auto-generated in all builds
- **ASAN presets**: `windows-msvc-debug-asan`, `linux-gcc-debug-asan`
- **`docs/WIDGET_API.md`**: 1746-line API reference

### Changed
- **Tests**: 245 → 285 (100% pass on MSVC and Clang)
- **Widgets**: 68 → 74
- **CMakePresets.json**: 6 → 10 presets

### Fixed
- **ID collisions**: 47 widgets missing PushID/PopID — all fixed
- **badge.cc**: Non-Widget class incorrectly receiving Widget API calls
- **listbox.cc**: ImGui::ListBox getter signature corrected

---

## Upgrade Guide

### From v3.2.x

1. **Pull latest**: `git pull origin master`
2. **Rebuild**: 
   ```bash
   cmake-msvc.cmd --preset windows-msvc-debug
   cmake-msvc.cmd --build --preset windows-msvc-debug
   ```
3. **No API breakage** — existing code compiles without changes.

### New APIs

```cpp
// CollapsingHeader
auto header = std::make_shared<unigui::CollapsingHeader>("cfg", "Settings", true);
header->SetContentCallback([] { /* render content */ });

// Selectable
auto sel = std::make_shared<unigui::Selectable>("item1", "Option A");
if (sel->WasClicked()) { /* ... */ }

// DragFloat / DragInt
auto drag = std::make_shared<unigui::DragFloat>("vol", "Volume", 0.5f, 0.01f, 0.0f, 1.0f);
if (drag->WasChanged()) { float v = drag->GetValue(); }

// ColorEdit
auto color = std::make_shared<unigui::ColorEdit>("bg", "Background", 0.2f, 0.2f, 0.3f);

// ListBox
auto lb = std::make_shared<unigui::ListBox>("files", "Choose", std::vector<std::string>{"a.txt", "b.txt"});
lb->SetOnChange([](int idx) { /* selection changed */ });
```

---

## Build Matrix

| Preset | Compiler | Features |
|--------|----------|----------|
| `windows-msvc-debug` | MSVC | Daily dev |
| `windows-msvc-release` | MSVC | Optimized |
| `windows-clang-coverage` | Clang | Coverage report |
| `windows-clang-tidy` | Clang | Static analysis |
| `windows-msvc-debug-asan` | MSVC | AddressSanitizer |
| `windows-msvc-sdl3-vulkan-debug` | MSVC | Vulkan backend |
| `linux-gcc-debug` | GCC | Linux dev |
| `linux-gcc-debug-asan` | GCC | Linux ASAN/UBSAN |
| `macos-clang-debug` | Clang | macOS dev |

---

## Stats

| Metric | v3.2.8 | v3.3.0 |
|--------|--------|--------|
| Widgets | 68 | **74** |
| Tests | 245 | **285** |
| ID Safety | 26% | **100%** |
| CMake Presets | 6 | **10** |
| Documentation | README only | **1746-line API doc** |
| clang-format | None | **Configured** |
| clang-tidy | None | **Configured** |
| Coverage | None | **41.4% lines** |

---

**Full Changelog:** [CHANGELOG.md](CHANGELOG.md)  
**API Docs:** [docs/WIDGET_API.md](docs/WIDGET_API.md)
