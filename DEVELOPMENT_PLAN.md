# TeamkillerUniGUI — Long-Term Development Plan

_Last updated: 2026-06-14 · Current version: 3.5.0 (+ Unreleased work)_

This document lays out a long-horizon roadmap for the project. It is meant to be
a living document: revisit it each release, check off what shipped, and re-scope
what's next. It complements — but does not replace — `CHANGELOG.md` (what
happened) and `RELEASE.md` (per-release notes).

Two new headline goals drive the near-term roadmap:

1. **Complete the wrapper.** Provide first-class UniGUI wrappers for the *full*
   Dear ImGui surface (today only ~¼ of ImGui's ~450–500 public functions have
   one), so users rarely need to drop to raw `ImGui::` — while raw ImGui stays
   fully supported and auto-themed.
2. **A trading-client toolkit.** Ship batteries-included, presentation-layer
   building blocks for data-dense, real-time trading UIs (order ticket,
   candlestick/OHLC chart, depth-of-market ladder, blotters/watchlist), backed
   by small in-memory models — without becoming a trading app itself.

## 1. Vision

> A modern, batteries-included C++23 Dear ImGui toolkit that makes it trivial to
> build beautiful, consistent, production-grade desktop (and eventually web)
> tooling — without giving up ImGui's immediate-mode power or its raw API.

Guiding principles:

1. **ImGui-compatible, never ImGui-hostile.** Raw ImGui calls must always keep
   working and stay auto-themed. We add layers, we don't wall off the engine.
   The goal of "complete the wrapper" is to make raw ImGui *unnecessary*, never
   *unavailable*.
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
7. **Domain-ready.** UniGUI ships batteries-included building blocks for
   data-dense, real-time domains — starting with trading/finance — without
   turning into a trading app itself: widgets are presentation + thin models;
   the market feed, order routing, and strategy stay in the embedder.

## 2. Where we are today (baseline)

- 84 widgets (100% PushID-safe), the `unigui::im` immediate-mode layer (≈35
  functions ≈ ¼ of ImGui's surface), declarative DSL, CSS styling engine,
  EventBus, plugin system, font manager.
- Theme engine: Dark/Light + 13 presets, unified style/color tokens, surface
  materials (Solid/Glass/Frosted/Acrylic/Minimal), semantic colors, elevation.
- 7 backends — 4 production (GLFW+GL3, GLFW/SDL3+Vulkan, DX11, DX12); Metal,
  WebGPU, Emscripten are **stubs**.
- Optional modules: SQLite, config (TOML/JSON/INI), IPC (shared memory + ZMQ),
  network (HTTP/WebSocket).
- ~110 GoogleTest files, benchmarks, and fuzz targets (CSV/JSON/CSS/config).
- CI: cross-platform build/test + format/tidy quality gate + advisory coverage.

### Recently completed

- **`src/v2/` consolidated.** The `v2/` directory was a naming holdover from the
  v2.9.0 namespace removal, not a second code path; its files were the *sole*
  implementations of `dsl`/`styling`/`events`/`plugin`/`config`/`sqlite`/`ipc`/
  `network`/`font_manager`. They (and their tests) now live in the mirrored
  `src/<module>/` and `tests/<module>/` layout — 0 duplicate paths remain.
- **API-stability policy.** `docs/API_STABILITY.md` defines the semver contract
  for `include/unigui/**`, three stability tiers, and the deprecation lifecycle;
  `<unigui/core/api.h>` adds `UNIGUI_DEPRECATED`/`UNIGUI_EXPERIMENTAL`/
  `UNIGUI_INTERNAL` markers; `core/version.h` gains `UNIGUI_VERSION_NUMBER`/
  `UNIGUI_VERSION_AT_LEAST`.
- **Expanded fuzzing.** `test_css_fuzz` (style engine) and `test_config_fuzz`
  (TOML/JSON/INI) join the CSV/JSON targets.
- **Warnings-as-errors infrastructure.** `cmake/CompilerWarnings.cmake` + opt-in
  `UNIGUI_WARNINGS_AS_ERRORS` + `linux-gcc-debug-werror` preset.
- **Advisory CI coverage floor** in `quality.yml` (`COVERAGE_FLOOR`).

### Known gaps / debt

- **ImGui wrapper is ~¼ complete.** ~250+ lower-level/utility/vector functions
  have no first-class wrapper: vector & scalar `Input/Slider/Drag` variants,
  window/scroll/cursor control, generic popups/modals, item & input queries,
  draw-list access, debug tool windows, and a few misc widgets
  (`InvisibleButton`, `ArrowButton`, `CheckboxFlags`, `ColorButton`).
- **No trading toolkit.** No candlestick/OHLC chart, no depth-of-market ladder,
  no order ticket, no blotter/watchlist/tape templates, no general number/
  currency/percent formatting, no OHLC aggregation model, no trading example.
  (Strong existing building blocks to reuse — see Horizon 3.)
- Metal / WebGPU / Emscripten backends are non-functional stubs.
- Theme "UI beautification" work is still in `Unreleased` — needs to land and be
  visually regression-tested.
- No automated visual/screenshot regression testing yet.

## 3. Roadmap by horizon

Horizons are intentionally relative (not calendar-locked) so the plan survives
schedule slips. Each item lists rough **effort** (S/M/L) and **priority**
(P0 critical / P1 important / P2 nice-to-have). Each phase ships as its own minor
release with tests + docs.

### Horizon 1 — Finish stabilization (carry-over, short)

- **P0 · M — Land the UI-beautification work.** Move the `Unreleased` theme/token
  changes into a tagged release; add before/after screenshots to docs. _(Held —
  needs a real build + GPU for screenshots + a version-cut decision.)_
- **P1 · M — Visual regression harness.** Use the `--frames N` headless path to
  capture framebuffer snapshots per theme/widget and diff them in CI. _(Needs a
  GPU-capable CI runner.)_
- **P1 · S — Coverage gate.** _Advisory landed._ Confirm the headless baseline,
  raise `COVERAGE_FLOOR` to just under it, then flip the step to a hard `exit 1`.
- **P2 · S — Warnings-as-errors.** _Infrastructure landed._ Verify the tree is
  warning-clean per compiler, flip `UNIGUI_WARNINGS_AS_ERRORS` on in CI, and
  extend `unigui_set_warnings()` to the tests/examples targets.

### Horizon 2 — Complete the ImGui wrapper (im layer first)

Goal: bring `unigui::im` to ~full ImGui coverage. The immediate layer is the
right vehicle — each function is a thin, allocation-light wrapper (no per-item
CMake/test plumbing). Add retained-mode widgets only where persistent
state/validation genuinely helps. Every phase updates the `im` tables in
`docs/API_INDEX.md` + `docs/WIDGET_API.md` and adds im-layer tests where a
headless/GL-optional path exists.

- **P1 · M — A1 · Inputs / sliders / drags completeness.** Vector `*2/3/4`,
  `*Scalar`, and range variants for `InputFloat/Int`, `DragFloat/Int`,
  `SliderFloat/Int`, plus `SliderAngle`, `VSliderFloat/Int`, `InputDouble`,
  `InputTextWithHint`.
- **P1 · M — A2 · Window / layout / scroll / cursor.** `SetNextWindow*`,
  `BeginChild/EndChild`, scrolling (`SetScrollHereY`, `GetScrollX/Y`…),
  `BeginGroup/EndGroup`, clip rects, cursor get/set, alignment helpers.
- **P1 · M — A3 · Popups / modals / menus.** Generic `OpenPopup`/`BeginPopup`/
  `BeginPopupModal`/context popups, plus menu builder helpers.
- **P1 · M — A4 · Item & input queries.** `IsItem*`, `GetItemRect*`, and
  keyboard/mouse query (`IsKey*`, `IsMouse*`, `GetMouse*`), layered with the
  existing `Shortcut` widget.
- **P2 · M — A5 · Misc widgets, debug tools, draw-list.** `InvisibleButton`,
  `ArrowButton`, `CheckboxFlags`, `ColorButton`; `ShowDemoWindow`/`Metrics`/
  `StyleEditor`; foreground/background draw-list access + a thin `ImDrawList`
  facade. Land a `coverage-vs-imgui` script (runs in CI where vcpkg is
  available) that diffs the installed `imgui.h` against `include/unigui/**` to
  report wrapper coverage % and guard against regressions.

### Horizon 3 — Trading-client toolkit

Goal: ship the four trading interface families plus the thin models they bind
to. New module gated by `UNIGUI_MODULE_TRADING` (default OFF; the library must
still build/test with it off). Headers under `include/unigui/trading/`, impls
under `src/trading/`, tests under `tests/trading/`, mirroring the project
convention. Widgets are presentation-only; streaming uses the existing
`core/main_thread.h` dispatcher + the `network/` module.

Reuse existing building blocks rather than rebuilding: `DataTable<T>` (virtual
scroll, `FlashRow`, row/cell color, checkbox columns, groups, context menu),
`TimeSeriesChart`, `SliderBar`/`MultiHandleSlider`, `RiskBar`/`FuturesRiskBar`,
`StatusLamp`, `SpinBox<T>`, `Table`, `core/locale.h`, `ext/plot.h`.

- **P1 · S — B0 · Financial formatting.** New `core/format_num.h` in
  `unigui::format` — thousands separators, currency (symbol + precision; USD/
  EUR/JPY/…), percentage, signed delta with up/down color hint, pip/tick;
  reusing `core/strutil.h`. Pure functions → **fully unit-testable headlessly;
  recommended first slice.**
- **P1 · M — B1 · Lightweight models.** `trading/order_book.h` (price levels;
  apply snapshot/delta; best bid/ask/spread), `trading/ohlc_series.h` (rolling
  OHLC aggregator from ticks by bar interval), `trading/quote.h` + position/
  order/trade row types. Pure data structures → **unit-testable headlessly.**
- **P1 · M — B2 · Candlestick / OHLC chart.** Widget bound to `ohlc_series`,
  wrapping ImPlot `PlotCandlestick` with an optional volume sub-panel; also
  expose a candlestick helper in `ext/plot.h`.
- **P1 · M — B3 · DOM / depth ladder.** Price-ladder widget bound to
  `order_book`: bid/ask size bars, center-on-last, click-to-trade column
  callbacks. Built on the `DataTable`/`Table` machinery + the freeze-pane
  support added in B5.
- **P1 · M — B4 · Order ticket.** Order-entry widget (side / qty / price / type
  / TIF) with `strutil` validation, submit/confirm callbacks, and keyboard/
  hotkey flow (reusing `Shortcut`); complements the existing `SliderBar`
  position workflow.
- **P1 · L — B5 · Blotters, watchlist, tape, ticker.** Pre-built `DataTable`
  configurations — PositionsBlotter, OrdersBlotter, TradesTape (time & sales),
  Watchlist/QuoteBoard, PriceTicker — plus finance cell renderers (delta arrows,
  mini sparkline, mini bar). Adds **freeze-pane** (pinned first-N columns) to
  `DataTable`, a measured gap both B3 and B5 need.
- **P1 · M — B6 · Trading example + guide.** `examples/trading_dashboard`
  assembling B0–B5 (chart + DOM + ticket + blotters + risk/status), runnable
  headless via `--frames N`; new `docs/TRADING.md` guide.

### Horizon 4 — Backend completeness & performance

Goal: turn the stub backends into real ones and make rendering measurably fast.

- **P1 · L — Implement the Metal renderer** (macOS), replacing the stub; validate
  against the Vulkan/MoltenVK path.
- **P1 · L — Implement WebGPU + Emscripten** for a working browser target; ship a
  web demo of `widget_gallery`.
- **P1 · M — DPI / multi-monitor robustness.** Per-monitor DPI scaling, fractional
  scaling, runtime DPI changes across all production backends.
- **P1 · M — Performance budget & benchmarks.** Per-frame CPU budgets for
  `DataTable`/`VirtualList`/`Table` — and the trading DOM/blotters under high
  update rates — at 100k+ rows; expand `tests/bench/`; track regressions in CI.
- **P2 · M — GPU-side text/MSAA improvements** and a shared backend capability
  query so features degrade gracefully per renderer.

### Horizon 5 — Capability growth

Goal: broaden what apps can build without leaving the toolkit.

- **P1 · L — Layout system.** A constraint/flex layout pass on top of the DSL so
  UIs reflow without manual sizing (complements VBox/HBox).
- **P1 · M — Accessibility.** Surface the existing `AccessibleName`/`Description`
  fields through a real a11y tree / screen-reader bridge where the platform
  allows; keyboard-only navigation audit.
- **P1 · M — Theming authoring tools.** Live theme editor example; export/import
  theme files; hot-reload CSS from disk.
- **P2 · L — Data binding / reactive layer.** Optional observable bindings so
  retained widgets update from model changes without manual `Set*` calls —
  synergizes directly with the Horizon-3 trading models.
- **P2 · M — Internationalization.** Build out `core/locale.h` into a full
  catalog/translation system; RTL layout support.
- **P2 · M — Plugin ecosystem.** Stable plugin ABI, versioned plugin interface,
  sample third-party plugins, and a plugin template repo.

### Horizon 6 — Ecosystem & reach (long term)

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
- **Wrapper-coverage tracking:** the `coverage-vs-imgui` script (Horizon 2)
  reports the first-class-wrapped % of the ImGui surface each CI run; the trend
  should move up, never down.
- **Trading module hygiene:** the library must build and pass tests with
  `UNIGUI_MODULE_TRADING=OFF`; trading widgets stay presentation-only; the
  models are header-light and unit-tested.
- **Docs:** every API change updates `docs/` + README badges/counts in the same
  PR; keep `README_zh.md` in sync with `README.md`.
- **Security/robustness:** no throwing parsers, no unchecked OS calls outside the
  backend layer, atomic file writes, bounds-safe widget rendering.
- **Performance:** profile hot paths (tables/lists/parsers/feeds); guard against
  regressions with benchmarks.
- **Dependency hygiene:** track the vcpkg baseline and ImGui/ImPlot versions;
  test upgrades behind a branch before bumping.

## 5. Release cadence & versioning

- **Semantic versioning** for the public headers, per `docs/API_STABILITY.md`.
  Breaking changes to `include/unigui/**` require a major bump and a deprecation
  cycle.
- Maintain `CHANGELOG.md` continuously under `Unreleased`; cut a release by
  moving it under a version heading and writing notes in `RELEASE.md`.
- Each release should state: supported platforms/backends, ImGui version, test
  count, and any deprecations.
- Roughly minor releases for features, patch releases for fixes; no fixed
  calendar — ship when a horizon item is done, tested, and documented.

## 6. Success metrics

Track these over time to know the plan is working:

- **Wrapper coverage:** first-class-wrapped % of the ImGui public surface
  (target: ≥ ~90% of the practical surface), tracked by `coverage-vs-imgui`.
- **Trading toolkit:** completeness — 4/4 widget families (ticket, OHLC chart,
  DOM ladder, blotters) + models + a runnable example shipped.
- **Stability:** CI green rate; count of experimental vs. stable public headers;
  `v2` duplicate code paths remaining (target met: 0).
- **Quality:** test count & coverage %; fuzz targets; open P0/P1 bug count.
- **Performance:** frame time for `DataTable`/`VirtualList`/DOM at 100k rows and
  high update rates; parser throughput (CSV/JSON).
- **Reach:** functional backends (target: 7/7); platforms with a passing test
  suite; packaging channels available.
- **Adoption:** examples that build on web; external plugins; downstream
  embedders.

## 7. How to use this document

- When picking up work, start from the **highest-priority item in the lowest open
  horizon** unless a release-blocking bug takes precedence. Within the current
  push, the recommended first slices are **B0 (financial formatting)** and **A1
  (im input/slider completeness)** — both are fully unit-testable headlessly.
- When you complete an item, check it off here, add a line to `CHANGELOG.md`, and
  update any affected docs/badges in the same PR.
- Re-scope horizons at each release: promote, demote, or split items as reality
  dictates. Keep the vision (§1) stable; let the tactics move.
</content>
