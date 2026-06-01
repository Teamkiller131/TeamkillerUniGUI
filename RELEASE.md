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
