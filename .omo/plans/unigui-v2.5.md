# TeamkillerUniGUI v2.5 — Engineering Polish

## TL;DR

> **Quick Summary**: Engineering polish: widget_gallery showcase, Doxygen API docs generation, vcpkg port submission, multi-platform CI matrix, form_demo example, test coverage report.
> 
> **Test Target**: ≥156 tests (no regression)
> **Tasks**: ~10 tasks

---

## Phase 1: Documentation (3 tasks)

- [ ] 1. Doxygen API Documentation Generation
  **What**: Run `doxygen Doxyfile`, verify `docs/index.html` generated. Add `docs/` to .gitignore. Add docs link to README.
  **QA**: `Test-Path "docs/index.html"` → True

- [ ] 2. All-Public-API Documented
  **What**: Verify all 45 widget headers have `///` doc comments on public methods. Add missing ones.
  **QA**: `doxygen Doxyfile 2>&1` → no warnings about undocumented members

- [ ] 3. Architecture Diagram (README)
  **What**: Update README with complete 45-widget table, backend comparison matrix, theme preview description.
  **QA**: `Select-String -Path "README.md" -Pattern "45 widgets"` → found

## Phase 2: Examples (2 tasks)

- [ ] 4. widget_gallery Example
  **What**: `examples/widget_gallery/main.cc` — organized tabs showing all 45 widgets. Each tab groups related widgets. `--frames N` flag.
  **QA**: Compiles and renders with `--frames 10`

- [ ] 5. form_demo Example
  **What**: `examples/form_demo/main.cc` — demonstrates Form with all field types (Text, Checkbox, Combo, Slider, Number), validation, submit. Realistic settings form.
  **QA**: Compiles and renders with `--frames 10`

## Phase 3: CI & Quality (3 tasks)

- [ ] 6. Multi-Platform CI Matrix
  **What**: Update `.github/workflows/build.yml` with matrix: Windows (msvc), Linux (gcc), macOS (clang). Each: configure → build → test.
  **QA**: Push triggers CI, all 3 platforms pass

- [ ] 7. Code Coverage Setup
  **What**: Add `UNIGUI_COVERAGE` CMake option. Add `--coverage` flags. Document how to generate `lcov` reports.
  **QA**: `cmake -DUNIGUI_COVERAGE=ON` configures with coverage flags

- [ ] 8. Test Coverage Baseline
  **What**: Run tests with coverage, document baseline percentage in README badge.
  **QA**: Coverage report generated, ≥80% line coverage for widget code

## Phase 4: Distribution (2 tasks)

- [ ] 9. vcpkg Port Files
  **What**: Create `ports/unigui/vcpkg.json` and `ports/unigui/portfile.cmake` for vcpkg registry. Test local overlay install.
  **QA**: `vcpkg install unigui --overlay-ports=ports/unigui` succeeds

- [ ] 10. Package Version Bump
  **What**: Bump version to 0.3.0 in vcpkg.json, CMakeLists.txt, README badges, CHANGELOG.
  **QA**: `ctest --preset windows-msvc-debug` → all 156 pass

---

## Success Criteria

```bash
# Docs generated
doxygen Doxyfile  # → docs/index.html exists

# All tests pass with coverage
cmake --preset windows-msvc-debug -DUNIGUI_COVERAGE=ON
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug  # 156/156

# Both new examples build
cmake --build --preset windows-msvc-debug
# → examples/widget_gallery/widget_gallery.exe
# → examples/form_demo/form_demo.exe
```
