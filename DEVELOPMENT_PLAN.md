# TeamkillerUniGUI — Long-Term Development Plan

_Last updated: 2026-08-14 · Current version: 4.9.0 (next phase planned → 4.10)_

This document lays out a long-horizon roadmap for the project. It is meant to be
a living document: revisit it each release, check off what shipped, and re-scope
what's next. It complements — but does not replace — `CHANGELOG.md` (what
happened) and `RELEASE.md` (per-release notes).

The project's two original headline goals are **achieved**:

1. ~~**Complete the wrapper.**~~ **Done (Horizon 2).** `unigui::im` now wraps **100%
   of ImGui's practical surface** (204 of 204 practical-surface targets, hard-gated
   at 95% in CI; **251 first-class `im::` functions** in total). Raw `ImGui::` stays
   fully supported and auto-themed.
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
previously-uncompiled module + POSIX-shmem code can't rot again.

The 4.5–4.6 line turned the audit's findings into structure. **Verification became
automatic**: every push now runs the suite under **ASan+UBSan+LeakSanitizer**
(`linux-asan`), the Linux/macOS test suites **hard-gate** (no more `|| true`), DX12 is
**compile-gated** (`windows-dx12`), doc version stamps are **CI-checked**
(`docs-version`), and the **Dear ImGui test engine** drives real clicks/typing through
widgets — 28 interaction-driven behavior tests on a reusable harness (`linux-testengine`).
**Accessibility reached the data-dense widgets** (4.5.0): DataTable, VirtualList,
TreeView, and DepthLadder now report and announce (~44 widgets wired). And the toolkit
gained its highest-leverage capability layer yet: **UI preset scaffolds** (4.6.0,
`unigui::presets`) — AppShell, SettingsPage, Dashboard, MasterDetail, LogConsole — so a
decent, themed, screen-reader-visible app is ~30 lines (`examples/preset_demo`). The
**headless-browser wasm smoke** then landed (post-4.6.0): every push now loads the
WebGL2 `web_demo` in headless Chromium and hard-gates on real rendered pixels — the web
went from "validated manually" to runtime-proven, taking runtime-verified backends to
**2 of 7**. With completeness, accessibility, and the verification net in place, the
frontier is now **preset/framework depth** (presets v2 — the adoption multiplier),
**runtime proof on the remaining backends** (WARP/GPU runners, golden images), **module
maturity** (ipc/network functional tests + API fixes), and **packaging reach**.

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

- **92 widgets** (100% PushID-safe), the `unigui::im` immediate layer (**251
  functions** — 204 of ImGui's 204 practical-surface targets, 100%, CI-gated),
  declarative DSL, CSS styling engine, EventBus, plugin system, font manager,
  and opt-in **multi-viewport** (drag windows out of the main window; 2026-08).
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
- **Accessibility** (`unigui::a11y`, 4.4.0–4.5.0): a per-frame element tree, ARIA-style
  live announcements, keyboard nav, an inspector, **~44 widgets wired — including the
  data-dense ones** (DataTable/VirtualList/TreeView/DepthLadder report dimensions/
  selection and announce interactions), and screen-reader bridges for all four
  platforms — opt-in via `AppConfig::accessibility`.
- ~165 GoogleTest files (**1300** default-preset cases on Windows, post client-suite
  merge; ~1300+ with all modules on) + **28 interaction-driven behavior tests** on the
  Dear ImGui test engine harness (`tests/interaction_harness.h`), benchmarks (LTTB/100k-row
  budgets, sanitizer-aware) and fuzz targets (CSV/JSON/CSS/config).
- CI (**all green**, 11 build jobs + 3 quality jobs): cross-platform build/test
  (Win/Linux/macOS, **hard-gated** — no `|| true`) + a **headless backend smoke that
  proves the GL path actually runs and draws pixels** (`render-verify`) + the
  **`linux-modules`** optional-module job + the **`linux-asan`** sanitizer job
  (ASan+UBSan+LSan, hard-gated) + the **`linux-testengine`** interaction-test job (the
  Dear ImGui test engine clicks/types through real widgets — behavior coverage, not
  render smoke) + the **`windows-dx12`** compile gate + the emscripten WebGL2/WebGPU
  wasm builds + warnings-as-errors on **both GCC and MSVC** + install-consume packaging +
  the **`docs-version`** stamp gate + **clang-tidy** (pinned 19) + advisory coverage.
  Every backend and module now **compiles in CI**; no silent test-swallowing remains.

### Recently completed

- **Client-suite merge (post-4.8.0, 2026-08-14).** The `feat/client-suite-20260814`
  branch (which had absorbed `feat/im-wrappers-20260728` and the filepath/chart-pad
  branches) merged into `master` — 12 commits, +1k lines, suite 1300/1300 green:
  - **Opt-in multi-viewport** (`AppConfig::multiViewport`): ImGui windows can be dragged
    out of the main window into real OS windows and merged back. Set *before* backend init
    (the backends read the flag at init to install their viewport interfaces) with a
    capability self-check that drops back to single-viewport instead of rendering blank;
    GL context save/restore delegated to new `PlatformBackend::SaveRenderContext`/
    `RestoreRenderContext` hooks; ignored on Emscripten. Fixed en route: **DX11's main RTV
    is now bound every frame** (a popped-out window used to hijack the main window's draw).
  - **`TimeSeriesChart` hardened for real trading clients**: span-relative auto-fit padding
    (the old value-relative padding flattened far-from-zero small-swing series),
    `SetYAxisRange` now applies-and-releases (the `ImPlotCond_Once` trap is gone),
    new `SetYAxisSpanLock(span)` (pin the Y *height*, keep panning — "固定纵轴" for a
    trader's yardstick) and `SetYAxisTickSpacing(step)` (explicit gridline step, guarded
    against label floods), and `SetPanEnabled`/`SetZoomEnabled` finally deprecated —
    they never gated anything.
  - **`im` layer grows to 248 functions**: tables, printf-style text, char-buffer inputs,
    the style/ID stacks, clipboard, viewport/context/font accessors. Fixed en route:
    `InputText`/`InputTextWithHint` now persist typing under `EnterReturnsTrue`
    (the write-back was keyed on a return value that only fires on Enter — a password box
    you could not type into).
  - **`FilePath` dialog now round-trips non-ASCII paths on Windows** (wide `W` APIs instead
    of the ANSI ones — GBK-mangled UTF-8 no more).
  - Merge-resolution notes: the regression test for the InputText fix carried two latent
    bugs (an ImGui `IM_ASSERT(buf != NULL)` trip and a missing post-focus idle frame) that
    only a Debug run exposes — both fixed; `docs/IM_API.md`/`BACKENDS.md`/`WIDGET_API.md`
    and the CHANGELOG were brought up to date with the new surface; the coverage script's
    parser now recognises the new wrapper return types.

- **UI preset scaffolds (4.6.0).** `UNIGUI_MODULE_PRESETS` / `unigui::presets`: AppShell
  (full app chrome), SettingsPage (schema-driven rows via getter/setter pairs), Dashboard
  (responsive metric/card grid), MasterDetail (splitter browser), LogConsole (ring-buffered,
  filterable). Themed, a11y-wired, PushID-safe, decent with zero config; 47 headless tests;
  builds with the module OFF; `examples/preset_demo` = a four-page app in ~60 lines.
  Drafted by five parallel implementers against the real widget APIs, green across all CI
  lanes (incl. both werror gates and ASan) on first contact. `docs/PRESETS.md`.
- **Dear ImGui test engine + 28 interaction-driven tests (4.6.0).** The 0-byte
  `integration_test.cc` became a real harness: the engine clicks/types/navigates actual
  widgets through ImGui's input queue — selection/input widgets, data widgets (incl.
  a11y-announcement round-trips), the dsl framework (Component/State rebuild, Store,
  Navigator), and keyboard-only navigation. Dev-only vcpkg feature (`imgui[test-engine]`,
  never pulled by consumers), `windows-msvc-debug-testengine` preset, hard-gated
  `linux-testengine` lane, `ports/imgui` overlay for the retagged upstream tarball.
  Found+fixed en route: DragFloat's unbounded-clamp-to-zero bug.
- **Data-widget accessibility + sanitizer lane (4.5.0).** DataTable/VirtualList/TreeView/
  DepthLadder now report into the a11y tree (dimensions/filter/selection values,
  per-visible-row registration) and announce interactions, hot-path-guarded by
  `a11y::IsEnabled()`. The `linux-asan` lane runs the whole suite under
  ASan+UBSan+LeakSanitizer on every push — the suite proved sanitizer-clean on both
  MSVC and GCC toolchains (one documented third-party Xlib suppression); bench budgets
  now skip under sanitizers so the lane fails on memory bugs, not timing noise.
- **Verification-gap closures + doc integrity (post-4.4.5 audit).** Hard-gated the
  Linux/macOS/werror ctest steps (dropped `|| true`), added the `windows-dx12` compile
  lane (DX12 previously compiled NOWHERE in CI), fixed the audit's two concrete bugs
  (banned `std::stod` in datatable.h; form-validator regex ReDoS hardening — 4.4.5),
  re-stamped 11 stale docs (one frozen at 3.8.12), fully resynced `README_zh.md`, and
  added the `docs-version` CI gate so stamp drift now fails CI in minutes.
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

_Closed since the 4.4.5 full-project audit (kept here as one line for the record): the
`|| true` test-swallowing, the DX12/module CI-compile gaps, the ASan-presets-unused gap,
the 0-byte integration test, doc-stamp drift (now CI-gated), the data-widget a11y hole,
the banned-parser and regex-ReDoS findings, and the DragFloat unbounded-clamp bug._

- **Runtime backend coverage: 4 of 7.** GLFW+OpenGL3 (Linux llvmpipe pixel-readback), the
  wasm/WebGL2 path (headless-browser smoke: Playwright + SwiftShader, canvas-only pixel
  verdict, hard-gated), and now **DX11 + DX12** (WARP software rasteriser: real device
  create → offscreen render → GPU pixel readback, no GPU, in the `windows` / `windows-dx12`
  lanes — post-4.6.0) are runtime-verified on every push. The remaining three
  (Vulkan/SDL3/Metal) stay build-only, and WebGPU's browser runtime stays a best-effort
  step (blank under the runner's SwiftShader — flips to a hard gate when the runner grows
  an adapter). The 4.4.2 leak batch proves this class of bug is real. Next: a GPU-capable
  runner for the rest + **golden-image diffing** on top, and a **windowed/swapchain** WARP
  pass to extend the DX smoke from offscreen render to actual present.
- **Multi-viewport is runtime-verified on GL + DX11 only** (2026-08). The opt-in flag
  shipped with the GL context save/restore and the per-frame DX11 RTV rebind proven on the
  two runtime-checked backends; the DX12/Vulkan/SDL3/Metal viewport paths, per-viewport
  DPI/focus routing, and the backdrop-clear contract for secondary viewports (§7 of
  `docs/BACKENDS.md`) have no runtime proof or tests yet. A WARP/headless multi-viewport
  smoke (enable the flag, pop a window out, assert pixels) would close most of it.
- ~~**ipc/network are safe but functionally unverified.**~~ **Resolved (c0cc2c5 + b22015a,
  post-4.6.0).** Loopback functional tests now cover all three paths — ZMQ pub/sub
  round-trip (`inproc://`), HTTP GET/POST against an in-process httplib server, and a
  WebSocket echo — and run headless in the `linux-modules` CI lane (55/55 green). The dead
  `Server::OnReceive` PUB-socket API now warns + is documented as a no-op; `ipc::Shutdown()`
  terminates the static zmq context (re-creatable for reuse); `LoadINI` handles comments,
  `[sections]` (dotted keys), and whitespace trimming. ipc/network are now at the bar the
  rest of the library sets.
- **Interaction coverage is young.** 28 behavior tests exist on a proven harness, but
  ~19% of the suite is still smoke-only and the presets themselves have no driven-input
  tests yet. The harness makes growth mechanical.
- ~~**Fluent `With*` API is on ~12% of widgets.**~~ **Resolved (post-4.6.0).** The rollout
  swept the whole retained layer: all 63 remaining direct-`Widget` classes (incl. the
  `DataTable<T>`/`BasketTicket<T>` templates) now derive `FluentWidget<T>`, +250 `With*`
  helpers landed for existing `Set*` config, and the CascadingCombo-style mid-chain
  type-degradation is gone. `tests/widgets/fluent_rollout_test.cc` pins it with a
  compile-time `static_assert` per class (base chainers must return `X&`).
- **clang-tidy is now a hard gate on `bugprone-*`** (post-4.6.0): `.clang-tidy` sets
  `WarningsAsErrors: 'bugprone-*'` and the CI job dropped `continue-on-error`, so a new
  bugprone finding fails CI. The pre-existing bugprone findings were cleared first (four
  `(int)(x+0.5f)`→`std::lround`, two `Form::Deserialize` inc-in-condition lifts, `MasterDetail`
  optional guards) with two sub-checks excluded by rationale (branch-clone false positive,
  crtp-accessibility stylistic). The **wrapper-coverage** metric is also gated now
  (`--threshold 95`, current **100%**). **Still advisory:** ~4,900 style warnings across the
  other tidy families (deliberate lowercase-suffix / brace-less-statement deviations — not
  bugs) and the lcov line-coverage job. Promote another tidy family into the gate as the
  tree is cleaned under it.
- **No multi-context story.** **9 `::Instance()` singletons** (inventoried 2026-08) assume
  one UI per process — fine today, a wall for embedding two independent UniGUI surfaces
  or parallel test isolation:
  | Singleton | Module | Per-context candidacy |
  |-----------|--------|-----------------------|
  | `config::Store` | config (optional) | app-level config cache — per-app instance |
  | `Settings` (core) | core | persistence/settings cache — per-app instance |
  | `events::Bus` | events (optional) | process-wide pub/sub — per-app or process singleton |
  | `fonts::Manager` | fonts | atlas/registry state — **per-context** (ImGui context owns fonts) |
  | `fx::AnimationManager` | fx | animation clock — per-context |
  | `plugin::Manager` | plugin (optional) | plugin registry — per-app |
  | `styling::Engine` | styling | CSS rules + hot-reload — per-context |
  | `theme::ThemeRegistry` | theme | preset registry — read-only catalog, could be static |
  | `Toast` | widgets | transient notifications — per-context |
  The likely shape of the fix: a `UIContext`/`InstanceRegistry` keyed by ImGui context,
  with the optional-module singletons moving behind it and the truly global ones
  (plugin registry) staying process-wide — design input only; the refactor itself
  remains deferred (P2·L).
- ~~Keyboard-only nav audit~~ **done** (86 widgets audited, 11 gaps fixed, driven
  keyboard tests); **in-the-wild screen-reader validation** (Narrator/VoiceOver/Orca)
  remains the last a11y item — it needs a human at a real screen reader.
- ~~Font manager hardcoded-path probe~~ **done** (per-distro candidates + `LoadSystemCJK`;
  the web build intentionally skips system fonts — load one explicitly there).
- **Software-GL renders crash inside Mesa** (`llvmpipe`/`softpipe` segfault in
  `libgallium` during `RenderDrawData` — a driver bug, absent on hardware GL), so the
  Linux smoke verifies bring-up + pixel-readback on the frames that survive.
- **`docs/WIDGET_API.md` depth is uneven** (16 deep sections vs 92 widgets; the one-line
  catalog lives in WIDGET_EXAMPLES.md).
- Optional trading follow-ups: a `PriceTicker` marquee and in-cell mini sparkline/bar
  renderers (needs a custom-draw cell hook in `DataTable` — now partially available via
  `SetCellRenderer`).

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
  nonClear=2100/3600 drawn=true`). _Remaining, cheapest first:_ ~~**(a) P1 · S — a
  headless-browser smoke** for the wasm artifacts~~ **Done.** `scripts/web_smoke.mjs`
  (Playwright + SwiftShader ANGLE) loads the artifact, screenshots the **canvas element**,
  and asserts real pixels in the emscripten CI job — WebGL2 hard-gated, WebGPU
  best-effort until the runner provides an adapter. Building it caught two verdict traps
  (page-chrome false-pass; headless GL virtualization black-screening a healthy
  artifact — fixed with canvas-only + swiftshader). **(b) P2 · M — a WARP
  software-adapter run** for DX11/DX12 on the Windows runner (real device creation +
  render without a GPU); **(c) P2 · L — a GPU-capable runner** for full renders on the
  rest, with **golden-image snapshot diffing** on top for regressions beyond "is the
  frame blank".
- ~~**P3 · S — Platform-aware font fallback.**~~ **Done.** Per-platform candidate lists
  (Debian/Arch/Fedora/BSD Noto locations; `%WINDIR%` on Windows) + a new
  `LoadSystemCJK()` glyph-fallback merge (YaHei/PingFang/Noto-CJK/WenQuanYi; CJK
  ideographs, kana, Hangul, punctuation, full-width forms). The web build documents
  the skip (no system fonts on MEMFS — load a font explicitly). Fixed en route:
  `Manager::Unload` double-free, empty-atlas MergeMode assert.
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
- **P1 · M — UI presets v2.** _v1 landed (4.6.0: AppShell / SettingsPage / Dashboard /
  MasterDetail / LogConsole — see above)._ Grow the preset layer where it multiplies
  adoption: **LoginPage/ConnectPage** (credentials + status + retry), a **WizardFlow**
  scaffold, **CommandPalette integration in AppShell** (Ctrl+P out of the box),
  preset-level theming knobs (accent/density), driven-input interaction tests for the
  presets themselves, and a README **screenshot** of `preset_demo` so the layer sells
  itself visually. Let real usage pick the next scaffolds.
- ~~**P2 · M — Module maturity (ipc/network).**~~ **Done (c0cc2c5 + b22015a).** The
  4.4.3–4.4.4 hardening made them safe; this made them *dependable*. Loopback functional
  tests landed for all three transports — ZMQ pub/sub round-trip over `inproc://`, HTTP
  GET/POST against an in-process httplib server, and a WebSocket echo — all headless-safe
  and gating the `linux-modules` CI lane (55/55). The dead `Server::OnReceive` PUB API now
  warns and is documented as a no-op with the Channel topology spelled out; `ipc::Shutdown()`
  terminates the process-wide zmq context (transparently re-created for post-shutdown reuse,
  with the channel-reuse constraint documented in the public header); and `LoadINI` now skips
  `;`/`#` comments, maps `[section]` headers to dotted keys, and trims key/value whitespace.
  An adversarial review→verify pass over the diff confirmed 0 defects.
- ~~**P2 · M — Fluent `With*` rollout.**~~ **Done (post-4.6.0).** Swept the retained layer
  in one pass: 63 direct-`Widget` classes → `FluentWidget<T>` (incl. the `DataTable<T>`/
  `BasketTicket<T>` templates via CRTP-on-template), +250 `With*` helpers for existing
  `Set*` config, and the CascadingCombo-style `Widget&` mid-chain break fixed layer-wide.
  Verified by `tests/widgets/fluent_rollout_test.cc`: a compile-time `static_assert` per
  class (base chainer must return `X&` — fails on any plain-`Widget` regression) plus
  runtime chain tests; full suite 1268/1268 green.

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
- **P2 · M — Language bindings.** _First increment landed._ A stable C ABI
  (`unigui_capi.h`, ABI v1) now covers the version/ABI gate, app lifecycle,
  HiDPI scale, and an immediate-mode drawing subset — pure C99, tested from a
  C TU and through the test engine (see §7). _Remaining:_ C#/Python/Go
  bindings over the surface and more `im` calls as demand appears.
- **P2 · L — Designer / live-preview tool.** _First increment landed (post-4.9)._
  `examples/designer` previews built-in DSL scenes live (stateful controls stay
  interactive), hot-reloads CSS on top (`--css`), and emits the scene's builder
  expression via the new `dsl::ToSource` — copy-to-clipboard, one click. Ten
  codegen tests pin the emission (structure/indentation, literals, variants,
  compilable callback placeholders); the app runs headless (`--frames N`).
  _Remaining:_ editing DSL scenes in-app (a text DSL + parser, or drag-drop),
  and richer code emission (state + callbacks).
- **P2 · S — Community.** Contribution ladder, "good first issue" curation,
  governance doc, public roadmap board mirroring this file.

## 4. Cross-cutting workstreams (continuous)

These run in parallel with every horizon:

- **Quality:** keep all CI green — Build & Test (11 lanes: Win/Linux/macOS + both
  `-Werror`/`/WX` gates + install-consume + the headless backend smoke + `linux-asan` +
  `linux-modules` + `linux-testengine` + `windows-dx12`) and Quality (`clang-tidy`
  pinned to 19, coverage, `docs-version`). **Grow interaction coverage** on the test-engine
  harness — every new widget/preset should get a driven-input behavior test, and the ~19%
  smoke-only tail converts opportunistically; expand fuzz/bench; keep `clang-tidy` free of
  real diagnostics (the ~4,900-warning advisory style backlog is separate). **Actually
  watch the gates:** two (`windows-werror`, `clang-tidy`) silently regressed for several
  releases while unobserved — an unwatched gate is no gate.
- **Cross-platform runtime verification:** the headless `--frames` smoke runs the
  GLFW+OpenGL3 backend on the Linux runner (xvfb + software GL) and asserts it boots and
  renders — turning "compiles" into "runs". The `windows-msvc-debug-modules` and
  `windows-msvc-debug-no-dx11` presets exercise the off-by-default module/backend code
  paths that the default build never touches.
- **Wrapper-coverage tracking:** `scripts/coverage_vs_imgui.py` (_landed_)
  reports the first-class-wrapped % of the ImGui practical surface each CI run,
  now enforced as a **hard `--threshold 95` gate** in `quality.yml` (currently
  **100% — the full 204-function practical surface**; headroom for a deliberate
  vcpkg imgui bump comes from the curated exclusion list, while a real regression
  fails CI).
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
  `v2` duplicate code paths remaining (target met: 0); **sanitizer-clean** under
  ASan+UBSan+LSan (currently: yes, one documented third-party suppression).
- **Quality:** test count (**1300** default preset on Windows / ~1300+ all-modules) & coverage %;
  **interaction-driven tests** (28 today — should grow with every widget/preset);
  smoke-only share of the suite (~19% — should shrink); fuzz targets; open P0/P1 bug count.
- **Ease of adoption:** lines of code for a decent app (**~30–60** via `unigui::presets`
  today — keep it there as presets grow); a11y-wired widgets (**~44**, target: every
  interactive widget).
- **Performance:** frame time for `DataTable`/`VirtualList`/DOM at 100k rows and
  high update rates; parser throughput (CSV/JSON).
- **Reach:** functional backends (**7/7** real; **all compile in CI**; runtime-verified:
  **2/7** — the GL path + wasm/WebGL2 in a real headless browser; target: WARP for DX
  next, then a GPU runner); platforms with a passing test suite (**Win/Linux/macOS all
  green**); packaging channels available.
- **Adoption:** examples that build on web; external plugins; downstream
  embedders.

## 7. How to use this document

- When picking up work, start from the **highest-priority item in the lowest open
  horizon** unless a release-blocking bug takes precedence. With **Horizons 2 & 3
  complete**, all 7 backends **real and compile-gated in CI**, **accessibility done
  through the data-dense widgets** (4.4.0–4.5.0), the **verification net installed**
  (sanitizers, hard gates, interaction tests, doc-stamp gate — 4.5.0–4.6.0 + the audit
  closures), the **UI preset layer shipped** (4.6.0–4.8.0), the **web runtime proven in a
  real browser on every push** (H4a — the wasm smoke), and the **client-suite merged**
  (multi-viewport + chart hardening + 47 new `im::` wrappers, 2026-08-14), the
  client-suite hardening phase shipped as 4.9.0 and the next phase is planned below
  (the completed phase is kept first as the record).

### Next phase — "client-suite hardening" (2026-08 → 4.9.0)

The merged client-suite work opened two fresh surfaces that deserve proof before
anything new: **multi-viewport** (landed with GL/DX11 runtime evidence only) and the
**chart-family APIs** (landed with headless tests, driven-input coverage pending). The
phase closes by cutting **4.9.0** from the merged work. Recommended order:

1. ~~**P0 · M — Prove multi-viewport on the runtime-verified backends.**~~ **Done
   (4.9.0).** `DXMultiViewportSmoke` runs the real app on a DX11 swapchain (WARP or
   hardware) with `UNIGUI_RENDER_VERIFY=1`: pop-out → main window still drawn (pixel
   readback, verified to fail against the pre-rebind-fix code) → stability → merge-back
   → no-flap. The backdrop-clear contract for secondary viewports was implemented (theme
   backdrop painted into each secondary viewport's background draw list) — and proving it
   found a real leak: the fill initially kept orphaned viewports alive forever, now fixed.
   The DX11 renderer gained a swapchain readback (verify parity with GL). _Remaining tail:
   Vulkan/SDL3/Metal viewport paths stay build-only; a capability-matrix section now
   documents exactly that in `docs/BACKENDS.md` §7.1._
2. ~~**P0 · S — Cut release 4.9.0.**~~ **Done.** CHANGELOG 4.9.0 (multi-viewport +
   SpanLock/TickSpacing + range semantics + InputText persistence + FilePath wide-char +
   deprecations + im wrappers + the hardening itself); `core/version.h` + `vcpkg.json`
   bumped together; docs re-stamped; README badges (tests 1305, im 248) updated.
3. ~~**P1 · M — Chart family follow-ups (client-driven).**~~ **Done (post-4.9.0).**
   `SetYAxisSpanLock`/`SetYAxisTickSpacing` have driven frame tests (injected
   wheel-zoom → one-frame bounce back to the locked span, centre kept; pure pan passes
   through), plus `GetYAxisRange()` for observability; the X-axis counterpart
   `SetXAxisTickSpacing` landed (visible-window-keyed ticks, `MakeTicks` pure math
   tested); the **in-cell mini sparkline/bar renderers** landed
   (`trading/cell_renderers.h` — `SparklineCell`/`BarCell` over `SetCellRenderer`,
   geometry-tested). `PriceTicker` turned out to already be implemented (the deferred
   note predated it). _Remaining: session-boundary-aware tick pairing for intraday
   charts — ticks land on session starts, not just round numbers._
4. ~~**P1 · S — Interaction coverage for the new surface.**~~ **Done (post-4.9.0).**
   Engine-driven: sortable table header → `TableGetSortSpecs`; `EnterReturnsTrue` typing
   persistence (interaction twin of the headless regression); and the **presets' input
   path** — `MasterDetail` row click → `WithOnSelect`, `Dashboard` card button →
   callback, `LogConsole` filter input → `FilteredSize()` (all five scaffolds now have
   driven-input coverage).
5. **P1 · M — Quality-front small items.** Promote the next clang-tidy family into the
   hard gate as its backlog clears; raise `COVERAGE_FLOOR` to just under the headless
   baseline and flip the coverage step to hard `exit 1`; keep converting the ~19%
   smoke-only tail opportunistically.
6. ~~**P2 · L — Backend runtime proof.**~~ **Windowed/swapchain WARP pass done (post-4.9).**
   `UNIGUI_DX11_WARP=1` forces the software rasterizer in the DX11 device creation, so a
   GPU-less runner gets a REAL device + swapchain + present; `DXMultiViewportSmoke.
   WarpAdapter_RendersWithoutGPU` proves the adapter really is the Microsoft Basic Render
   Driver and that real pixels land through it. The app-level smokes can now hard-gate on
   headless Windows CI instead of skipping. _Remaining: the GPU-capable runner for
   Vulkan/SDL3/Metal, golden-image diffing wired into a CI lane (the tooling exists —
   see the DPI & visual-proof phase), and per-monitor scale inheritance._
7. ~~**P2 · L — Multi-context.**~~ **First increment done (post-4.9).** The wall is
   cracked, not demolished: a `detail::ContextRegistry<T>` (LRU-capped per-ImGui-context
   instance map) now backs four singletons — `fonts::Manager`, `fx::AnimationManager`,
   `styling::Engine`, `Toast` (via an overridable factory, keeping its "_toast" widget
   name) — with public default constructors documented for the registry, the app loop
   resetting per-context instances on Shutdown, and four isolation tests (per-context
   instances, stable identity, no-context default fallback). The remaining five
   (`config::Store`, `Settings`, `events::Bus`, `plugin::Manager`, `ThemeRegistry`) are
   per-app/process by design and stay function-local statics until a real
   embed-two-surfaces consumer asks for them.
8. ~~**P2 — Long-horizon backlog (unchanged):**~~ **Two items advanced (post-4.9).**
   **Plugin-ABI stabilisation landed**: `kPluginInterfaceVersion` + a mandatory
   `PluginInterfaceVersion()` export — the manager rejects a version mismatch (or a
   missing export, i.e. a pre-versioning plugin) before instantiation, with the ABI
   policy documented (frozen within a version; additive-end-only growth; bump on any
   break). The example plugin exports the version; tests pin the compatibility gate.
   **vcpkg-registry packaging prepared**: `registry/` skeleton (ports/unigui +
   baseline + git-tree version db) and `scripts/packaging/prepare_vcpkg_registry.ps1`
   (assembles the registry repo at release time, generates the version db, prints the
   consumer snippet) — validated end-to-end: a `vcpkg install` resolving unigui through
   the generated registry builds and installs the package (vcpkg 2026-03-04). The
   source-tarball SHA512 is left as the accepted zero placeholder with pinning
   instructions; publishing waits for a release tag. **C ABI bindings — first
   increment landed**: `unigui_capi.h` + `src/capi/` expose a versioned C99 surface
   (ABI gate, version query, app lifecycle with an opaque handle + C frame callback,
   content scale, native window handle, and an immediate-mode drawing subset:
   begin/end, text, button, checkbox, slider, separator) with the growth policy
   documented in docs/C_API.md (additive-only ABI-version bumps; frozen layouts).
   A pure-C TU (`tests/capi/capi_c_test.c`, compiled as C — the project now enables
   the C language) proves the header really is C; six headless tests pin the ABI
   gate/version/config contracts; five test-engine tests click through the C boundary
   (button/checkbox/slider via `//**/` window-crossing paths); and a real DX11/WARP
   lifecycle test creates, draws from a C callback, runs a capped frame loop, and
   destroys — all through the ABI. **Designer tool — first increment landed**:
   `examples/designer` live-previews built-in DSL scenes (stateful controls stay
   interactive), hot-reloads CSS on top, and emits the scene's C++ builder
   expression via the new `dsl::ToSource` (structure/labels/params round-trip;
   callbacks become compilable placeholders — ten codegen tests pin it; the app
   smoke-runs headless with `--frames`). _Remaining: RTL layout mirroring,
   in-app scene editing, more `im` calls / bindings on demand, fractional-DPI
   cross-monitor polish (needs a multi-monitor runner), and in-the-wild
   screen-reader validation (human).

### Next phase — "completeness sweep" (post-4.9 → 4.10)

The post-4.9 tree is green across all three suites (1316 / 1356 / 1432) and both
remotes carry the work. The next batch closes the last measurable completeness gaps
rather than opening new surfaces. Recommended order:

1. ~~**P0 · S — `unigui::im` to 100% of the practical surface.**~~ **Done (post-4.9).**
   `GetFontBaked`, `GetItemFlags` and `TreeNodeGetOpen` are now first-class `im::`
   calls (204 of 204 targets — the coverage script reads **100.0%**, and its parser
   learned the last two return types). Three headless tests pin them; README/README_zh
   and this file now quote the full-coverage figure (251 first-class functions).
2. ~~**P1 · M — Session-boundary X ticks** (the chart-family tail).~~ **Done (post-4.9).**
   `SetXAxisSessionTicks(bool)` + the pure `MakeSessionTicks(axis, lo, hi, step, maxTicks)`:
   the explicit X grid gains every session-boundary anchor (span start/end), so intraday
   labels land on session edges even when the step doesn't divide the span; a collapsed
   lunch boundary (11:30/13:00 sharing one axis coordinate) yields a single tick. Four
   pure tests (boundary inclusion, shared-boundary dedup, window clipping, budget guard)
   + a frame smoke; the first-frame ±1e300 placeholder window is budget-guarded before
   any allocation. **Closes the chart-family tail.**
3. ~~**P1 · M — Trading interaction tests.**~~ **Done (post-4.9).** New
   `windows-msvc-debug-testengine-modules` preset (engine + trading together) and three
   engine-driven tests: `OrderTicket` valid draft → submit click → `OnSubmit` with the
   draft; invalid price → the disabled submit button fires nothing; `DepthLadder` level
   click → `OnLevelClick(side, price, size)`. _CandlestickChart stays engine-free: ImPlot
   crashes under the engine's per-frame state manipulation (yield assert + access
   violation) — documented in the test file; headless frame tests remain its coverage._
4. ~~**P1 · S — Count audit + WIDGET_API depth pass.**~~ **Done (post-4.9).** The
   historical "95 widgets" did not reproduce from any clean rule (the only match counted
   20 helper classes + 2 model classes as widgets). The audited count is **92** = 86
   `.cc`-backed widgets + 3 trading + 3 header-only (`DataTable<T>`, `ConnectionStatusBar`,
   `DockSpace`) — badges and docs now quote 92 with the derivation rule written into
   `docs/API_INDEX.md` and `docs/WIDGET_API.md` so the number stays maintainable. The
   `im` count is 251 (100% of the practical surface, see item 1).
5. ~~**P2 · M — Quality gate measurement.**~~ **Measured (post-4.9).** The local
   `windows-clang-tidy` build does not reproduce CI: the local toolchain differs from the
   pinned Linux clang-tidy-19 (fno-exceptions mismatch on `try` in main_thread/eventbus/
   window + a `bugprone-unchecked-string-to-number-conversion` in window.cc that CI's
   pinned version does not flag). Per-family promotion must therefore be based on the CI
   lane's numbers, not local ones — deferred until the CI baseline can be read; the
   wrapper-coverage gate is already at 100% (see item 1).
6. **P2 — Carry-over (unchanged):** backend runtime proof (GPU runner + golden
   images), multi-context singletons, and the long-horizon backlog from item 8 above.

### Next phase — "DPI & visual-proof" (post-4.9, → 4.10)

The completeness sweep closed every measurable gap it could reach locally. The next
batch attacks the two standing client-facing pain points that surfaced during it —
**DPI** (the multi-viewport smoke had to pin DPI to 1.0 because fractional monitor
scaling mixes physical and logical coordinates) and **visual regression proof** (the
pixel readback exists but nothing persists it into goldens). Recommended order:

1. ~~**P1 · M — Runtime / fractional DPI.**~~ **Done (post-4.9).** Root cause found and
   fixed: the GLFW platform never reported `io.DisplayFramebufferScale`, so the back
   buffer was rasterized at the wrong physical size at any non-1.0 DPI (and the
   multi-viewport smoke had to pin DPI to 1.0). The platform now reports it at bring-up
   and on change, polls the content scale every `NewFrame` (one GLFW call) and fires a
   new `PlatformBackend::SetContentScaleCallback` on change; the app handler snaps via
   `dpi::NormalizeContentScale` and updates `FontScaleDpi` (dynamic font re-raster).
   Tests: two platform tests (framebuffer-scale wiring, steady-scale no-fire) and the
   multi-viewport smoke now runs at the monitor's REAL scale (150% locally) and passes —
   proving the pop-out coordinate math at fractional DPI. _Remaining tail: per-monitor
   scale inheritance across monitors needs a multi-monitor runner (CI)._
2. ~~**P1 · M — Golden-image infrastructure.**~~ **Done (post-4.9).** The C++ side writes
   the rendered back buffer as dependency-free RAW RGBA when `UNIGUI_GOLDEN_CAPTURE=<path>`
   is set (shared `src/detail/golden_capture.h`, wired into the DX11 renderer and the GL
   path); `scripts/golden.py` owns the rest with stdlib-zlib only: `raw2png` (minimal
   PNG codec), `capture` (run an example → PNG), `diff` (per-channel threshold + changed-
   region summary + exit code). Roundtrip verified: two captures diff to 0 pixels (exit
   0), a one-pixel change reports the exact region (exit 1). No committed corpus: goldens
   are machine-dependent (GPU/DPI/fonts) — the corpus gets generated per-runner once a
   GPU-capable CI lane exists (the recipe is the `capture` subcommand).
3. ~~**P1 · S — Singleton inventory.**~~ **Done (post-4.9).** All 9 `::Instance()`
   singletons inventoried into the Known-gaps section with per-context candidacy
   (`fonts::Manager`/`fx::AnimationManager`/`styling::Engine`/`Toast` per-context;
   `ThemeRegistry` a read-only catalog; the rest per-app/process) as the design input
   for the still-deferred multi-context refactor.
4. **P2 — Carry-over (unchanged):** the GPU-capable runner itself, multi-context
   singletons, and the long-horizon backlog from item 8 above.

- When you complete an item, check it off here, add a line to `CHANGELOG.md`, and
  update any affected docs/badges in the same PR.
- Re-scope horizons at each release: promote, demote, or split items as reality
  dictates. Keep the vision (§1) stable; let the tactics move.
