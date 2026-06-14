# CLAUDE.md

Guidance for Claude Code (and other AI agents) working in this repository.
Read this first — it captures the architecture, conventions, and workflows
that aren't obvious from any single file.

## Project at a glance

**TeamkillerUniGUI** is a C++23 [Dear ImGui](https://github.com/ocornut/imgui)
wrapper (v3.5.0). It layers a unified dark+light theme engine, a high-level
retained-mode widget library (82 widgets), an immediate-mode helper layer
(`unigui::im`), a declarative DSL, CSS-like styling, an EventBus, a plugin
system, and optional sub-modules (SQLite, config, IPC, networking) on top of
ImGui. It targets Windows, Linux, macOS, and Web via a pluggable backend
abstraction (GLFW/SDL3 platforms × OpenGL3/Vulkan/DX11/DX12/Metal/WebGPU
renderers).

- Language/standard: **C++23**
- Build: **CMake 3.31+** + **Ninja**, dependencies via **vcpkg** (manifest mode)
- Tests: **GoogleTest** (`ctest`), ~110 test files
- Compilers: MSVC 19.40+ / GCC 14+ / Clang 18+
- License: MIT (bundled font under SIL OFL 1.1)

## Repository layout

```
include/unigui/      Public headers (the API surface — keep stable & documented)
  core/              scope.h, strutil.h, locale.h, undo_stack.h, version.h
  theme/             theme.h, *_tokens.h, surface_style.h, presets/*
  widgets/           (widget headers live under include/unigui/… per module)
  im/                Immediate-mode free functions (unigui::im)
  dsl/               Declarative DSL builders (unigui::dsl)
  events/            EventBus (unigui::events)
  styling/           CSS-like style engine (unigui::styling)
  plugin/            Plugin interface + manager (unigui::plugin)
  fonts/, config/, sqlite/, ipc/, network/, backend/, fx/, app/, ext/
src/                 Implementation (.cc) mirroring the include tree
  widgets/           One .cc per widget (button.cc, table.cc, …)
  backend/           Platform + renderer backends, backend_registry.cc
  core/, theme/, fx/, im/, fonts/, app/
  dsl/, styling/, events/, plugin/, config/, sqlite/, ipc/, network/
                     Optional-module impls, each mirroring include/unigui/<module>/
examples/            hello_unigui, widget_gallery, theme_demo, form_demo,
                     plot_demo, plugin_example, v3_overview, vulkan_triangle, …
tests/               GoogleTest suites mirroring src (widgets/, core/, theme/,
                     backend/, fx/, im/, dsl/, styling/, events/, plugin/, config/,
                     sqlite/, ipc/, integration_test.cc, bench/, fuzz/)
docs/                API_INDEX, WIDGET_API, WIDGET_EXAMPLES, GETTING_STARTED, …
scripts/             check_env.ps1, build.ps1, helpers
cmake/, ports/       CMake helpers and vcpkg port overlays
.github/workflows/   build.yml (cross-platform), quality.yml (format/tidy)
```

## Build & test

vcpkg must be available (`$VCPKG_ROOT` / `$env:VCPKG_ROOT`). Builds are driven
by **CMake presets** (see `CMakePresets.json`).

### Linux / macOS
```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```
Or use the named presets: `linux-gcc-debug`, `linux-gcc-debug-asan`,
`macos-clang-debug`.

### Windows (MSVC)
Always go through the MSVC wrapper so the toolset is pinned correctly:
```powershell
pwsh -File scripts/check_env.ps1            # toolchain self-check first
pwsh -File scripts/build.ps1 -Preset windows-msvc-debug -Test
# or manually:
cmake-msvc.cmd --preset windows-msvc-debug
cmake-msvc.cmd --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure
```

### Useful presets
| Purpose | Preset |
|---------|--------|
| MSVC debug/release | `windows-msvc-debug`, `windows-msvc-release` |
| AddressSanitizer | `windows-msvc-debug-asan`, `linux-gcc-debug-asan` |
| clang-tidy lint | `windows-clang-tidy` |
| Coverage | `windows-clang-coverage` (target `coverage`) |
| Warnings-as-errors | `linux-gcc-debug-werror` (or `-DUNIGUI_WARNINGS_AS_ERRORS=ON`) |
| SDL3 + Vulkan | `windows-msvc-sdl3-vulkan-{debug,release}` |

### Run an example (headless-friendly)
Examples accept `--frames N` to render N frames and exit — use this in CI/smoke:
```bash
./build/examples/hello_unigui/hello_unigui --frames 10
```

## Conventions (follow these when editing)

- **C++23, standard-first.** Avoid platform-specific APIs outside `src/backend/`.
  Keep OS-specific code behind the `PlatformBackend` / `RendererBackend`
  interfaces.
- **ID safety is mandatory.** Every widget scopes its ImGui IDs via
  `PushID(name)/PopID()`. New widgets must do the same; give each instance a
  unique name rather than relying on labels. Prefer the RAII `IDScope`/
  `WindowScope`/… guards from `<unigui/core/scope.h>` over manual `Begin/End`.
- **No throwing parsers.** Use the helpers in `<unigui/core/strutil.h>` instead
  of `std::stoi/stof/stod` — recent hardening replaced unsafe conversions with
  non-throwing variants. Don't reintroduce throwing conversions.
- **Header/impl mirror.** A public header in `include/unigui/X/foo.h` has its
  implementation in `src/X/foo.cc`. Keep the public API in the header minimal,
  documented, and stable.
- **Fluent API.** Widgets expose chainable `With*` setters (base helpers return
  `Widget&`; CRTP `FluentWidget<Derived>` widgets like `Button` preserve the
  derived type). Match this style when adding configuration.
- **Two UI layers coexist.** `unigui::im` = stateless immediate-mode controls;
  retained-mode widget classes = persistent state, validation, undo/redo,
  serialization. Don't collapse them; pick the right layer for the change.
- **Formatting & lint.** Run `clang-format -i` on changed files (config in
  `.clang-format`). `.clang-tidy` is enforced by the `windows-clang-tidy`
  preset and the `quality.yml` workflow.
- **Tests required.** Add/adjust GoogleTest cases under `tests/` (mirroring the
  source dir) for any behavior change. Performance-sensitive code has
  benchmarks under `tests/bench/`; parsers have fuzz targets under `tests/fuzz/`.
- **Modularity.** Features are gated by `UNIGUI_MODULE_*` / `UNIGUI_BACKEND_*`
  CMake options. New optional functionality should be guard-able the same way,
  and must still compile with the module switched off.
- **API stability.** `include/unigui/**` is a semver-governed contract — see
  `docs/API_STABILITY.md`. Don't break stable APIs in a minor/patch; deprecate
  with `UNIGUI_DEPRECATED("…")` (from `<unigui/core/api.h>`) and remove only in a
  major. Mark unsettled APIs `UNIGUI_EXPERIMENTAL`. Bump `core/version.h` +
  `vcpkg.json` together.
- **Changelog.** User-visible changes go in `CHANGELOG.md` under the
  `Unreleased` section (Added/Changed/Fixed), in the existing prose style.

## Documentation map

| File | What it covers |
|------|----------------|
| `README.md` / `README_zh.md` | Overview, quick start, full widget table |
| `docs/README.md` | Documentation hub / index |
| `docs/GETTING_STARTED.md` | Build + first app |
| `docs/WIDGET_API.md` | Full widget API reference (all 82) |
| `docs/WIDGET_EXAMPLES.md` | One minimal example per widget |
| `docs/API_INDEX.md` | Master index (widgets + `im` + DSL + core) |
| `docs/MODULES.md` | Sub-module overview |
| `docs/API_STABILITY.md` | **Public-API contract (semver, tiers, deprecation lifecycle)** |
| `docs/TROUBLESHOOTING.md` | Build / CRT / CI FAQ |
| `INTEGRATION.md` | Submodule + vcpkg embedding |
| `RELEASE.md` | Per-release notes |
| `DEVELOPMENT_PLAN.md` | **Long-term roadmap (read for direction)** |

When you add or change a widget/API, update the relevant docs **and** the
counts/badges in `README.md` (widget count, test count, version) so they stay
accurate.

## Git & contribution workflow

- Branch from `master` with a typed name: `feat/…`, `fix/…`, `docs/…`,
  `perf/…`, `refactor/…`, `test/…` (matches existing commit history).
- Commit messages use Conventional-Commit-style prefixes
  (`feat(table): …`, `fix(settings): …`, `perf(table): …`, `test(fuzz): …`).
  Explain the *why*, keep commits focused.
- Ensure the full test suite passes and CI (`build.yml`, `quality.yml`) is green
  before opening a PR against `master`; fill in the PR template.
- **Do not open a pull request unless explicitly asked.**

## Things that commonly trip people up

- On Windows, configure through `cmake-msvc.cmd` (not bare `cmake`) so the MSVC
  environment is set up; `scripts/check_env.ps1` flags stale toolsets on `PATH`.
- Translucent surface materials (Glass/Frosted/Acrylic) must render against the
  theme backdrop — the app loop clears every backend to `GetBackdropColor()`.
  Don't hard-code a black/opaque clear.
- DX11/DX12 backends are Windows-only; disable them on Linux/macOS
  (`-DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF`).
- Some GL-context tests are skipped in headless Linux CI — that's expected.
- Metal/WebGPU/Emscripten renderers are **stubs**; don't assume they're
  functional when wiring features through the backend layer.
</content>
</invoke>
