# TeamkillerUniGUI — ROADMAP

_Last updated: 2026-06-24 · Current release: **v3.7.0** · Companion to [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md)_

This roadmap is **peer-research-informed**: it positions UniGUI against the
current Dear ImGui ecosystem (June 2026) and turns the gaps that research
surfaced into a phased plan. It continues the horizon numbering from
`DEVELOPMENT_PLAN.md` (Horizons 1–5 are largely delivered — see §1) and looks
forward to Horizons 6–11. Horizons are relative, not calendar-locked; each ships
as its own minor release with tests + docs.

---

## 1. Where UniGUI stands (v3.7.0)

- **93 retained widgets** (100% `PushID`-safe) + the `unigui::im` immediate layer
  (**201 functions = 100% of Dear ImGui's *practical* public surface**).
- **Theme engine**: Dark/Light + 13 presets, unified style/colour tokens,
  surface materials (Solid/Glass/Frosted/Acrylic/Minimal), semantic colours
  incl. `Up`/`Down` + market **`Polarity`**, elevation.
- **Trading toolkit** (Horizon 3, complete + extended): formatters/models,
  candlestick + DOM ladder + order ticket + blotters, and the v3.7.0
  client-fit batch (`EditableDataGrid`, `BasketTicket`, `GroupedRiskTree`,
  `MetricCard`, `ConnectionStatusBar`, `SessionAxis`, …).
- **DSL**, **CSS styling engine**, **EventBus**, **plugin system**, **font
  manager**, reactive **`Observable<T>`**, i18n catalog, optional modules
  (SQLite / config / IPC / network).
- **7 backends — 4 production** (GLFW+OpenGL3, GLFW/SDL3+Vulkan, DX11, DX12);
  **Metal / WebGPU / Emscripten are stubs.**
- ~110 GoogleTest files + benchmarks + fuzz targets; cross-platform CI with a
  format/tidy gate and an advisory coverage floor.

**Delivered from the prior plan:** H1 stabilization (warnings-as-errors on GCC),
H2 im-wrapper completion (100% practical surface), H3 trading toolkit, plus
H5 slices (Observable, i18n fallback chain, theme authoring). **Still open from
it:** functional web/mobile backends, visual-regression harness, hard coverage
gate — folded into the horizons below.

---

## 2. Peer landscape & positioning

| Project | Paradigm | Lang | Niche / strength | Relevance to UniGUI |
|---|---|---|---|---|
| **Dear ImGui Bundle / Hello ImGui** (pthom) | Immediate (ImGui) | C++ **+ Python** | Batteries-included app framework: window/backend/docking/assets, ImPlot(+3D), node editor, ImGuizmo, ImmVision, file dialogs, command palette, Markdown; **desktop + iOS + Android + WebAssembly**; `imgui_test_engine`; deployable HTML | **The closest peer.** Ahead on platform reach (web/mobile), Python, test-engine, add-on breadth |
| **Dear ImGui** (upstream) | Immediate | C++ | The engine. v1.92 added **dynamic fonts** (`FontScaleDpi`, on-demand CJK), multi-select, docking/multi-viewport | UniGUI must track 1.92 font system + nav |
| **Qt / QML** | Retained, declarative | C++ | Heavyweight, commercial, full a11y + tooling | Different weight class; UniGUI is the lightweight ImGui-native alternative |
| **Slint** | Retained, declarative DSL | Rust/C++/JS/Py | Embedded + desktop, `.slint` markup, GPU | Declarative competitor; UniGUI's DSL/CSS overlaps a slice |
| **RmlUi** | Retained, HTML/CSS | C++ | HTML/CSS layout engine for games | Overlaps UniGUI's CSS engine ambition |
| **Nuklear** | Immediate | C | Single-header, minimal | Lower-level; not a wrapper peer |
| **egui** | Immediate | Rust | The Rust-world IMGUI | Cross-language reference point |

**UniGUI's defensible niche** (where it leads even the Bundle): the **broadest
retained-mode widget library** over ImGui, a **token-based theme engine** with
surface materials, a **CSS styling engine + declarative DSL**, and a **real
trading/finance domain toolkit**. The roadmap protects that lead while closing
the platform/quality gaps the Bundle has and UniGUI doesn't.

---

## 3. Strategic gaps (ranked, from peer research)

| # | Gap | Evidence | Horizon |
|---|---|---|---|
| **G1** | **Web + mobile are stubs.** Hello ImGui ships functional Emscripten/iOS/Android + deployable HTML; UniGUI's Emscripten/WebGPU/Metal don't run. | Bundle feature set | **H6** |
| **G2** | **Pre-1.92 font system.** ImGui 1.92 added dynamic fonts (on-demand CJK glyphs, `style.FontScaleDpi`); UniGUI's FontManager pre-builds ranges — costly for CJK and blurry at fractional DPI (a real bug was just patched in `im` caret). | ImGui 1.92 release | **H6** |
| **G3** | **No interaction / visual-regression testing.** `imgui_test_engine` runs headless GUI tests + screenshot/video capture in CI; UniGUI has only render-no-crash + headless unit tests. | imgui_test_engine | **H7** |
| **G4** | **No accessibility bridge.** ImGui itself has keyboard/gamepad nav but **no screen-reader/AT** support anywhere in the ecosystem. UniGUI already stores `AccessibleName`/`AccessibleDescription` — unused. | ImGui a11y issues | **H9** |
| **G5** | **App-shell boilerplate gap.** Hello ImGui does layout persistence, asset/font embedding, HiDPI content-scale, one-call deployable apps; UniGUI's `RunApp` is thinner. | Hello ImGui | **H8** |
| **G6** | **Chart performance at scale.** ImPlot candlesticks drop to ~30 FPS at 50k points — UniGUI's trading charts need decimation for tick-dense series. | ImPlot perf issues | **H10** |
| **G7** | **Distribution friction.** No vcpkg-registry port / Conan recipe; consumers vendor the submodule (as `jzdz_client_suite` does). | jzdz submodule pin | **H11** |

---

## 4. Roadmap by horizon

Effort **S/M/L**, priority **P0/P1/P2**. Each item names the gap it closes.

### Status — shipped in the 3.8.x series

The 3.8.0–3.8.9 releases worked through every horizon below. Summary of what
landed (✅) and what remains deferred (⏳, with rationale):

| H | Shipped (version) | Deferred |
|---|-------------------|----------|
| **H6** | ✅ Dear ImGui 1.92 + ImPlot 1.0 toolchain, dynamic fonts, `App::SetContentScale` (3.8.0); ✅ `core/dpi.h` content-scale normalization + `SetContentScaleFromMonitor` (3.8.8) | ⏳ Functional Emscripten/WebGPU + Metal backends — runtime needs a GPU/browser, not headless-verifiable; remain stubs |
| **H7** | ✅ MSVC `/W4 /WX` warnings-as-errors gate + CI job, whole tree warning-clean on MSVC **and** GCC (3.8.4) | ⏳ `imgui_test_engine` + visual-regression (need a GL context/capture); hard coverage gate (headless coverage is a lower bound — kept advisory with a documented "flip when stable" path) |
| **H8** | ✅ `MultiSplitter::SerializeLayout`/`RestoreLayout` + `core/layout_store.h` (3.8.2) | ⏳ Asset/font embedding & one-call deploy; theme/locale persistence helpers |
| **H9** | ✅ `core/accessibility.h` focus-tracker seam + `Widget::AnnounceAccessible`, Button adopter (3.8.3) | ⏳ Platform UIA provider (Windows-specific runtime); keyboard-nav audit |
| **H10** | ✅ `core/decimate.h` LTTB/min-max (3.8.0); ✅ `DataTable` row-accessor source + sign-color + `TimeSeriesChart::SetMaxRenderPoints` (3.8.1); ✅ `TimeSeriesChart::UpsertPoint` + `SetSessionAxis` (3.8.9) | ⏳ `CandlestickChart` decimation wiring; ImPlot3D |
| **H11** | ✅ `CommandPalette` widget #94 (3.8.5); ✅ `FileDialog` widget #95 (3.8.6); ✅ consumable `find_package(unigui)` + version-drift fix + vcpkg port refresh (3.8.7) | ⏳ Conan recipe & Python bindings (no toolchain available here to verify; Python is a stretch goal); ImGuizmo/node-editor wrappers |

Deferred items are the ones that require hardware, a browser, or a toolchain not
available to headless CI — they are intentionally left rather than shipped
unverified. The originally-planned scope follows.

### Horizon 6 — Platform reach: Web + HiDPI fonts (G1, G2) — _flagship_

- **P0 · L — Functional Emscripten/WebGPU backend.** Promote the WebGPU renderer
  + Emscripten platform from stub to a runnable target; ship a `widget_gallery`
  web demo (the long-stated "web frontier"). This is the single biggest
  competitive gap vs the Bundle.
- **P0 · M — Adopt Dear ImGui 1.92 dynamic fonts.** Rebuild `FontManager` on the
  new on-demand glyph system: drop glyph-range pre-building, support
  `PushFont(NULL, size)`, and wire `style.FontScaleDpi` to per-monitor DPI. Wins
  crisp CJK + HiDPI for the CN trading use case (and supersedes the manual caret
  DPI workaround).
- **P1 · M — Functional Metal backend** for macOS-native (currently a stub).
- **P1 · S — HiDPI content-scale in `App`** (`SetContentScale` / per-viewport
  DPI), so embedders stop hand-scaling `FontGlobalScale`.

### Horizon 7 — Quality & confidence (G3)

- **P0 · L — Integrate `imgui_test_engine`.** Headless interaction tests (click
  the run/stop toggle, edit a grid cell, submit a basket ticket) beyond
  render-no-crash; run in CI.
- **P1 · M — Screenshot/visual-regression harness.** Capture per-theme/per-widget
  framebuffers via the engine's capture tool and diff in CI (closes the
  long-deferred item from `DEVELOPMENT_PLAN.md` H1).
- **P1 · S — Hard coverage gate + MSVC/Clang warnings-as-errors** (GCC already
  enforced).

### Horizon 8 — App-shell parity (G5)

- **P1 · M — Layout persistence.** Save/restore `DockSpace` + `MultiSplitter`
  ratios (the latter has no `imgui.ini` coverage) keyed by name, via
  `config::Store`.
- **P1 · M — Asset & font embedding** + a one-call deployable-app path matching
  Hello ImGui's `HelloImGui::Run` ergonomics, on top of the existing `RunApp`.
- **P2 · S — Theme/locale persistence** across restarts.

### Horizon 9 — Accessibility (G4) — _differentiator_

- **P1 · L — AT bridge.** Wire the existing `AccessibleName`/`Description` into a
  focus-change event stream and a platform accessibility provider (Windows UI
  Automation first); expose a smoke test. No ImGui peer ships this.
- **P2 · M — Keyboard-nav audit** of retained widgets under
  `NavEnableKeyboard`; document gamepad mappings.

### Horizon 10 — Data-density at scale (G6) + jzdz carry-over

- **P1 · M — Chart decimation/downsampling** for 100k+ candle/tick series in
  `CandlestickChart`/`TimeSeriesChart`; wire `SessionAxis` into the X formatter
  and add `UpsertPoint`/re-fit (the two deferred items from
  `docs/jzdz-fit-plan.md`).
- **P1 · M — `DataTable` row-accessor data source** (`count + index→const T&`) +
  built-in financial cell renderers, removing per-frame copies and raw `IM_COL32`
  in consumers.
- **P2 · M — ImPlot3D** opt-in for 3D surfaces (vol surfaces, depth-over-time).

### Horizon 11 — Ecosystem & distribution (G7)

- **P1 · M — vcpkg registry port + Conan recipe**, so consumers `find_package`
  instead of vendoring the submodule.
- **P2 · M — Add-on parity where it fits UniGUI's retained model**: a
  command-palette widget, a first-class file-dialog widget (beyond `FilePath`),
  and thin wrappers for `ImGuizmo` / `imgui-node-editor`.
- **P2 · L — Python bindings** (stretch; the Bundle's reach lever). C++ stays the
  first-class API.

---

## 5. Guiding principles & non-goals

- **ImGui-compatible, never ImGui-hostile** — raw `ImGui::` always works and
  stays auto-themed; track upstream (1.92 fonts/nav) rather than fork.
- **C++-first.** Python is a reach goal, not a core target.
- **Presentation-layer only.** Widgets stay thin; feed/order/strategy logic lives
  in the embedder (the trading toolkit's contract).
- **Semver discipline** per `docs/API_STABILITY.md`; deprecate, don't break.
- **Docs + tests are part of the feature** — no widget/API without reference
  docs, an example, and a test.
- **Non-goals:** becoming a retained-mode framework à la Qt; shipping a trading
  app; a visual GUI designer.

---

## 6. Success metrics

- **Web demo** of `widget_gallery` runs in-browser (WASM).
- **CJK renders crisply** at 1.0/1.5/2.0× DPI with the 1.92 font system; no
  pre-built glyph ranges.
- **CI runs interaction + visual-regression tests** (not just render-no-crash),
  with a hard coverage gate on all three compilers.
- **Accessibility smoke test** passes (focus + name exposure via UIA).
- **100k-point candlestick chart holds ≥60 FPS** with decimation.
- **`vcpkg install unigui`** works from a registry port.

---

## Appendix — peer-research sources

- Dear ImGui Bundle / Hello ImGui — https://github.com/pthom/imgui_bundle · https://imgui-bundle.pages.dev/
- Dear ImGui (engine; v1.92 dynamic fonts) — https://github.com/ocornut/imgui · https://github.com/ocornut/imgui/issues/8465 · https://github.com/ocornut/imgui/blob/master/docs/FONTS.md
- Dear ImGui Test Engine — https://github.com/ocornut/imgui_test_engine
- ImGui accessibility / navigation — https://github.com/ocornut/imgui/issues/787 · https://github.com/ocornut/imgui/issues/8022
- ImPlot / ImPlot3D (charting + perf) — https://github.com/epezent/implot · https://github.com/epezent/implot/issues/290 · https://github.com/brenocq/implot3d
- Useful ImGui extensions — https://github.com/ocornut/imgui/wiki/Useful-Extensions
- Landscape references — Slint https://github.com/slint-ui/slint · RmlUi https://github.com/mikke89/RmlUi · "Are we GUI yet?" https://areweguiyet.com/
</content>
