# TeamkillerUniGUI v2.9 — Enterprise & Polish

## TL;DR

> **Quick Summary**: Localization/i18n, settings persistence API, undo/redo system (global), serialization for Form/Table data, accessibility features, release preparation.
> 
> **Test Target**: ≥185 tests (175 + ≥10 new)
> **Tasks**: ~10 tasks

---

## Phase 1: Enterprise Features (5 tasks)

- [ ] 1. Localization / i18n System
  **What**: `unigui::Locale` — string translation table. `SetLocale("zh_CN")`, `Tr("key")`. Widget labels support translation keys.
  **QA**: `ctest -R Locale` → 2 tests

- [ ] 2. Settings Persistence API
  **What**: `unigui::Settings` — key-value store. `Save("window.x", 100)`, `Load("window.x")`. JSON or INI based. Auto-save on shutdown.
  **QA**: `ctest -R Settings` → 2 tests

- [ ] 3. Global Undo/Redo System
  **What**: `unigui::UndoStack` — command pattern. `Execute(UndoableCommand)`, `Undo()`, `Redo()`. Pluggable into any widget.
  **QA**: `ctest -R UndoStack` → 2 tests

- [ ] 4. Form Serialization
  **What**: Form: `Serialize()`/`Deserialize(json)` — export/import form state as JSON string.
  **QA**: `ctest -R FormTest` → +2 tests

- [ ] 5. Table Data Export
  **What**: Table: `ExportCSV()`/`ImportCSV()`. Copy to clipboard.
  **QA**: `ctest -R TableTest` → +2 tests

## Phase 2: Polish (3 tasks)

- [ ] 6. Accessibility Hints
  **What**: Widget: `SetAccessibleName()`, `SetAccessibleDescription()`. Screen reader support via ImGui native accessibility API.
  **QA**: Compiles, accessibility hints set

- [ ] 7. Widget Size Constraints
  **What**: Widget: `SetMinSize()`, `SetMaxSize()`. Standardize across all widgets.
  **QA**: `ctest -R Widget` → +1 test

- [ ] 8. Theme Preset Export
  **What**: `Theme::ExportJSON()` / `Theme::ImportJSON()`. Save/load custom color schemes.
  **QA**: `ctest -R ThemeTest` → +2 tests

## Phase 3: Release (2 tasks)

- [ ] 9. Performance Benchmarks
  **What**: `tests/bench/` — frame time benchmarks for 100+ widgets. CI regression detection.
  **QA**: Benchmarks run in CI, no >10% regression

- [ ] 10. v1.0.0 Release Preparation
  **What**: Version bump to 1.0.0. Final CHANGELOG. Release notes. Tag `v1.0.0`.
  **QA**: All 185+ tests pass, all 4+ examples build

---

## Scope

### INCLUDE: i18n, settings, undo/redo, serialization, accessibility, benchmarks, v1.0.0
### EXCLUDE: Plugin system, hot-reload, network features, database integration
