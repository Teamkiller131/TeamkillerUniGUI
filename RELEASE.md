# TeamkillerUniGUI v4.9.0 Release Notes

**Release Date:** 2026-08-14 | **Version:** 4.9.0 | **Widgets:** 95 | **Tests:** 1305 (1342 with the test engine)

> The **client-suite release**: opt-in multi-viewport with runtime proof, a trading-grade
> `TimeSeriesChart` hardening batch (span-lock / tick-spacing / range semantics), the
> `im` wrapper batch (248 functions), and the InputText / FilePath fixes that surfaced
> from a real client. Every headline feature ships with pixel-level or engine-driven
> tests — see `CHANGELOG.md` for the full list.

---

## Highlights

### Multi-viewport, proven at runtime

`AppConfig::multiViewport` (opt-in) lets ImGui windows be dragged out of the main
window into real OS windows and merged back. Landed with two latent bugs fixed (the
DX11 per-frame RTV rebind — a popped-out window used to blank the main window — and
the init-order trap that made the flag useless), the backdrop-clear contract extended
to secondary viewports (translucent materials no longer render against upstream's
hardcoded black), and a pixel-level `DXMultiViewportSmoke` that pops a window out on a
real DX11 swapchain and asserts the main window keeps rendering — `UNIGUI_RENDER_VERIFY`
now works on DX11. The capability matrix (Vulkan/SDL3/Metal still build-only) is
documented in `docs/BACKENDS.md` §7.1.

### `TimeSeriesChart` for traders

`SetYAxisSpanLock(span)` pins the Y-axis *height* while keeping panning (a trader's
"fixed axis" — wheel-zoom snaps back in one frame, centre kept); `SetYAxisTickSpacing`
sets explicit gridline steps; auto-fit padding became span-relative; `SetYAxisRange`
applies-and-releases (the `ImPlotCond_Once` trap is gone); `SetPanEnabled`/
`SetZoomEnabled` are finally deprecated — they never gated anything. All of it
driven-tested through real ImGui+ImPlot frames.

### `im` layer grows to 248 functions

Tables, printf-style text, char-buffer inputs, the style/ID stacks, clipboard,
viewport/context/font accessors — with engine-driven interaction tests for the
sortable-header and EnterReturnsTrue paths. Fixed en route: `InputText` now persists
typing under `EnterReturnsTrue` (a password box you could not type into), and
`FilePath` round-trips non-ASCII paths on Windows.

---

## Upgrade Guide

### From v4.8.x

1. **No source breaks** — the only API changes are additions plus the deprecated
   `TimeSeriesChart::SetPanEnabled/SetZoomEnabled`/`WithPanEnabled/WithZoomEnabled`
   no-ops (compile-time warnings; removal waits for a major).
2. **Opt in to multi-viewport:** `cfg.multiViewport = true` — default off (it changes
   window handling for the whole application).
3. **New readback for CI:** `UNIGUI_RENDER_VERIFY=1` now also covers DX11
   (`DX11Renderer::LastVerifyDrawn()`).

---

# TeamkillerUniGUI v3.8.0 Release Notes

**Release Date:** 2026-06-24 | **Version:** 3.8.0 | **Widgets:** 93 | **Tests:** 1000+

> First release of the **3.8.x roadmap series** (see `ROADMAP.md`). Kicks off
> **Horizon 6 — platform reach & HiDPI** with the toolchain jump to Dear ImGui
> 1.92 + ImPlot 1.0, and lands the Horizon-10 decimation core.

---

## Highlights

### Toolchain modernization: Dear ImGui 1.92.8 + ImPlot 1.0

The vendored deps move to **Dear ImGui 1.92.8** and **ImPlot 1.0** (via `vcpkg.json`
overrides). This brings ImGui's new **dynamic font system** (on-demand glyph
rasterization, `style.FontScaleDpi`) — the foundation for crisp CJK + HiDPI text
without pre-built glyph ranges — and ImPlot 1.0's `ImPlotSpec` per-call styling.
The unused `imgui-node-editor` dependency was removed (no imgui-1.92-compatible
version exists in vcpkg, and it was never linked). The entire tree builds and
tests clean against the new deps.

### HiDPI content scale (Horizon 6)

- `App::SetContentScale(float)` / `GetContentScale()` — drive `ImGuiStyle::FontScaleDpi`
  so fonts re-rasterise crisply at any scale (1.0/1.5/2.0×) via the dynamic font
  system. No glyph-range pre-building.
- `AppConfig::dpiScaleFonts` — opt into Dear ImGui's automatic per-monitor font
  DPI scaling (`io.ConfigDpiScaleFonts`).

### Series decimation (Horizon 10 foundation)

`core/decimate.h` — pure, header-only downsampling for data-dense charts:
- `LttbIndices` / `Decimate` (Largest-Triangle-Three-Buckets) preserve visual shape.
- `MinMaxBuckets` guarantee per-bucket extremes survive (volatile price/OHLC data).

Unit-tested; the reusable basis for chart render-point capping, addressing ImPlot's
large-series slowdown without mutating the stored series.

---

## Upgrade Guide

### From v3.7.x

1. **Pull + re-resolve vcpkg.** The `vcpkg.json` now pins imgui 1.92.8 + implot 1.0
   via `overrides`; reconfigure to rebuild those deps. **Clean-rebuild** the affected
   preset (`cmake-msvc.cmd --preset windows-msvc-debug`).
2. **No source breaks** — imgui 1.92 keeps the obsolete enum aliases UniGUI used
   (`ImGuiCol_NavHighlight`, `TabActive`, …); no call-site changes required.
3. **For HiDPI / multi-monitor:** set `cfg.dpiScaleFonts = true`, or call
   `unigui::SetContentScale(scale)` at runtime.

---

# TeamkillerUniGUI v3.7.0 Release Notes

**Release Date:** 2026-06-15 | **Version:** 3.7.0 | **Widgets:** 93 | **Tests:** 996

---

## Highlights

### Trading-client fit — widgets to retire hand-rolled ImGui

This release implements the UniGUI side of the **`jzdz_client_suite` fit plan** (`docs/jzdz-fit-plan.md`): a batch of widgets and helpers built to replace the patterns a real multi-strategy trading client kept hand-rolling. All are `FluentWidget`/`PushID`-safe where applicable and headless-tested.

**New widgets / helpers**

- **`EditableDataGrid<T>`** + `DataTable::SetCellRenderer` — typed per-column cell editors (`SetComboColumn`/`SetIntColumn`/`SetFloatColumn`/`SetButtonColumn`) with a `SetRowReadOnly` "frozen-when-running" predicate, rendered through the **stateless `unigui::im` layer** so there is **no per-row widget cache**. Retires the `static std::map<int,Widget>` grids.
- **`BasketTicket<T>`** — editable basket / program-trading grid (toolbar + validator highlight + deferred row removal + submit-when-valid); host owns CSV/XLSX parsing via an import callback.
- **`GroupedRiskTree`** — hierarchical risk view on `TreeView` with `Worst`/`Mean`/`Sum` parent rollup and warn/danger threshold colouring (pure, unit-tested rollup).
- **`MetricCard`** — KPI/pod tile (accent rail + status dot + value/delta/subtext or custom body).
- **`ConnectionStatusBar`** + `format::Latency` — link-health strip composing `StatusLamp` + `Sparkline` with adaptive µs/ms latency, FPS, and a reconnect countdown.
- **`SessionAxis`** — pure, header-only gap-collapsing intraday time axis (lunch/pre-post gaps removed) + `HH:MM` formatter; `AShareFutures()` ships the CN day session.
- **`ToggleButton`** (run/stop), **`ButtonGroup`** (aligned cluster), **`WidgetPool<T>`** (keyed retained-widget cache).
- **`PnlText`/`StatusText`/`GradedText`** + theme **`Up`/`Down`** semantic tokens and **`Polarity`** (CN red-up default) — centralised, market-correct P&L colouring.
- **`TagList`** — inline wrapping chip container for limit-up/down / status flags.

**Enhancements**

`DataTable` `SetEmptyText` + non-UB `SetCellCheckboxValue`; `ComboBox` placeholder + allow-empty (`-1`); `StatusLamp::SetCaption`; `ConfirmDialog::Open(onConfirm)`; `MultiSplitter::Configure` (idempotent) + per-panel min-px; `TimeSeriesChart::AppendSample`; `WeakInvokeOnMainThread` + `LifetimeToken`; trading blotters honour `theme::Polarity`.

**Fix:** a stale version smoke test (was pinned to minor 5).

---

## Upgrade Guide

### From v3.6.x

1. **Pull latest**, then **rebuild cleanly** — several widgets gained struct members, so a stale *incremental* build can misbehave. Delete the build dir and reconfigure (`cmake-msvc.cmd --preset windows-msvc-debug`).
2. **No breaking changes** — existing code compiles unchanged; the new widgets are available via `<unigui/unigui.h>`.
3. **For Chinese-market UIs**, set the colour convention once at startup:
   ```cpp
   unigui::theme::SetPolarity(unigui::theme::Polarity::RedUp); // a rise is red
   ```

### New APIs at a glance

```cpp
// EditableDataGrid — typed cell editors, no per-row cache
unigui::EditableDataGrid<Pod> grid("pods", columns);
grid.SetComboColumn(1, itemsFn, getSel, onChange)
    .SetRowReadOnly([](int, const Pod& p){ return p.running; });

// MetricCard + ConnectionStatusBar
unigui::MetricCard("acct").WithTitle("账户A").WithValue("1,234,567").WithDelta(1.2, "+1.20%");
unigui::ConnectionStatusBar("link").WithConnected(true).WithLatencyUs(850, 1200).WithFps(60);

// PnlText (polarity-aware)
unigui::PnlText(pnl, unigui::format::SignedDelta(pnl));
```

---

# TeamkillerUniGUI v3.6.0 Release Notes

**Release Date:** 2026-06-15 | **Version:** 3.6.0 | **Widgets:** 86 | **Tests:** 933

---

## Highlights

### Four new dashboard / data widgets

All four are `FluentWidget`-based, `PushID`-safe, draw-list rendered, and fully headless-tested (40 new test cases). Gated by `UNIGUI_MODULE_WIDGETS`.

- **Sparkline** — compact axis-less trend chart (Line / Area / Bar) for inline use in tables, watchlists, and KPI cards. Auto- or fixed-range, trend colouring (green up / red down), last-point dot, and a rolling `PushValue()` / `SetMaxPoints()` streaming mode. No ImPlot dependency.
- **Gauge** — circular / radial progress dial (full ring or open-bottom speedometer arc) with a theme-accent fill and centre percent / custom label. Complements the linear `ProgressBar`.
- **SegmentedControl** — compact single-select button group sharing one rounded frame (the `1D / 1W / 1M` selector) with accent highlight, optional fill-width, and an `onChange` callback.
- **PriceTicker** — horizontally scrolling symbol / price / Δ marquee with green/red ▲/▼ tinting, adjustable speed, and pause — the classic trading header strip.

These close the two deferred widget follow-ups from Horizon 3 (in-cell sparkline and the price-ticker marquee).

### Windows / MSVC hardening

- **CSS, theme, and locale parsers no longer throw on MSVC.** MSVC's `std::regex` enforces a backtracking-complexity governor (libstdc++/libc++ do not), so the CSS engine, `ImportThemeJSON()`, and `Locale::LoadFromFile()` could throw `std::regex_error(error_complexity)` on large/malformed input — `LoadFromFile` reads untrusted files, making it an unguarded crash path. All three were rewritten as linear hand-written scanners (no throw, and the CSS fuzz targets drop from ~36s-and-failing to <0.1s). The project now upholds its "parsing never throws" guarantee on every compiler.
- **`Table` numeric sort is non-throwing** — `std::stod`-in-`try/catch` (one exception per non-numeric cell, every sort) replaced with `std::from_chars`.
- **`cmake-msvc.cmd` auto-discovers vcpkg** when `VCPKG_ROOT` is unset (vcpkg on `PATH`, then the VS-bundled copy), so a fresh clone configures with the Ninja presets out of the box.

---

## Upgrade Guide

### From v3.5.x

1. **Pull latest**: `git pull origin master`
2. **Rebuild** with your usual preset (e.g. `cmake-msvc.cmd --build --preset windows-msvc-release`).
3. **No breaking changes** — existing code compiles unchanged. The four new widgets are available via `<unigui/unigui.h>` (or their individual headers) when `UNIGUI_MODULE_WIDGETS` is on.

### New APIs at a glance

```cpp
// Sparkline — inline trend chart
unigui::Sparkline px("px");
px.WithSize(80, 20).WithColorByTrend().WithData({11.2f, 11.5f, 11.1f, 11.8f, 12.0f});

// Gauge — radial KPI dial
unigui::Gauge cpu("cpu");
cpu.WithRange(0, 100).WithValue(63).WithSweepDegrees(270).WithCenterLabel("CPU");

// SegmentedControl — timeframe selector
unigui::SegmentedControl tf("tf", {"1D", "1W", "1M", "1Y"});
tf.WithSelected(0).WithOnChange([](int i, const std::string& l){ /* reload */ });

// PriceTicker — scrolling marquee
unigui::PriceTicker tape("tape", {{"AAPL","192.30",+1.2f}, {"MSFT","410.10",-0.8f}});
tape.WithSpeed(60.f);
```

---

# TeamkillerUniGUI v3.5.0 Release Notes

**Release Date:** 2026-06-02 | **Version:** 3.5.0 | **Widgets:** 82 | **Tests:** 637

---

## Highlights

### Better UX for CascadingCombo

The cascading dropdown is no longer locked to a vertical stack and is much easier to size:

- `SetLayout(Layout::Horizontal | Vertical)` — arrange combos side-by-side or stacked.
- `SetItemWidth(float)` — set one width for all combos; `SetItemWidth(int level, float)` or `Level::width` to override a single level.
- `SetSpacing(float)` — control the gap between combos in horizontal layout.
- Fluent `WithLayout(...).WithItemWidth(...).WithSpacing(...)`; `SetHorizontal(bool)` retained for compatibility.

See the new guide: [docs/CASCADINGCOMBO.md](docs/CASCADINGCOMBO.md).

### Table & DataTable improvements

- **Table**: interactive column sorting (`SetSortable`, numeric-aware default + custom `SortComparator`) and cell embedding (`SetCellRenderer`) to host any ImGui content inside a cell. Sorting now parses each cell key once for better performance.
- **DataTable**: inline checkbox columns via `SetCellCheckbox(col, fn)`, with `SpanAllColumns` auto-disabled when checkbox columns are present so clicks register correctly.

### Display polish

- **StatusLamp** gains a glow effect (`SetGlowEnabled`), with the glow's vertical padding included in the widget bounds so it lays out correctly in tables/rows.
- **Charts** follow the active theme background (`SetThemeBackground`).
- Broader embedded font glyph coverage.

### Documentation overhaul

- [docs/WIDGET_API.md](docs/WIDGET_API.md) was rewritten as a verified, categorized reference for **all 82 widgets** — constructors, methods, and copy-paste examples.
- New in-depth guides: [docs/TREEVIEW.md](docs/TREEVIEW.md) and [docs/CASCADINGCOMBO.md](docs/CASCADINGCOMBO.md).
- `README.md` / `README_zh.md` synced: correct widget (82) and test (637) counts, version, new components, and guide links.

---

## Upgrade Guide

### From v3.4.x

1. **Pull latest**: `git pull origin master`
2. **Rebuild** with your usual preset (e.g. `cmake-msvc.cmd --build --preset windows-msvc-release`).
3. **No breaking changes** — existing code compiles unchanged. `CascadingCombo::SetHorizontal(bool)` still works and now maps onto the new layout system.

### New APIs at a glance

```cpp
// CascadingCombo: horizontal layout + widths
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal)
   .WithItemWidth(120.f)
   .WithSpacing(8.f);
cc->SetItemWidth(0, 150.f);              // widen the first level only

// Table: sortable + embedded cell content
table->SetSortable(true);
table->SetCellRenderer([](int row, int col){ /* draw + return true */ return false; });

// DataTable: inline checkbox column
dt->SetCellCheckbox(0, [](int row){ return &rowChecked[row]; });

// StatusLamp glow
lamp->SetGlowEnabled(true);
```

---

**Full Changelog:** [CHANGELOG.md](CHANGELOG.md) · **API Docs:** [docs/WIDGET_API.md](docs/WIDGET_API.md)

---

# TeamkillerUniGUI v3.3.1 Release Notes

**Release Date:** 2026-06-01 | **Version:** 3.3.1 | **Widgets:** 83 | **Tests:** 579

---

## Highlights

### 9 New Widgets for Production Use

| Widget | Category | Purpose |
|--------|----------|---------|
| PanelBox | Container | Dark panel with title bar + tinted content |
| RiskBar | Display | Colored progress bar with warn/danger thresholds |
| StatusLamp | Display | Circular status indicator with draft blink |
| AlertBar | Feedback | Persistent top alert banner with animation |
| ConfirmDialog | Feedback | Modal confirmation popup with danger style |
| CascadingCombo | Input | N-level cascading dropdown (BeginCombo/EndCombo) |
| SliderBar | Input | Multi-tick draggable slider bar |
| FuturesRiskBar | Display | Multi-marker progress bar (actual/estimated/overnight) |

### Enhancements

- **TreeView**: RowRenderer callback + 6 new TreeNode fields (icon, suffix, progress, bgColor, labelColor, progressColor)
- **DataTable**: RowClickCallback, sort indicators, SetColumnMinWidth
- **ThemeRegistry**: GetCurrentThemeName()
- **FontManager**: LoadSystemEmoji() for auto emoji fallback
- **Theme**: SetFontScale() / GetFontScale()

### Bug Fixes

- **DataTable StickyHeader**: Added TableSetupScrollFreeze(0,1) — header now stays fixed
- **Emoji rendering**: Auto-loads Segoe UI Emoji on Windows

---

# TeamkillerUniGUI v3.3.0 Release Notes

**Release Date:** 2026-06-01  
**Version:** 3.3.0  
**C++ Standard:** C++23  
**Platforms:** Windows (MSVC + Clang), Linux (GCC), macOS (Clang)

---

## Highlights

### 🛡️ 100% ID Safety — Zero Widget ID Collisions

Every one of the **74 widgets** now automatically scopes ImGui IDs via `PushID(name)/PopID()`. Same widget label? No problem.

```cpp
auto ok    = std::make_shared<unigui::Button>("btn_ok",    "OK");
auto ok2   = std::make_shared<unigui::Button>("btn_ok2",   "OK"); // same label, NO conflict!
```

### 🆕 6 New Widgets

| Widget | Description |
|--------|-------------|
| `CollapsingHeader` | Expandable section with content callback |
| `Selectable` | Highlightable list/menu item |
| `ColorEdit` | Hex color editor with preview (`#RRGGBBAA`) |
| `DragFloat` | Drag-adjustable float input |
| `DragInt` | Drag-adjustable integer input |
| `ListBox` | Scrollable item list with selection callback |

### 🔬 Developer Tooling

| Tool | How |
|------|-----|
| **Coverage** | `cmake --build build/windows-clang-coverage --target coverage` → HTML report |
| **clang-tidy** | `.clang-tidy` config + `windows-clang-tidy` preset |
| **clang-format** | `.clang-format` (4-space K&R style) |
| **Sanitizers** | ASAN presets for Windows + Linux |
| **compile_commands.json** | Auto-generated for all builds (IDE + clang-tidy) |

### 📚 Complete API Documentation

[docs/WIDGET_API.md](docs/WIDGET_API.md) — 1746 lines, all 74+ widgets with C++23 code examples.

---

## Changelog

### Added
- **6 new widgets**: `CollapsingHeader`, `Selectable`, `ColorEdit`, `DragFloat`, `DragInt`, `ListBox`
- **ID Safety**: 100% PushID/PopID coverage across all 62 widget Render() methods
- **`.clang-format`** + **`.clang-tidy`** configs
- **Coverage pipeline**: `windows-clang-coverage` preset + `--target coverage`
- **clang-tidy preset**: `windows-clang-tidy` — lint on every compile
- **`cmake-msvc.cmd`**: Portable MSVC build wrapper (vswhere-based)
- **`compile_commands.json`**: Auto-generated in all builds
- **ASAN presets**: `windows-msvc-debug-asan`, `linux-gcc-debug-asan`
- **`docs/WIDGET_API.md`**: 1746-line API reference

### Changed
- **Tests**: 245 → 285 (100% pass on MSVC and Clang)
- **Widgets**: 68 → 74
- **CMakePresets.json**: 6 → 10 presets

### Fixed
- **ID collisions**: 47 widgets missing PushID/PopID — all fixed
- **badge.cc**: Non-Widget class incorrectly receiving Widget API calls
- **listbox.cc**: ImGui::ListBox getter signature corrected

---

## Upgrade Guide

### From v3.2.x

1. **Pull latest**: `git pull origin master`
2. **Rebuild**: 
   ```bash
   cmake-msvc.cmd --preset windows-msvc-debug
   cmake-msvc.cmd --build --preset windows-msvc-debug
   ```
3. **No API breakage** — existing code compiles without changes.

### New APIs

```cpp
// CollapsingHeader
auto header = std::make_shared<unigui::CollapsingHeader>("cfg", "Settings", true);
header->SetContentCallback([] { /* render content */ });

// Selectable
auto sel = std::make_shared<unigui::Selectable>("item1", "Option A");
if (sel->WasClicked()) { /* ... */ }

// DragFloat / DragInt
auto drag = std::make_shared<unigui::DragFloat>("vol", "Volume", 0.5f, 0.01f, 0.0f, 1.0f);
if (drag->WasChanged()) { float v = drag->GetValue(); }

// ColorEdit
auto color = std::make_shared<unigui::ColorEdit>("bg", "Background", 0.2f, 0.2f, 0.3f);

// ListBox
auto lb = std::make_shared<unigui::ListBox>("files", "Choose", std::vector<std::string>{"a.txt", "b.txt"});
lb->SetOnChange([](int idx) { /* selection changed */ });
```

---

## Build Matrix

| Preset | Compiler | Features |
|--------|----------|----------|
| `windows-msvc-debug` | MSVC | Daily dev |
| `windows-msvc-release` | MSVC | Optimized |
| `windows-clang-coverage` | Clang | Coverage report |
| `windows-clang-tidy` | Clang | Static analysis |
| `windows-msvc-debug-asan` | MSVC | AddressSanitizer |
| `windows-msvc-sdl3-vulkan-debug` | MSVC | Vulkan backend |
| `linux-gcc-debug` | GCC | Linux dev |
| `linux-gcc-debug-asan` | GCC | Linux ASAN/UBSAN |
| `macos-clang-debug` | Clang | macOS dev |

---

## Stats

| Metric | v3.2.8 | v3.3.0 |
|--------|--------|--------|
| Widgets | 68 | **74** |
| Tests | 245 | **285** |
| ID Safety | 26% | **100%** |
| CMake Presets | 6 | **10** |
| Documentation | README only | **1746-line API doc** |
| clang-format | None | **Configured** |
| clang-tidy | None | **Configured** |
| Coverage | None | **41.4% lines** |

---

**Full Changelog:** [CHANGELOG.md](CHANGELOG.md)  
**API Docs:** [docs/WIDGET_API.md](docs/WIDGET_API.md)
