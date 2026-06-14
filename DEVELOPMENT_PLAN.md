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

- 84 widgets (100% PushID-safe), the `unigui::im` immediate-mode layer (**201
  functions; A1–A6 complete** — inputs/sliders/drags, window/layout/scroll/cursor, popups/modals/menus, item & input queries, misc widgets/debug/draw-list, and the A6 remainder: tooltips, disabled scope, combo/listbox/selectable, trees/headers, tab bars, plots/progress, color editors & conversion, window-state queries — **100% of ImGui's practical surface**), declarative DSL, CSS styling engine,
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
- **Trading toolkit complete (Horizon 3).** B0–B6 done (formatting, models,
  candlestick chart, DOM/depth ladder, order ticket, blotters/watchlist/tape +
  `DataTable` freeze-pane, and the `trading_dashboard` example). Optional
  follow-ups only: a PriceTicker marquee and in-cell mini sparkline/bar renderers
  (the latter needs a custom-draw cell hook in `DataTable`).
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
- ~~**P2 · S — Warnings-as-errors.**~~ **Mostly done.** Infrastructure landed; the
  whole tree (library + tests + examples) now builds **warning-clean under GCC
  with `UNIGUI_WARNINGS_AS_ERRORS=ON`** and all 895 tests pass. `-Wextra`'s
  `missing-field-initializers` is suppressed (`-Wno-missing-field-initializers`)
  because it conflicts with the clang-tidy `readability-redundant-member-init`
  policy. _Remaining: confirm MSVC/Clang are equally clean and flip the flag on
  in a CI preset._

### Horizon 2 — Complete the ImGui wrapper (im layer first)

Goal: bring `unigui::im` to ~full ImGui coverage. The immediate layer is the
right vehicle — each function is a thin, allocation-light wrapper (no per-item
CMake/test plumbing). Add retained-mode widgets only where persistent
state/validation genuinely helps. Every phase updates the `im` tables in
`docs/API_INDEX.md` + `docs/WIDGET_API.md` and adds im-layer tests where a
headless/GL-optional path exists.

- ~~**P1 · M — A1 · Inputs / sliders / drags completeness.**~~ **Done.** Vector `*2/3/4`
  and range variants for `InputFloat/Int`, `DragFloat/Int`, `SliderFloat/Int`,
  plus `SliderAngle`, `VSliderFloat/Int`, `InputDouble`, `InputTextWithHint`.
  `unigui::im` count: 22 → 47. 14 new headless tests added.
- ~~**P1 · M — A2 · Window / layout / scroll / cursor.**~~ **Done.** `SetNextWindow*`
  (pos/size/constraints/content/collapsed/focus/scroll/bgalpha), `BeginChild/EndChild`
  (string + numeric ID overloads), scrolling (`GetScrollX/Y`, `GetScrollMaxX/Y`,
  `SetScrollX/Y`, `SetScrollHereX/Y`, `SetScrollFromPosX/Y`), `BeginGroup/EndGroup`,
  `PushClipRect/PopClipRect`, cursor get/set (`GetCursor(Screen)Pos`, `SetCursor*`,
  `GetCursorStartPos`, `GetContentRegionAvail`, `GetWindowPos/Size/Width/Height`),
  `PushItemWidth/PopItemWidth/SetNextItemWidth/CalcItemWidth`,
  `AlignTextToFramePadding`, line-metric getters, `SetItemDefaultFocus`.
  14 new headless tests added.
- ~~**P1 · M — A3 · Popups / modals / menus.**~~ **Done.** `OpenPopup` (string+numeric
  ID), `OpenPopupOnItemClick`, `BeginPopup`, `BeginPopupModal`, `EndPopup`,
  `CloseCurrentPopup`, `IsPopupOpen`, `BeginPopupContextItem/Window/Void`;
  `BeginMenuBar/EndMenuBar`, `BeginMainMenuBar/EndMainMenuBar`,
  `BeginMenu/EndMenu`, `MenuItem` (×2 overloads). 11 new headless tests added.
- ~~**P1 · M — A4 · Item & input queries.**~~ **Done.** `IsItemHovered/Active/Focused/
  Clicked/Visible/Edited/Activated/Deactivated/DeactivatedAfterEdit/ToggledOpen`,
  `IsAnyItemHovered/Active/Focused`, `GetItemRectMin/Max/Size`;
  `IsKeyDown/Pressed/Released`, `IsMouseDown/Clicked/Released/DoubleClicked/
  Dragging/HoveringRect`, `GetMousePos`, `GetMouseDragDelta`, `ResetMouseDragDelta`.
  9 new headless tests added.
- ~~**P2 · M — A5 · Misc widgets, debug tools, draw-list.**~~ **Done (widgets + debug +
  draw-list).** `InvisibleButton`, `ArrowButton`, `CheckboxFlags` (int + unsigned int),
  `ColorButton`; `ShowDemoWindow`, `ShowMetricsWindow`, `ShowStyleEditor`;
  `GetWindowDrawList`, `GetBackgroundDrawList`, `GetForegroundDrawList`.
  10 new headless tests added.
- ~~**P2 · M — A6 · Practical-surface completion + coverage tracking.**~~ **Done.**
  The remaining commonly-needed controls now have im wrappers: `TextUnformatted`/
  `TextLink`/`TextLinkOpenURL`, tooltips (`BeginTooltip`/`EndTooltip`/`SetTooltip`/
  `BeginItemTooltip`/`SetItemTooltip`), `BeginDisabled`/`EndDisabled`,
  `BeginCombo`/`EndCombo`, `BeginListBox`/`EndListBox`, `Selectable` (×2),
  `TreeNode`/`TreeNodeEx`/`TreePop`/`SetNextItemOpen`/`CollapsingHeader` (×2),
  tab bars (`BeginTabBar`/`BeginTabItem`/…), `ProgressBar`/`PlotLines`/
  `PlotHistogram`, color editors/pickers/conversion, window-state queries
  (`IsWindow*`), and misc utilities (`CalcTextSize`, `SetKeyboardFocusHere`,
  `GetTime`, `GetFrameCount`, `Set/GetMouseCursor`). `im` count 157 → **201 =
  100% of ImGui's practical surface**. New `scripts/coverage_vs_imgui.py`
  computes the figure (and the curated out-of-scope list) and runs **advisory in
  `quality.yml`** — closing the deferred `coverage-vs-imgui` CI item. 13 new
  headless test cases.

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

- ~~**P1 · S — B0 · Financial formatting.**~~ **Done.** `core/format_num.h`
  (`unigui::format`): `Thousands`/`Fixed`/`Currency`/`Percent`/`SignedDelta`/
  `TickAlign` + `Sign`/`Direction`. Pure, header-only, unit-tested
  (`tests/core/format_num_test.cc`).
- ~~**P1 · M — B1 · Lightweight models.**~~ **Done.** Header-only models under
  `include/unigui/trading/`: `order_book.h` (`OrderBook` — snapshots/deltas, best
  bid/ask, spread, mid, top-N, max-size), `ohlc_series.h` (`OhlcSeries` — tick→bar
  aggregation, rolling window, ImPlot column extractors), `quote.h`
  (`Quote`/`Position`/`Order`/`Trade`). Gated by `UNIGUI_MODULE_TRADING` (default
  OFF); tests in `tests/trading/`.
- ~~**P1 · M — B2 · Candlestick / OHLC chart.**~~ **Done.** `CandlestickChart` widget
  (`trading/candlestick_chart.h`, gated by `UNIGUI_MODULE_TRADING`) bound to a
  non-owning `OhlcSeries`: candle wicks + bodies via the plot draw-list, optional
  volume sub-panel (linked-X subplot, bull/bear-coloured bars), OHLCV crosshair
  tooltip, theme-aware background, date/time X axis, fluent `With*` API. The
  reusable low-level `unigui::trading::PlotCandlesticks()` free function is
  exposed for callers driving their own `BeginPlot/EndPlot` (kept in `trading/`
  rather than `ext/plot.h` to avoid coupling the core to `implot_internal`).
  11 new headless tests (`tests/trading/candlestick_chart_test.cc`).
- ~~**P1 · M — B3 · DOM / depth ladder.**~~ **Done.** `DepthLadder` widget
  (`trading/depth_ladder.h`, gated by `UNIGUI_MODULE_TRADING`) bound to a
  non-owning `OrderBook`: asks highest→lowest on top, optional spread/mid divider,
  bids highest→lowest below; per-level depth bars scaled to `OrderBook::MaxSize()`,
  size + side-tinted price; per-side click-to-trade callback (`SetOnLevelClick`),
  auto-/one-shot centre-on-market (`SetAutoCenter`/`CenterOnMarket`), configurable
  geometry/colours, theme-aware background, fluent `With*` API. Drawn directly via
  the window draw-list (no `DataTable` dependency, so the planned B5 freeze-pane is
  not a prerequisite). 13 new headless tests
  (`tests/trading/depth_ladder_test.cc`).
- ~~**P1 · M — B4 · Order ticket.**~~ **Done.** `OrderTicket` widget
  (`trading/order_ticket.h`, gated by `UNIGUI_MODULE_TRADING`) over an editable
  `OrderDraft` (symbol / side / `OrderType` / `TimeInForce` / qty / limit / stop):
  pure `Validate()` (→ `OrderValidation`) using `core/strutil` trim + type-
  conditional price/stop rules, `Submit()` that tick-snaps via `format::TickAlign`
  and fires the submit callback, an optional confirmation modal (`SetConfirm`),
  and a Ctrl+Enter hotkey reusing `ShortcutManager`. Coloured Buy/Sell toggle,
  type/TIF combos, disabled-when-unused price/stop inputs, inline validation
  message, fluent `With*` API. 19 new headless tests
  (`tests/trading/order_ticket_test.cc`).
- ~~**P1 · L — B5 · Blotters, watchlist, tape.**~~ **Done.** `trading/blotters.h`
  (header-only, gated by `UNIGUI_MODULE_TRADING`) ships pre-built `DataTable<T>`
  factories — `MakePositionsBlotter`, `MakeOrdersBlotter`, `MakeTradesTape`
  (time & sales), `MakeWatchlist` (quote board) — each wiring columns, a
  financial cell formatter, sign-aware cell colours with ▲/▼ delta arrows, and a
  pinned leading column. Per-row cell formatters + colour/format helpers
  (`DeltaColor`/`SideColor`/`WithArrow`/`FormatClock`) are pure & unit-tested.
  Adds **freeze-pane** to `DataTable` (`SetFrozenColumns`) — the measured gap.
  13 new headless tests (`tests/trading/blotters_test.cc`). _PriceTicker marquee
  and in-cell mini sparkline/bar renderers deferred (the latter needs a
  custom-draw cell hook in `DataTable`, which is string-cell based today)._
- ~~**P1 · M — B6 · Trading example + guide.**~~ **Done.**
  `examples/trading_dashboard` assembles B0–B5 (candlestick + volume, DOM ladder,
  order ticket, positions/orders/watchlist/time-&-sales blotters) over synthetic
  self-animating models; runnable headless via `--frames N`, gated on
  `UNIGUI_MODULE_WIDGETS` + `UNIGUI_MODULE_TRADING`. `docs/TRADING.md` covers the
  whole toolkit end to end. **Horizon 3 complete.**

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
- **Wrapper-coverage tracking:** `scripts/coverage_vs_imgui.py` (_landed_)
  reports the first-class-wrapped % of the ImGui practical surface each CI run
  (advisory in `quality.yml`); the trend should move up, never down. Currently
  **100%** of the practical surface. _Next: flip to a hard `--threshold` gate._
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
