# TeamkillerUniGUI — Long-Term Development Plan

_Last updated: 2026-06-13 · Current version: 3.5.0 (+ Unreleased UI-beautification work)_

This document lays out a long-horizon roadmap for the project. It is meant to be
a living document: revisit it each release, check off what shipped, and re-scope
what's next. It complements — but does not replace — `CHANGELOG.md` (what
happened) and `RELEASE.md` (how we ship).

## 1. Vision

> A modern, batteries-included C++23 Dear ImGui toolkit that makes it trivial to
> build beautiful, consistent, production-grade desktop (and eventually web)
> tooling — without giving up ImGui's immediate-mode power or its raw API.

Guiding principles:

1. **ImGui-compatible, never ImGui-hostile.** Raw ImGui calls must always keep
   working and stay auto-themed. We add layers, we don't wall off the engine.
2. **Two layers, one look.** The immediate-mode (`unigui::im`) and retained-mode
   widget layers render through the same theme tokens and must stay visually
   identical.
3. **Safe by construction.** ID safety, RAII scopes, non-throwing parsers, and
   move-only guards are the default, not opt-in.
4. **Modular & embeddable.** Everything beyond the core is gated by
   `UNIGUI_MODULE_*` / `UNIGUI_BACKEND_*`; the library must build and pass tests
   with any module switched off.
5. **Cross-platform parity.** Windows is the primary target today; Linux/macOS
   are first-class; Web is the long-term frontier.
6. **Docs and tests are part of the feature.** No widget/API lands without
   reference docs, an example, and GoogleTest coverage.

## 2. Where we are today (baseline)

- 82 widgets (100% PushID-safe), immediate-mode layer, declarative DSL, CSS
  styling engine, EventBus, plugin system, font manager.
- Theme engine: Dark/Light + 13 presets, unified style/color tokens, surface
  materials (Solid/Glass/Frosted/Acrylic/Minimal), semantic colors, elevation.
- 7 backends — 4 production (GLFW+GL3, GLFW/SDL3+Vulkan, DX11, DX12); Metal,
  WebGPU, Emscripten are **stubs**.
- Optional modules: SQLite, config (TOML/JSON/INI), IPC (shared memory + ZMQ),
  network (HTTP/WebSocket).
- ~110 GoogleTest files (637 tests), benchmarks, CSV/JSON fuzz targets.
- CI: cross-platform build/test + format/tidy quality gate.
- Recent hardening: non-throwing parsers, shared `strutil`, EventBus RAII
  subscriptions, atomic settings save, Windows non-ASCII paths, CSV import perf.

### Known gaps / debt to address

- **`src/v2/` parallel implementations** (dsl, eventbus, style_engine, config,
  database, ipc, network, font_manager, plugin_manager) suggest an in-progress
  migration. The dual code paths need to be reconciled and consolidated.
- Metal / WebGPU / Emscripten backends are non-functional stubs.
- Theme "UI beautification" work is still in `Unreleased` — needs to land and be
  visually regression-tested.
- No automated visual/screenshot regression testing yet.
- API-stability guarantees are informal; there's no published semver contract
  for the public headers.

## 3. Roadmap by horizon

Horizons are intentionally relative (not calendar-locked) so the plan survives
schedule slips. Each item lists rough **effort** (S/M/L) and **priority**
(P0 critical / P1 important / P2 nice-to-have).

### Horizon 1 — Stabilize & consolidate (next 1–2 minor releases)

Goal: pay down the `v2`/duplication debt, ship the in-flight UI work, and make
the public API contract explicit.

- **P0 · M — Land the UI-beautification work.** Move the `Unreleased` theme/token
  changes into a tagged release; add before/after screenshots to docs.
- **P0 · L — Reconcile `src/v2/`.** Decide per-module whether v2 replaces or
  augments the original; delete dead paths, document the survivor, migrate tests.
- **P0 · S — Publish an API-stability policy.** Semver contract for
  `include/unigui/**`; mark experimental headers explicitly; add a deprecation
  process to `RELEASE.md`.
- **P1 · M — Visual regression harness.** Use the existing `--frames N` headless
  path to capture framebuffer snapshots per theme/widget and diff them in CI.
- **P1 · S — Coverage gate.** Wire `windows-clang-coverage` into CI and set a
  floor; track coverage per module.
- **P1 · M — Expand fuzzing.** Beyond CSV/JSON, fuzz the CSS style engine, the
  config (TOML/INI) parsers, and DSL input paths.
- **P2 · S — Warnings-as-errors** across all presets where compilers agree.

### Horizon 2 — Backend completeness & performance (2–4 releases out)

Goal: turn the stub backends into real ones and make rendering measurably fast.

- **P1 · L — Implement the Metal renderer** (macOS), replacing the stub; validate
  against the Vulkan/MoltenVK path.
- **P1 · L — Implement WebGPU + Emscripten** for a working browser target; ship a
  web demo of `widget_gallery`.
- **P1 · M — DPI / multi-monitor robustness.** Per-monitor DPI scaling, fractional
  scaling, runtime DPI changes across all production backends.
- **P1 · M — Performance budget & benchmarks.** Per-frame CPU budgets for
  `DataTable`/`VirtualList`/`Table` at 100k+ rows; expand `tests/bench/`; track
  regressions in CI.
- **P2 · M — GPU-side text/MSAA improvements** and a shared backend capability
  query so features degrade gracefully per renderer.

### Horizon 3 — Capability growth (4–8 releases out)

Goal: broaden what apps can build without leaving the toolkit.

- **P1 · L — Layout system.** A constraint/flex layout pass on top of the DSL so
  UIs reflow without manual sizing (complements VBox/HBox).
- **P1 · M — Accessibility.** Surface the existing `AccessibleName`/`Description`
  fields through a real a11y tree / screen-reader bridge where the platform
  allows; keyboard-only navigation audit.
- **P1 · M — Theming authoring tools.** Live theme editor example; export/import
  theme files; hot-reload CSS from disk.
- **P2 · L — Data binding / reactive layer.** Optional observable bindings so
  retained widgets update from model changes without manual `Set*` calls.
- **P2 · M — Internationalization.** Build out `core/locale.h` into a full
  catalog/translation system; RTL layout support.
- **P2 · M — Plugin ecosystem.** Stable plugin ABI, versioned plugin interface,
  sample third-party plugins, and a plugin template repo.

### Horizon 4 — Ecosystem & reach (long term)

Goal: make UniGUI easy to adopt and contribute to at scale.

- **P2 · M — Packaging.** Publish to vcpkg registry and/or a Conan package;
  versioned binary releases; `find_package(unigui)` story hardening.
- **P2 · M — Language bindings.** Explore C API + bindings (e.g. C#, Python) over
  a stable C ABI surface.
- **P2 · L — Designer / live-preview tool.** Standalone app that previews DSL/CSS
  and emits code.
- **P2 · S — Community.** Contribution ladder, "good first issue" curation,
  governance doc, public roadmap board mirroring this file.

## 4. Cross-cutting workstreams (continuous)

These run in parallel with every horizon:

- **Quality:** keep CI green on all presets; grow coverage; expand fuzz/bench;
  zero `clang-tidy` regressions.
- **Docs:** every API change updates `docs/` + README badges/counts in the same
  PR; keep `README_zh.md` in sync with `README.md`.
- **Security/robustness:** no throwing parsers, no unchecked OS calls outside the
  backend layer, atomic file writes, bounds-safe widget rendering.
- **Performance:** profile hot paths (tables/lists/parsers); guard against
  regressions with benchmarks.
- **Dependency hygiene:** track the vcpkg baseline and ImGui version; test
  upgrades behind a branch before bumping.

## 5. Release cadence & versioning

- **Semantic versioning** for the public headers. Breaking changes to
  `include/unigui/**` require a major bump and a deprecation cycle.
- Maintain `CHANGELOG.md` continuously under `Unreleased`; cut a release by
  moving it under a version heading (see `RELEASE.md`).
- Each release should state: supported platforms/backends, ImGui version, test
  count, and any deprecations.
- Roughly minor releases for features, patch releases for fixes; no fixed
  calendar — ship when a horizon item is done, tested, and documented.

## 6. Success metrics

Track these over time to know the plan is working:

- **Stability:** CI green rate; number of `v2` duplicate code paths remaining
  (target: 0); count of experimental vs. stable public headers.
- **Quality:** test count & coverage %; fuzz targets; open P0/P1 bug count.
- **Performance:** frame time for `DataTable`/`VirtualList` at 100k rows; parser
  throughput (CSV/JSON).
- **Reach:** functional backends (target: 7/7); platforms with a passing test
  suite; packaging channels available.
- **Adoption:** examples that build on web; external plugins; downstream
  embedders.

## 7. How to use this document

- When picking up work, start from the **highest-priority item in the lowest open
  horizon** unless a release-blocking bug takes precedence.
- When you complete an item, check it off here, add a line to `CHANGELOG.md`, and
  update any affected docs/badges in the same PR.
- Re-scope horizons at each release: promote, demote, or split items as reality
  dictates. Keep the vision (§1) stable; let the tactics move.
</content>
