# TeamkillerUniGUI — Long-Term Development Plan

_Last updated: 2026-06-29 · Current version: 4.1.1_

This document lays out a long-horizon roadmap for the project. It is meant to be
a living document: revisit it each release, check off what shipped, and re-scope
what's next. It complements — but does not replace — `CHANGELOG.md` (what
happened) and `RELEASE.md` (per-release notes).

The project's two original headline goals are **achieved**:

1. ~~**Complete the wrapper.**~~ **Done (Horizon 2).** `unigui::im` now wraps **100%
   of ImGui's practical surface** (201 functions, A1–A6). Raw `ImGui::` stays fully
   supported and auto-themed.
2. ~~**A trading-client toolkit.**~~ **Done (Horizon 3).** The four trading widget
   families (order ticket, candlestick/OHLC chart, depth ladder, blotters) + thin
   models + the `trading_dashboard` example shipped.

Since then the toolkit became an **opinionated application framework** (Horizon 5:
Component/State/Store/Navigator + reactive + layout + theming tools), passed a
**multi-dimension correctness/security audit** (P0–P3 all fixed and shipped across
3.17–3.19, plus the 4.0 `Result<T>` → `std::expected` major), and had its
**cross-platform backends hardened and verified at runtime in CI**. The **Metal**
renderer and the **Emscripten/WebGL2** web backend are now real (4.2.0; the latter
build-verified in CI with a downloadable `web_demo` artifact). The near-term focus
shifts to the remaining frontier: the **WebGPU** renderer, **accessibility**,
**visual-regression** testing, and **packaging reach**.

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

- **95 widgets** (100% PushID-safe), the `unigui::im` immediate layer (**201
  functions = 100% of ImGui's practical surface**, A1–A6 complete), declarative DSL,
  CSS styling engine, EventBus, plugin system, font manager.
- **Application framework** (`unigui::dsl`): `Component` + reactive `State<T>`,
  `Store<T>`, `Navigator`, `Watch`/`OnCleanup`, the live `DrawInspector()` overlay,
  and the `Custom` escape hatch — see `docs/FRAMEWORK.md`.
- **Reactive + layout cross-cutting layers**: `core/observable.h`
  (`Observable`/`Computed`/`Bind` + first-class widget binding) and
  `core/flex_layout.h` (`SolveFlex`/`SolveFlexWrap` + `Layout::FlexRow` + `dsl::Flex`).
- Theme engine: Dark/Light + 13 presets, unified style/color tokens, surface
  materials, semantic colors, elevation; theme export/import + CSS hot-reload + a live
  `theme_editor`.
- 7 backends — 6 functional (GLFW+GL3, GLFW/SDL3+Vulkan, DX11, DX12, **Metal**, and the
  **Emscripten/WebGL2** web path), all hardened on their failure paths. Only **WebGPU**
  stays a stub that fails cleanly. The default GLFW+OpenGL3 path is **CI-verified to run
  on Linux** (headless xvfb + llvmpipe smoke) and its identical GL code path covers
  macOS; the **wasm build is CI-verified** (emsdk + `emcmake`, with a `web_demo`
  artifact). Metal is build-verified on the macOS CI runner.
- **`Result<T>` is `std::expected<T, ErrorCode>`** (4.0): errors via `Err()`, monadic
  ops, throwing `value()`; adopted in `sqlite::Database::Open` and `config::Store::Load*`.
- Optional modules — SQLite, config (TOML/JSON/INI), IPC (shmem + ZMQ), network
  (HTTP/WebSocket) — **now build & test on Windows** via the
  `windows-msvc-debug-modules` preset (they were bit-rotted/unbuilt before the audit).
- ~155 GoogleTest files (**1169** default-preset cases), benchmarks (incl. an LTTB
  perf budget) and fuzz targets (CSV/JSON/CSS/config).
- CI (**all green**): cross-platform build/test (Win/Linux/macOS) + a **headless
  backend smoke that proves the GL path actually runs** + warnings-as-errors on **both
  GCC and MSVC** + install-consume packaging + **clang-tidy** (pinned 19) + advisory
  coverage.

### Recently completed

- **Multi-dimension correctness/security audit — P0–P3, shipped 3.17.0–3.19.0.** A
  49-agent audit (each finding adversarially verified) surfaced and fixed, with
  regression tests: a reachable **IPC integer-overflow OOB read/write** (security), a
  **TabWidget use-after-free**, a **WebSocket callback data race**, a **CSS gradient
  `std::out_of_range`**, banned `std::stod` parsers, a **DX11-off link break**, a
  declared-but-undefined `sqlite::Row::Get`, public-header hygiene (no leaked impl deps
  / Win32 macros), EventBus shutdown-drain, Observable notify-after-destroy, and
  per-frame allocation trims (`DepthLadder`/`Table`).
- **Optional-module build resurrected.** IPC/network/config/SQLite had never compiled
  on Windows (an ODR duplicate, wrong CMake targets, `<winsock2.h>` ordering, a missing
  `bcrypt`); they now build + test under the new `windows-msvc-debug-modules` preset.
- **`Result<T>` → `std::expected` (4.0.0, breaking).** A semver-major modernization —
  `Err(ErrorCode::X)`, the monadic surface, throwing `value()` — adopted in
  `Database::Open` and `Store::Load*`.
- **Backend cross-platform hardening + CI runtime verification (4.1.x).** Fixed the
  **macOS-was-completely-broken** GL path (forward-compat core context + GLSL 150), the
  **non-compiling Web build** (`int*`→`double*`, a main-loop UAF), Vulkan partial-init
  leaks, SDL3 host-window/`SDL_Quit` ownership, factory half-null pairs, and non-Windows
  DPI (new `PlatformBackend::GetContentScale()`); added a headless `--frames` CI smoke
  that **proves the GLFW+OpenGL3 backend runs on Linux**.
- **CI made fully green.** `windows-werror` (red since 3.17.0 on a `NOMINMAX`
  collision) and `clang-tidy` (an older clang couldn't parse C++23 `<expected>` → pinned
  clang-tidy-19) both fixed.
- **Comprehensive reference docs.** 7 new header-verified docs (ARCHITECTURE, REACTIVE,
  LAYOUT, IM_API, DSL, THEMING, BACKENDS) + rewritten MODULES/GETTING_STARTED + a
  rebuilt docs hub.
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

- **Metal / WebGPU renderers are non-functional stubs** (they now fail cleanly
  rather than pretending to succeed); the **Emscripten** path compiles but is
  unverified end-to-end. Real implementations are Horizon 4.
- **Software-GL rendering crashes inside the Mesa driver.** `llvmpipe`/`softpipe`
  segfault *within* `libgallium` during `ImGui_ImplOpenGL3_RenderDrawData` — a driver
  bug, **not** the UniGUI backend (which brings the GL context up cleanly) and absent
  on hardware GL. Consequence: the Linux CI smoke verifies backend *bring-up* rather
  than a full software render.
- **No automated visual / screenshot regression testing yet** (needs a GPU-capable
  CI runner).
- **Coverage, wrapper-coverage, and clang-tidy gates are still advisory** — flip each
  to a hard gate once its baseline is confirmed stable.
- **~4,900 clang-tidy *style* warnings** remain (mostly `f`-suffix / brace nits that
  match the house style); they're tolerated (`WarningsAsErrors` is empty, so only real
  diagnostics fail the step). Curating the check set and driving the count down is
  optional future cleanup.
- Optional trading follow-ups: a `PriceTicker` marquee and in-cell mini sparkline/bar
  renderers (the latter needs a custom-draw cell hook in `DataTable`).

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
- ~~**P2 · S — Warnings-as-errors.**~~ **Done (GCC + MSVC).** The whole tree (library +
  tests + examples) builds warning-clean under **GCC `-Werror`** (`linux-werror`) **and
  MSVC `/W4 /WX`** (`windows-werror`), both **enforced on every push/PR** in
  `build.yml`. (The MSVC gate had silently regressed since 3.17.0 on a `NOMINMAX`
  macro collision until the 4.1.x backend work re-greened it — a reminder that an
  unwatched gate is no gate.) `-Wextra`'s `missing-field-initializers` stays suppressed
  (it conflicts with the clang-tidy `readability-redundant-member-init` policy).

### Horizon 2 — Complete the ImGui wrapper (im layer first) — ✅ COMPLETE

`unigui::im` reached **201 functions = 100% of ImGui's practical surface** (A1–A6 all
shipped). The per-phase record is kept below.

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

_**Landed (4.1.x) — cross-platform hardening + CI runtime verification.**_ The four
production backends were audited and hardened on every failure path; the
**macOS-was-completely-broken** GL path (forward-compat core context + GLSL 150) and
the **non-compiling Web build** were fixed; Vulkan partial-init leaks and SDL3
host-window/`SDL_Quit` ownership bugs were closed; the factory now honours its
`{nullptr,nullptr}` contract; and a headless `--frames` CI smoke **proves the
GLFW+OpenGL3 backend boots and renders on Linux** (the macOS GL code path is validated
by the same run). What remains is making the *stub* renderers real:

- **P1 · L — Implement the Metal renderer** (macOS), replacing the cleanly-failing
  stub; validate against the Vulkan/MoltenVK path.
- **P1 · L — Implement WebGPU + Emscripten** for a working browser target; ship a web
  demo of `widget_gallery`. _(Emscripten now **compiles**; the WebGPU/WebGL renderer is
  the remaining gap.)_
- ~~**P1 · M — DPI / multi-monitor robustness.**~~ **Partly done.** Non-Windows DPI now
  reads the platform window's content scale (`PlatformBackend::GetContentScale()` →
  `glfwGetWindowContentScale` / SDL), fixing blurry retina/HiDPI text on macOS/Linux.
  _Remaining:_ fractional-scaling polish and runtime DPI changes across all backends.
- **P2 · S — A GPU-capable CI runner** so the Linux/macOS smoke can verify a *full*
  render; today it falls back to asserting bring-up when the Mesa software GL driver
  crashes inside `RenderDrawData`.
- **P1 · M — Performance budget & benchmarks.** _In progress._ Trading-model
  benchmarks landed (`tests/trading/bench_test.cc`): `OrderBook` under 200k
  price deltas + 5k full-book snapshots, and `OhlcSeries` over 1M ticks +
  per-frame column extraction — each with a regression floor. A `DataTable`
  virtual-scroll benchmark at **100k rows** (`tests/bench/bench_test.cc`) proves
  steady-state per-frame cost stays bounded by the visible window (joining the
  existing `VirtualList`/CSV-import budgets). The row-vector **`Table` is now
  `ImGuiListClipper`-virtualized** with its own 100k-row steady-state benchmark,
  and all budgets run in the **Release CI jobs** (a tracked gate). _Remaining:_
  per-widget micro-budgets as the widget set grows.
- **P2 · M — GPU-side text/MSAA improvements** and a shared backend capability
  query so features degrade gracefully per renderer.

### Horizon 5 — Capability growth

Goal: broaden what apps can build without leaving the toolkit.

- **P0 · L — Framework transformation.** _Landed (all four phases)._ The toolkit
  is now an opinionated application framework. **(1)** the component model —
  `dsl::Component` + reactive `dsl::State<T>` with dirty-tracked rebuilds and
  `dsl::Host`/`dsl::Custom` composition (`dsl/component.h`); **(3)** the application
  layer (`dsl/app.h`) — `dsl::Store<T>`, `dsl::Navigator`, `Component::Watch` /
  `OnCleanup`; **(2 + 4)** the **golden-path guide** (`docs/FRAMEWORK.md`), a
  flagship reference app (`examples/framework_demo`), and the live
  **`dsl::DrawInspector()`** component/state inspector. _Next:_ deepen the idiom
  (forms/validation as components, a routing/URL story, devtools beyond the
  inspector) as real apps drive requirements.

- **P1 · L — Layout system.** _Started._ A header-only CSS-flexbox solver landed
  (`core/flex_layout.h`: `SolveFlex`) — pure and fully unit-tested. It now handles
  the **main axis** (grow/shrink, min/max clamps, `justify-content`, gaps), the
  **cross axis** (`align-items`), and **line wrapping** (`SolveFlexWrap`,
  CSS `flex-wrap`). A widget-facing **`Layout::FlexRow`** container applies it
  through ImGui child regions with `justify` + cross-axis `align` (hardened
  against the zero-width BeginChild trap and the manual-cursor bounds assertion),
  and the declarative DSL gained a **`dsl::Flex`** node rendered through it.
  _Remaining (optional):_ a wrapping container (FlexRow currently lays out a
  single line); per-child cross-size ergonomics in the DSL; nested-flex polish.
- **P1 · M — Accessibility.** Surface the existing `AccessibleName`/`Description`
  fields through a real a11y tree / screen-reader bridge where the platform
  allows; keyboard-only navigation audit.
- ~~**P1 · M — Theming authoring tools.**~~ **Done.** Theme export/import
  (`ExportThemeJSON`/`ImportThemeJSON`, round-trip tested), **CSS hot-reload from
  disk** (`styling::Engine::LoadFile()` + `ReloadIfChanged()`/`Clear()`/
  `WatchedFile()`), and a live **`examples/theme_editor`** that ties them
  together — switch preset/surface/font/accent live, export/import the palette as
  JSON, and `--css <file>` to hot-edit a stylesheet while it runs (headless via
  `--frames N`).
- ~~**P2 · L — Data binding / reactive layer.**~~ **Done.** Header-only
  `core/observable.h`: `Observable<T>` (change-detecting `Set`, `ForceSet`,
  in-place `Mutate`, `Subscribe`/`SubscribeAndFire`) with **RAII `Subscription`**
  handles that auto-unsubscribe and safely outlive the observable (shared registry
  + weak ref), a `Bind(source, sink)` helper, and **`Computed<T>`** —
  derived/recomputing observables that are N-ary, heterogeneous, composable
  (Computed-of-Computed), lifetime-safe via per-source value caching, and
  eventually-consistent. **First-class widget binding**:
  `ValueWidget<T>::BindValue` (two-way, across every value widget, with an
  `ApplyBoundValue` hook for buffer-backed inputs) and `Label::BindText`
  (one-way), both with a source-lifetime guard. **Trading models are
  reactive-ready** too — `Quote`/`Position`/`Order`/`Trade` carry value equality,
  so they drive `Observable`/`Computed`/`Bind` directly (live derived metrics).
  Fully unit-tested.
- **P2 · M — Internationalization.** _Mostly done._ `core/locale.h` is now a
  real catalog: a **fallback chain** (current → base language → fallback locale →
  key) so partially-translated locales degrade gracefully, **positional
  `{0}`/`{1}` argument substitution** (`Tr(key, args)`), and **RTL detection**
  (`IsRTL()` for ar/he/fa/ur) — all unit-tested. _Remaining:_ full RTL layout
  *mirroring* (a layout-engine concern, tracked with the Horizon-5 layout work).
- **P2 · M — Plugin ecosystem.** Stable plugin ABI, versioned plugin interface,
  sample third-party plugins, and a plugin template repo.

### Horizon 6 — Ecosystem & reach (long term)

Goal: make UniGUI easy to adopt and contribute to at scale.

- **P2 · M — Packaging.** _`find_package` hardening landed._ A standalone
  downstream consumer (`tests/packaging/consumer`) is built and **run against an
  install tree** by the `install-consume` CI job (and `scripts/test_install.ps1`
  locally), so a broken exported target, a missing `find_dependency()`, or an
  uninstalled generated header now fails CI instead of a downstream user's build.
  _Remaining:_ publish to a vcpkg registry and/or a Conan package; versioned
  binary releases.
- **P2 · M — Language bindings.** Explore C API + bindings (e.g. C#, Python) over
  a stable C ABI surface.
- **P2 · L — Designer / live-preview tool.** Standalone app that previews DSL/CSS
  and emits code.
- **P2 · S — Community.** Contribution ladder, "good first issue" curation,
  governance doc, public roadmap board mirroring this file.

## 4. Cross-cutting workstreams (continuous)

These run in parallel with every horizon:

- **Quality:** keep all CI green — Build & Test (Win/Linux/macOS + both `-Werror`/`/WX`
  gates + install-consume + the headless backend smoke) and Quality (`clang-tidy`
  pinned to 19, coverage). Grow coverage; expand fuzz/bench; keep `clang-tidy` free of
  real diagnostics (the ~4,900-warning advisory style backlog is separate). **Actually
  watch the gates:** two (`windows-werror`, `clang-tidy`) silently regressed for several
  releases while unobserved — an unwatched gate is no gate.
- **Cross-platform runtime verification:** the headless `--frames` smoke runs the
  GLFW+OpenGL3 backend on the Linux runner (xvfb + software GL) and asserts it boots and
  renders — turning "compiles" into "runs". The `windows-msvc-debug-modules` and
  `windows-msvc-debug-no-dx11` presets exercise the off-by-default module/backend code
  paths that the default build never touches.
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
- **Reach:** functional backends (**4/7** production today — GLFW+GL3 **CI-verified to
  run on Linux**, Vulkan, DX11, DX12; Metal/WebGPU stubs, Emscripten compiles; target
  7/7); platforms with a passing test suite (**Win/Linux/macOS all green**); packaging
  channels available.
- **Adoption:** examples that build on web; external plugins; downstream
  embedders.

## 7. How to use this document

- When picking up work, start from the **highest-priority item in the lowest open
  horizon** unless a release-blocking bug takes precedence. With **Horizons 2 & 3
  complete** and the audit + backend-hardening arc shipped, the open frontier is
  **Horizon 4** (a real **Metal** renderer, then **WebGPU/Emscripten** + a web demo) and
  **Horizon 5** (**accessibility**; deepening the framework idiom), plus the
  cross-cutting **visual-regression harness** and flipping the **coverage / clang-tidy**
  gates from advisory to hard once their baselines are pinned.
- When you complete an item, check it off here, add a line to `CHANGELOG.md`, and
  update any affected docs/badges in the same PR.
- Re-scope horizons at each release: promote, demote, or split items as reality
  dictates. Keep the vision (§1) stable; let the tactics move.
</content>
