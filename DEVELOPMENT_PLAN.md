# TeamkillerUniGUI — Long-Term Development Plan

_Last updated: 2026-06-30 · Current version: 4.4.4_

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
**cross-platform backends hardened and verified at runtime in CI**. **All seven backends
are now real and online** — the **Metal** renderer (4.2.0), the **Emscripten/WebGL2** web
backend (4.2.0), and the **WebGPU** web renderer (4.3.0, emdawnwebgpu) — with the wasm
builds CI-verified and shipped as downloadable `web_demo` artifacts (a tabbed widget
gallery). Browser verification of those artifacts also caught and fixed a latent
black-screen bug in the GLFW+OpenGL3/WebGL path (4.3.1), so the GL backend is now
confirmed rendering, not just compiling.

The 4.4.x line then closed two more fronts. **Accessibility landed across all four
platforms** (4.4.0–4.4.1): a per-frame a11y tree, ARIA-style live announcements, keyboard
nav, an inspector, ~39 widgets wired, and screen-reader bridges for Windows (UI Automation),
web (ARIA live regions), macOS (NSAccessibility), and Linux (AT-SPI) — with a
framebuffer-readback **render-verify CI gate** (`UNIGUI_RENDER_VERIFY`) that would have caught
the 4.3.1 black screen. And an **adversarial review→fix→verify loop** swept the code written
without a local runtime: the Metal/WebGPU/wasm/DX12 backends (4.4.2 — a per-frame WebGPU
texture leak, a Metal autorelease-pool stall, a DX12 bring-up leak) and the optional
sqlite/config/ipc/network modules (4.4.3–4.4.4 — 13 bugs incl. a zmq >64 KB stack overflow
and a throwing port parser), which also added a **`linux-modules` CI job** so the
previously-uncompiled module + POSIX-shmem code can't rot again. With backend **completeness**
and **accessibility** done, the frontier is now backend **performance**, a fuller
**visual-regression** harness (GPU-runner + headless-browser + golden-image), and
**packaging reach**.

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
- **UI preset scaffolds** (`unigui::presets`, 4.6.0): AppShell / SettingsPage /
  Dashboard / MasterDetail / LogConsole — prefab, themed, a11y-wired compositions so a
  decent app is ~30 lines (`docs/PRESETS.md`, `examples/preset_demo`).
- **Application framework** (`unigui::dsl`): `Component` + reactive `State<T>`,
  `Store<T>`, `Navigator`, `Watch`/`OnCleanup`, the live `DrawInspector()` overlay,
  and the `Custom` escape hatch — see `docs/FRAMEWORK.md`.
- **Reactive + layout cross-cutting layers**: `core/observable.h`
  (`Observable`/`Computed`/`Bind` + first-class widget binding) and
  `core/flex_layout.h` (`SolveFlex`/`SolveFlexWrap` + `Layout::FlexRow` + `dsl::Flex`).
- Theme engine: Dark/Light + 13 presets, unified style/color tokens, surface
  materials, semantic colors, elevation; theme export/import + CSS hot-reload + a live
  `theme_editor`.
- 7 backends, all functional (GLFW+GL3, GLFW/SDL3+Vulkan, DX11, DX12, **Metal**, the
  **Emscripten/WebGL2** web path, and **WebGPU** via emdawnwebgpu), all hardened on their
  failure paths. The default GLFW+OpenGL3 path is **CI-verified to run on Linux**
  (headless xvfb + llvmpipe smoke) and its identical GL code path covers macOS; both
  **wasm builds (WebGL2 + WebGPU) are CI-verified** (emsdk 4.0.10 + `emcmake`, with
  `web_demo` artifacts). Metal is build-verified on the macOS CI runner.
- **`Result<T>` is `std::expected<T, ErrorCode>`** (4.0): errors via `Err()`, monadic
  ops, throwing `value()`; adopted in `sqlite::Database::Open` and `config::Store::Load*`.
- Optional modules — SQLite, config (TOML/JSON/INI), IPC (shmem + ZMQ), network
  (HTTP/WebSocket) — build & test on Windows via the `windows-msvc-debug-modules` preset
  **and now in CI** via the `linux-modules` job (4.4.3), which compiles every module
  (incl. the POSIX shmem path that exists nowhere else) and runs their suites. An
  adversarial review fixed 13 latent bugs across them (4.4.3–4.4.4).
- **Accessibility** (`unigui::a11y`, 4.4.0–4.4.1): a per-frame element tree, ARIA-style
  live announcements, keyboard nav, an inspector, ~39 widgets wired, and screen-reader
  bridges for all four platforms — opt-in via `AppConfig::accessibility`.
- ~160 GoogleTest files (**1178** default-preset cases; ~1290 with all modules on),
  benchmarks (incl. an LTTB perf budget) and fuzz targets (CSV/JSON/CSS/config).
- CI (**all green**, 11 build jobs): cross-platform build/test (Win/Linux/macOS) + a
  **headless backend smoke that proves the GL path actually runs and draws pixels**
  (`render-verify`) + the **`linux-modules`** optional-module job + the **`linux-asan`**
  sanitizer job (ASan+UBSan+LSan, hard-gated) + the **`linux-testengine`** interaction-test
  job (the Dear ImGui test engine clicks/types through real widgets — behavior coverage,
  not render smoke) + the **`windows-dx12`** compile gate + the emscripten WebGL2/WebGPU
  wasm builds + warnings-as-errors on **both GCC and MSVC** + install-consume packaging +
  **clang-tidy** (pinned 19) + advisory coverage.

### Recently completed

- **Accessibility across all four platforms (4.4.0–4.4.1).** A real `unigui::a11y` layer:
  a per-frame element tree (`BeginFrame`/`AddNode`/`Tree`), ARIA-style live announcements
  (`Announce` + `Live` politeness), richer node state/roles, an in-app `DrawInspector()`,
  always-on keyboard nav (`NavEnableKeyboard`), and a real `InstallSystemBridge()` with a
  per-platform backend — **Windows** UI Automation (Narrator/NVDA/JAWS), **web** ARIA live
  regions, **macOS** NSAccessibility (VoiceOver), and **Linux** AT-SPI2 over the a11y D-Bus
  (opt-in `-DUNIGUI_A11Y_ATSPI=ON`). ~39 widgets report via `Widget::ReportAccessible`;
  one-flag opt-in (`AppConfig::accessibility`). Shipped with a **framebuffer-readback
  render-verify CI gate** (`UNIGUI_RENDER_VERIFY` → `drawn=true`) that catches the
  "renders nothing" class the 4.3.1 black screen slipped through. An adversarial review of
  the new a11y code then fixed 5 bugs (4.4.1) — incl. a malformed AT-SPI signal that made
  the Linux bridge a silent no-op, and a combobox OOB read.
- **Adversarial review→fix→verify of the blind-written backends (4.4.2).** The
  Metal/WebGPU/wasm/DX12 code written without a local runtime was swept the same way: found
  and fixed a **critical per-frame WebGPU `WGPUTexture` leak**, a **Metal autorelease-pool
  stall** (drawable-pool exhaustion on the manual loop), a WebGPU device-chain leak, and a
  **DX12 bring-up leak** — each fix then re-verified by an adversarial pass. (DX12 is built
  by no CI lane, so it's compile-checked locally with `-DUNIGUI_BACKEND_DX12=ON`.)
- **Optional-module hardening + CI coverage (4.4.3–4.4.4).** The same loop over the
  sqlite/config/ipc/network modules — built by **no CI job**, so 13 bugs had accumulated:
  a **critical zmq >64 KB stack-buffer overflow**, a **throwing `std::stoi` on HTTP port**,
  a sqlite rule-of-three double-free + open-failure leak, POSIX shmem `MAP_FAILED`/HANDLE
  leaks, a silent HTTPS→cleartext downgrade, and config save-corruption (`SaveTOML` losing
  the whole file on empty/non-canonical values). Added a **`linux-modules` CI job** (which
  immediately flushed out a latent `find_package(SQLite3)` Linux break), and a
  fix-verification pass caught one incomplete fix → 4.4.4.
- **All 7 backends online — Metal + WebGL2 + WebGPU (4.2.0–4.3.1).** The last three
  stub backends became real: a working **Metal** `imgui_impl_metal` renderer on a
  `CAMetalLayer` (4.2.0, build-verified on the macOS CI runner); the **Emscripten/WebGL2**
  web backend — the whole 40k-line library cross-compiles to WebAssembly via a FetchContent
  + Emscripten-ports CMake mode, rendering through GLES3/WebGL2 (4.2.0); and the **WebGPU**
  web renderer via emsdk 4.0.10 + `--use-port=emdawnwebgpu` with async device acquisition
  (4.3.0). CI build-verifies both wasm targets and uploads a tabbed **widget-gallery**
  `web_demo` artifact for each. Browser-testing the artifact then caught a latent
  black-screen bug — the GL backend never called `ImGui_ImplOpenGL3_NewFrame()`, so it
  built no shader/buffers — fixed in 4.3.1 (which also repaired the desktop GLFW+OpenGL3
  path).
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

- **All 7 backends are real** (Metal/WebGL2/WebGPU landed 4.2.0–4.3.1) — no stub
  remains. The wasm backends are build-verified in CI; their in-browser runtime is
  validated manually (CI has no GPU/browser), and WebGL2 is confirmed rendering as of
  4.3.1.
- **Visual / pixel-level CI — partially addressed.** The 4.3.1 black-screen bug (GL drew
  nothing) passed every CI signal because the Linux smoke grepped *log lines*, not
  *pixels*. The smoke now reads the framebuffer back and asserts the GL backend actually
  drew (`UNIGUI_RENDER_VERIFY` → `glError`/`drawn=true`), so that class is caught. Still
  missing: a GPU-capable runner for a software-free full render, a headless-browser smoke
  for the WebGL/WebGPU artifacts, and golden-image snapshot diffing — see Horizon 4.
- **Font manager probes a hardcoded Linux emoji path** (`/usr/share/fonts/.../
  NotoColorEmoji.ttf`) on every platform — a harmless warning off Linux, but it should
  be platform-aware; the web build has no system CJK/emoji fonts at all.
- **Software-GL rendering crashes inside the Mesa driver.** `llvmpipe`/`softpipe`
  segfault *within* `libgallium` during `ImGui_ImplOpenGL3_RenderDrawData` — a driver
  bug, **not** the UniGUI backend (which brings the GL context up cleanly) and absent
  on hardware GL. Consequence: the Linux CI smoke verifies backend *bring-up* rather
  than a full software render.
- ~~**DX12 backend has no CI lane.**~~ **Closed.** `UNIGUI_BACKEND_DX12` still defaults OFF,
  but the new **`windows-dx12`** CI job builds the library with DX12 ON (alongside DX11) on
  every push, so `dx12_renderer.cc` + the `UNIGUI_HAS_DX12` paths in `app.cc` are now
  compile-gated (a break fails CI). It's a compile gate — DX12 device creation needs a real
  GPU the headless runner lacks — mirroring how `linux-modules` covers the module + POSIX-shmem
  sources. **No CI-uncovered backend/module compile paths remain.**
- **Runtime backend coverage is still GL-only.** Six of seven renderers (DX11/DX12/Vulkan/
  SDL3/Metal/WebGPU) are now build-covered but have **no automated runtime render test**; only
  GLFW+OpenGL3 gets the Linux llvmpipe pixel-readback. Closing this needs a GPU-capable runner
  (or a WARP software adapter for DX) and a headless-browser smoke for the wasm artifacts.
- **Coverage and clang-tidy gates are still advisory** — flip each to a hard gate once its
  baseline is confirmed stable. (The Linux/macOS/werror **ctest** steps now hard-gate, so a
  broken non-GL test fails CI; the GL-context `BackendTest` self-skips headless and is
  excluded there, covered by the render smoke instead.)
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

Goal: ~~turn the stub backends into real ones~~ **(done — all 7 are real, 4.2.0–4.3.1)**
and make rendering measurably fast + regression-proof. The open work is now backend
**performance** and a **visual-regression** harness, not completeness.

_**Landed (4.1.x) — cross-platform hardening + CI runtime verification.**_ The four
production backends were audited and hardened on every failure path; the
**macOS-was-completely-broken** GL path (forward-compat core context + GLSL 150) and
the **non-compiling Web build** were fixed; Vulkan partial-init leaks and SDL3
host-window/`SDL_Quit` ownership bugs were closed; the factory now honours its
`{nullptr,nullptr}` contract; and a headless `--frames` CI smoke **proves the
GLFW+OpenGL3 backend boots and renders on Linux** (the macOS GL code path is validated
by the same run).

_**Landed (4.2.0–4.3.1) — every backend is now real and online.**_

- ~~**P1 · L — Implement the Metal renderer** (macOS).~~ **Done (4.2.0):** real
  `imgui_impl_metal` on a `CAMetalLayer`, build-verified on the macOS CI runner.
- ~~**P1 · L — Implement WebGPU + Emscripten** for a working browser target.~~ **Done
  (4.2.0 WebGL2, 4.3.0 WebGPU):** UniGUI cross-compiles + links to WebAssembly through
  both the WebGL2 (OpenGL3/GLES3) and WebGPU (emdawnwebgpu) renderers; CI builds a
  tabbed widget-gallery `web_demo` artifact for each. In-browser runtime is validated
  manually.
- ~~**P0 · S — Fix the GLFW+OpenGL3/WebGL black screen.**~~ **Done (4.3.1):** the GL
  renderer never called `ImGui_ImplOpenGL3_NewFrame()`, so it built no shader/buffers
  and drew nothing; added a `RendererBackend::NewFrame()` hook. Caught by browser-testing
  the `web_demo` artifact — it also fixed the desktop GLFW+OpenGL3 path.
- ~~**P1 · M — DPI / multi-monitor robustness.**~~ **Partly done.** Non-Windows DPI now
  reads the platform window's content scale (`PlatformBackend::GetContentScale()` →
  `glfwGetWindowContentScale` / SDL), fixing blurry retina/HiDPI text on macOS/Linux.
  _Remaining:_ fractional-scaling polish and runtime DPI changes across all backends.
- **P1 · M — Visual-regression / pixel-level CI.** _Started._ The 4.3.1 black-screen bug
  drew nothing yet passed every CI signal because the smoke greps *log lines*, not
  *pixels*. **✅ Landed:** `UNIGUI_RENDER_VERIFY=1` makes the app read the GL framebuffer
  back after `RenderDrawData` (post-frame `glGetError()` + a clear-vs-drawn pixel-grid
  count → `[render-verify] … drawn=true|false`), and the Linux smoke asserts `drawn=true`
  after a clean run — so the black-screen class now fails CI (verified: `glError=0x0
  nonClear=2100/3600 drawn=true`). _Remaining:_ a **GPU-capable CI runner** so the
  Linux/macOS smoke can verify a *full* render without the software-GL fallback (today it
  reverts to asserting bring-up when the Mesa driver crashes inside `RenderDrawData`), a
  **headless-browser smoke** for the WebGL/WebGPU artifacts, and **golden-image snapshot
  diffing** for regressions beyond "is the frame blank".
- **P3 · S — Platform-aware font fallback.** The font manager probes a hardcoded Linux
  emoji path (`/usr/share/fonts/.../NotoColorEmoji.ttf`) on every OS; make it
  platform-aware, and offer an opt-in CJK font merge for the web build (which has no
  system fonts).
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
- **P1 · M — Accessibility.** _Largely complete._ The `unigui::a11y` module provides a real
  **per-frame accessibility tree** (`BeginFrame`/`AddNode`/`Tree`) on top of the focus
  tracker, **ARIA-style live announcements** (`Announce` + `Live` politeness, callback or
  drain), richer `Node` state/roles, an in-app **`DrawInspector()`**, and a logging
  reference bridge plus a real **`InstallSystemBridge()`** with per-platform backends:
  **Windows** UI Automation (Narrator/NVDA/JAWS), **web** ARIA live regions (any browser
  screen reader), and **macOS** NSAccessibility (VoiceOver). The app loop enables **keyboard
  navigation** (`NavEnableKeyboard`), **`AppConfig::accessibility`** opts the whole layer in
  with one flag, and **~44 widgets** report into the tree via `Widget::ReportAccessible`
  (buttons, all inputs incl. password-presence-only, selections, sliders, combos,
  tabs/tree/table/collapsing-header, color/date pickers, and chrome). A native **Linux
  AT-SPI** bridge (AT-SPI2 `Announcement` events over the a11y D-Bus, opt-in via
  `-DUNIGUI_A11Y_ATSPI=ON`, compile-verified in CI) rounds out all four platforms. The whole
  layer was then **adversarially reviewed (4.4.1)**, fixing 5 bugs — incl. a malformed AT-SPI
  signal that made the Linux bridge a silent no-op, a combobox OOB read, and an unbounded
  announcement queue. **The data-dense widgets — previously the biggest hole — were wired in
  4.5.0**: DataTable (dimensions/filter/selection + per-visible-row registration + selection
  announcements), VirtualList, TreeView (per-node registration, selection-aware container),
  and DepthLadder (inside-market value, click-to-trade announcements), all hot-path-guarded
  by `a11y::IsEnabled()` and covered by headless tree/value tests.
  _Remaining:_ a focused keyboard-only nav audit and in-the-wild screen-reader runtime
  validation (Narrator/VoiceOver/Orca/browser SRs) of the bridges.
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
- **Reach:** functional backends (**7/7** today — GLFW+GL3 **CI-verified to run on
  Linux**, Vulkan, DX11, DX12, Metal, plus the **WebGL2 and WebGPU** wasm paths,
  CI-build-verified); platforms with a passing test suite (**Win/Linux/macOS all
  green**); packaging channels available.
- **Adoption:** examples that build on web; external plugins; downstream
  embedders.

## 7. How to use this document

- When picking up work, start from the **highest-priority item in the lowest open
  horizon** unless a release-blocking bug takes precedence. With **Horizons 2 & 3
  complete**, the audit + backend-hardening arc shipped, **Horizon 4's backend
  completeness done** (Metal + WebGL2 + WebGPU, 4.2.0–4.3.1 — all 7 backends real), and
  **Horizon 5's accessibility largely done** (4.4.0–4.4.1, all four platforms), the open
  frontier is **runtime backend verification** (a GPU-capable runner + a headless-browser
  smoke for the wasm artifacts — all 7 backends are now build-covered in CI, but only GL is
  runtime-verified), the rest of the **visual-regression harness** (golden-image diffing),
  **Horizon 4's performance work**, **deepening the Horizon-5 framework idiom**, and the small
  hardening items — flipping the **coverage / clang-tidy** gates from advisory to hard, the
  **keyboard-only nav audit**, and **platform-aware font fallback**. (Closed after the 4.4.5
  audit: the `|| true` test-gating hole, the DX12 CI-coverage gap, and the **ASan CI lane** —
  the `linux-asan` job now runs the whole suite under **ASan+UBSan+LeakSanitizer** on every
  push, hard-gated; its first run found only one third-party Xlib init leak, now documented
  in `tests/lsan.supp`. The suite is otherwise sanitizer-clean, locally on MSVC ASan and in
  CI on GCC.)
- When you complete an item, check it off here, add a line to `CHANGELOG.md`, and
  update any affected docs/badges in the same PR.
- Re-scope horizons at each release: promote, demote, or split items as reality
  dictates. Keep the vision (§1) stable; let the tactics move.
</content>
