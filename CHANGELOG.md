## [Unreleased]

### Fixed
- **Accessibility correctness pass** (found by an adversarial review of the 4.4.0 a11y code):
  - **Linux AT-SPI bridge emitted a malformed signal** — the `Announcement` body used
    `(siiv(so))`, but every `org.a11y.atspi.Event.*` signal is `(siiva{sv})` (a properties
    dict, not a source tuple). Orca/atk-bridge couldn't decode it, so the bridge was a
    silent no-op. Now matches GTK's emitter `(siiva{sv})` with an empty dict (the daemon
    supplies the sender in the header).
  - **ComboBox out-of-bounds read** — the a11y value string reused a `hasSel` captured
    before the dropdown opened; picking "(none)" the same frame set `selected_ = -1` and
    `items_[selected_]` read out of bounds. Re-checks bounds at the call site.
  - **Unbounded announcement queue** — `Announce()` appended to a queue nothing in the app
    loop drains, so a subscribe-only app grew it forever. The queue is now capped (256).
  - **Stale focus** — `Focused()` never auto-cleared (`ClearFocus` was only called by
    tests). `BeginFrame()` now clears focus when the prior frame reported none.
  - **Windows bridge stale HWND** — re-installing after a window recreate kept the old
    handle; the provider now re-points to the new HWND on every install (matching macOS).

## [4.4.0] - 2026-06-30

> **Accessibility across all four platforms, plus CI that verifies pixels.** UniGUI gains a
> real accessibility layer — a per-frame element tree, ARIA-style live announcements,
> keyboard navigation, an inspector, and screen-reader bridges for **Windows** (UI
> Automation), the **web** (ARIA live regions), **macOS** (NSAccessibility), and **Linux**
> (AT-SPI) — with **~39 widgets** wired and a one-flag opt-in (`AppConfig::accessibility`).
> A new framebuffer-readback CI gate catches "renders nothing" regressions the old log-only
> smoke missed; the GLFW+OpenGL3/WebGL black-screen bug it would have caught is fixed.

### Added
- **Accessibility: a real a11y tree + live announcements + keyboard nav** (`unigui::a11y`).
  The focus tracker now backs a fuller accessibility layer: a **per-frame element tree**
  (`BeginFrame`/`AddNode`/`Tree`) so a bridge, inspector, or test can enumerate the whole
  UI; **ARIA-style live announcements** (`Announce` with `Live::Polite`/`Assertive`,
  drainable or callback-driven) for focus-independent status/validation messages; richer
  `Node` state (`focused`/`disabled`) and roles (Toggle/Progress/Window/Group/Status/…);
  an in-app **`a11y::DrawInspector()`** (tree + focus + recent announcements) and bridges:
  the reference **`InstallLoggingBridge()`** (logger) and a real **`InstallSystemBridge()`**
  with a per-platform implementation: **Windows** raises UI Automation notification events
  (Narrator / NVDA / JAWS); the **web** build mirrors focus/announcements into an ARIA
  live region in the page DOM (any browser screen reader); **macOS** posts NSAccessibility
  announcement notifications (VoiceOver); **Linux** emits AT-SPI2 `Announcement` events over
  the a11y D-Bus via GLib/GIO (Orca) when built with `-DUNIGUI_A11Y_ATSPI=ON`, else logging.
  The new **`AppConfig::accessibility`** flag
  opts the whole layer in with one line (installs the bridge once the window is up). The app loop
  now enables **`ImGuiConfigFlags_NavEnableKeyboard`** (Tab/arrow keyboard navigation) and
  resets the tree each frame; **~39 widgets** report into it via the new
  `Widget::ReportAccessible(role, focused, value, disabled)` — buttons (Button, IconButton,
  ImageButton, ToggleButton, ButtonGroup), inputs (LineEdit, InputText/Int/Float,
  PasswordInput [presence only, never the secret], Multiline, SearchBox), selections
  (CheckBox, RadioGroup, ToggleSwitch, ComboBox, MultiCombo, CascadingCombo, ListBox,
  ListView, Selectable, SegmentedControl), sliders (DragInt/Float, SliderBar,
  MultiHandleSlider), containers (TabWidget, TreeView, Table, CollapsingHeader), color
  (ColorEdit, ColorPicker), DatePicker, Hyperlink, and chrome (MenuBar, ToolBar, Label,
  StatusBar, ProgressBar). Disabled by default
  (zero per-frame cost). The web `web_demo` gains an **Accessibility** tab demoing it.

### Fixed
- **Font manager probed a Linux-only path on the web.** The emoji loader fell through to
  `/usr/share/fonts/.../NotoColorEmoji.ttf` under Emscripten and logged a misleading "not
  found" warning (the wasm MEMFS has no system fonts). It now skips the probe on the web
  with a debug note; the CJK loader likewise no longer does futile `fopen`s there. Desktop
  behavior (Windows/macOS/Linux system-font merge) is unchanged. To get CJK/emoji on the
  web, load a font explicitly via the font manager.

### Added
- **Pixel-level render verification in CI.** Setting `UNIGUI_RENDER_VERIFY=1` makes the
  app read the GL framebuffer back after `RenderDrawData` and log
  `[render-verify] glError=… nonClear=N/total drawn=true|false` — proving the UI actually
  drew pixels rather than just clearing. The Linux headless smoke now asserts `drawn=true`
  after a clean run, so a "renders nothing" regression (like the 4.3.1 black screen) fails
  CI instead of slipping past a log-only check. Inert (zero cost) unless the env var is set,
  and GL-backends only.

## [4.3.1] - 2026-06-29

### Fixed
- **GLFW+OpenGL3 / WebGL backend rendered nothing (black screen).** The OpenGL3 renderer
  never called `ImGui_ImplOpenGL3_NewFrame()`, which lazily builds the GL device objects
  (shader program + vertex/index buffers + font texture). Without it ImGui's draw calls
  ran with no program/buffer bound — the GL log filled with `INVALID_OPERATION:
  drawElements: no valid shader program in use` / `bufferData: no buffer`. Added a
  default-no-op `RendererBackend::NewFrame()` hook, overridden by the OpenGL3 renderer and
  invoked before `ImGui::NewFrame()`. This also repairs the desktop GLFW+OpenGL3 backend;
  it had slipped through because Windows defaults to DX11, the macOS CI runner can't make a
  GL context, and the Linux smoke greps log lines rather than pixels. Confirmed fixed
  in-browser on the WebGL2 `web_demo`.

### Changed
- **`web_demo` is now a tabbed widget gallery.** One window with a tab bar showcasing ~40
  UniGUI widgets (Buttons, Inputs, Toggles, Selection, Progress, Layout, Navigation, Data,
  File & Color, Info, Cards & Effects) plus a live theme switcher — instead of leaning on
  `ImGui::ShowDemoWindow()`. The misleading CJK panel (tofu without a CJK font on the web)
  is replaced with an honest note.

## [4.3.0] - 2026-06-29

> **WebGPU on the Web — the last backend stub is gone.** UniGUI now also compiles and
> links as a WebAssembly + **WebGPU** application, alongside the WebGL2 path. All seven
> backends are now real.

### Added
- **WebGPU renderer (Emscripten).** `WebGPURenderer` renders to the HTML5 `#canvas`
  through the browser's WebGPU and drives `imgui_impl_wgpu`. imgui 1.92's wgpu backend
  requires the modern `webgpu.h` (WGPUStringView, the surface API, CallbackInfo structs),
  so the WebGPU build uses Emscripten 4.0.10's `--use-port=emdawnwebgpu` (the legacy
  `-sUSE_WEBGPU` binding no longer compiles). Device acquisition is asynchronous: BringUp
  starts the adapter/device request with `AllowSpontaneous` callbacks (which fire from the
  browser event loop) and the renderer stays inert until the device callback configures
  the surface and runs `ImGui_ImplWGPU_Init` — early frames simply draw nothing, which
  fits the browser RAF loop. Enable with `-DUNIGUI_WEB_WEBGPU=ON` under an `emcmake`
  configure; `web_demo` then targets `BackendType::WebGPU`.
- **CI builds both wasm backends.** The `emscripten` job now build-verifies the WebGL2
  *and* WebGPU `web_demo` (emsdk 4.0.10) and uploads each as a separate artifact
  (`unigui-web-demo`, `unigui-web-demo-webgpu`). CI has no GPU/browser, so build + link
  are gated in CI and the in-browser runtime is validated manually with the artifacts.

### Changed
- The Emscripten wasm builds move to emsdk **4.0.10** (required by emdawnwebgpu; the
  WebGL2 build runs on it unchanged).

## [4.2.0] - 2026-06-29

> **Two backends go from stub to real: macOS Metal and the Web.** The Metal renderer is
> now a working `imgui_impl_metal` implementation, and UniGUI compiles, links, and runs
> as a WebAssembly + WebGL2 application — the full framework (95 widgets, theme engine,
> DSL, styling, immediate-mode layer) in the browser, build-verified in CI with a
> downloadable `web_demo` artifact.

### Added
- **Metal renderer (macOS).** `MetalRenderer` drives `imgui_impl_metal` against a
  `CAMetalLayer` attached to the GLFW window's `NSView`: device + command queue setup in
  `BringUp`, per-frame drawable acquisition + render-pass encoding in `NewFrameMetal`,
  and present/commit in `RenderDrawData`. Compiled as Objective-C++ with ARC. The macOS
  CI job build-verifies it. Replaces the previous clean-fallback stub.
- **Emscripten / WebGL2 web backend.** UniGUI now cross-compiles to WebAssembly. A wasm
  build can't use the vcpkg dependency stack (glad is desktop-GL-only; GLFW/WebGL2/
  freetype come from Emscripten ports), so `cmake/Emscripten.cmake` sources imgui
  (docking), implot, and spdlog via FetchContent and exposes the same target names the
  library expects. The renderer runs through the OpenGL3 backend targeting GLES3/WebGL2
  (`#version 300 es`), and the browser owns the frame loop via `emscripten_set_main_loop`.
- **`web_demo` example.** A self-contained `web_demo.html`/`.js`/`.wasm` showing the
  retained-mode widgets, theme, and immediate-mode helpers running in the browser. Built
  by a new `emscripten` CI job (emsdk + `emcmake`) that also uploads it as an artifact.

### Fixed
- **Emscripten backend brought up no window.** The bespoke `EmscriptenPlatform` only
  adopted a window handle passed to `Init()`, but the app loop calls `Init(nullptr)` — so
  `BackendType::Emscripten` created no GL context and `NewFrame()` never drove
  `ImGui_ImplGlfw`; it would have rendered nothing. Emscripten's GLFW port implements the
  same `glfw3.h` APIs against the HTML5 canvas + WebGL2, so the platform now delegates to
  the proven `GLFWPlatform` (real context + `ImGui_ImplGlfw_InitForOpenGL`).
- **WebGL canvas was never cleared.** `Render()` issued the GL clear + buffer swap only
  for `BackendType::GLFW_GL3`; the Emscripten backend renders through the same OpenGL3
  path, so the clear/swap now applies to both GL backends.

## [4.1.1] - 2026-06-28

### Fixed
- **`/W4 /WX` build (the `windows-werror` CI gate):** `widgets/filepath.cc`,
  `widgets/dirpath.cc`, and `ipc/ipc.cc` redefined `NOMINMAX` (and
  `WIN32_LEAN_AND_MEAN`), colliding with the project-wide definitions added in 3.17.0
  → `C4005`. The local defines are now `#ifndef`-guarded. (The warnings-as-errors gate
  had been silently red since 3.17.0 — surfaced while wiring up the backend CI.)

### Changed (CI)
- The headless backend smoke from 4.1.0 is finalized so it genuinely verifies the
  GLFW+OpenGL3 backend **runs** on Linux. A gdb backtrace showed the earlier Linux
  segfault is *inside* the Mesa software-GL driver (`libgallium`) during
  `ImGui_ImplOpenGL3_RenderDrawData` — a driver bug, not the backend, which brings the
  3.3-core context + `#version 150` shaders + renderer up cleanly. The Linux job now
  prefers a full clean render under llvmpipe then softpipe, and only if both crash in
  the driver falls back to asserting the backend reached a working GL state. The macOS
  job is best-effort (GitHub macOS runners have no headless GL session; the identical
  GL path is validated at runtime by the Linux job).

## [4.1.0] - 2026-06-28

> **Backend hardening + CI that actually runs the backends.** A backend audit found
> that macOS and the Web build were broken on their primary paths and that the
> Linux/macOS CI only ever proved the backends *compiled*. The cross-platform paths
> are fixed and CI now runs a headless `--frames` smoke that asserts the GLFW+OpenGL3
> backend boots and renders.

### Fixed
- **macOS: the default GLFW+OpenGL3 backend now works.** Two independent defects made
  it non-functional: the GLFW core-profile context omitted `GLFW_OPENGL_FORWARD_COMPAT`
  (so `glfwCreateWindow` returned null on every Mac, and with Metal a stub the fallback
  ladder bottomed out at "no usable backend"); and the renderer used GLSL `#version 130`,
  invalid in a core profile. Now: `GLFW_OPENGL_FORWARD_COMPAT` on Apple + `#version 150`
  everywhere (valid on any GL ≥ 3.2 core context).
- **The Web (Emscripten) build now compiles.** `emscripten_platform.cc` passed `int*`
  to `emscripten_get_element_css_size` (which takes `double*`) — a hard compile error
  hidden from desktop CI; and `EmscriptenSetMainLoop` registered the address of a
  by-value `std::function` (a per-frame use-after-free). Both fixed (+ a HiDPI canvas
  backing-size correction).
- **Vulkan renderer no longer leaks on partial bring-up failure.** `Shutdown()` gated
  teardown on a `ready` flag set only at the very end, so any mid-bring-up failure
  (no ICD, llvmpipe, split present queue — common on headless Linux) leaked the
  `VkInstance`/`VkDevice`/pool/surface. `Shutdown()` is now idempotent and every
  failure routes through it; the swapchain is validated after creation.
- **SDL3 platform no longer destroys a host-owned window or calls global `SDL_Quit()`.**
  It now tracks window/subsystem ownership, uses refcount-safe `SDL_InitSubSystem`/
  `SDL_QuitSubSystem(VIDEO)`, checks `ImGui_ImplSDL3_InitForVulkan`, null-checks
  `SDL_Vulkan_GetInstanceExtensions`, and logs every failure path.
- **HiDPI/retina text is no longer blurry on macOS/Linux.** `DetectDPIScale` only
  implemented the Win32 path (returning 1.0 elsewhere); the app now uses the platform
  window's content scale via a new `PlatformBackend::GetContentScale()`.
- **Backend factory honours the `{nullptr,nullptr}` contract.** The WebGPU and
  Emscripten cases returned half-pairs `{valid platform, null renderer}`; they are now
  gated, and Emscripten pairs with the OpenGL3 (WebGL) renderer it actually uses.
- Smaller: a GLFW error callback for diagnosable bring-up failures; checked
  `ImGui_ImplGlfw_Init*` returns; a `glfwGetRequiredInstanceExtensions` null guard; an
  OpenGL3 double-init guard; the Metal stub no longer creates an ImGui context as a
  side effect of its failing `Init()`.

### Added
- **`PlatformBackend::GetContentScale()`** (default 1.0) — the HiDPI content-scale
  factor, implemented for GLFW (`glfwGetWindowContentScale`) and SDL3.
- **CI now verifies the backends RUN, not just compile.** The Linux job runs
  `hello_unigui --frames 10` under `xvfb` + software `llvmpipe` (a real 3.3-core GL
  context) and the macOS job runs it directly — both asserting exit 0 (no `|| true`),
  so a fail-to-launch or fail-clean Init fails the job.

## [4.0.0] - 2026-06-28

> **Breaking: `Result<T>` is now `std::expected`.** The hand-rolled result type is
> replaced by a thin alias over `std::expected<T, ErrorCode>`, and the two fallible
> APIs the audit flagged adopt it. Major because the error-construction and
> `value()`-access semantics change.

### Breaking
- **`unigui::Result<T>` is now `using Result = std::expected<T, ErrorCode>`.**
  - Construct an error with **`Err(ErrorCode::X)`**. The old implicit
    `Result<T>(ErrorCode)` constructor is gone — `std::expected` requires the error
    wrapped in `std::unexpected`, which `Err()` does for you.
  - `value()` on the error path now **throws `std::bad_expected_access<ErrorCode>`**
    instead of being undefined behaviour (the predecessor returned a dangling ref).
  - In exchange: the full monadic surface (`and_then` / `or_else` / `transform` /
    `value_or`), and the inconsistent "no value, error == None" state the old type
    permitted is no longer representable.
- **`sqlite::Database::Open`** now returns `Result<void>` (was `bool`) —
  `Err(ErrorCode::OpenFailed)` on failure.
- **`config::Store::LoadTOML` / `LoadJSON` / `LoadINI`** now return `Result<void>`
  (was `bool`), distinguishing `Err(ErrorCode::FileNotFound)` from
  `Err(ErrorCode::ParseFailed)`.
- New `ErrorCode` entries: `FileNotFound`, `ParseFailed`, `OpenFailed`.

### Migration
- `return ErrorCode::X;` → `return Err(ErrorCode::X);`
- `if (!db.Open(p)) …` still compiles (`std::expected` has an explicit
  `operator bool`); prefer `if (auto r = db.Open(p); !r) handle(r.error());` to use
  the code.

### Notes
- The wide virtual `Init()` hierarchy (`app::Init`, `PlatformBackend` /
  `RendererBackend::Init`, `IPlugin::Init`) intentionally stays `bool` — converting a
  deep virtual interface to `Result` is a large break for little gain, and is out of
  scope here.

## [3.19.0] - 2026-06-28

> **P2/P3: lifetime hardening, the untested seams get tests, per-frame allocations
> trimmed, and safe modernization.** Closes out the audit roadmap.

### Fixed
- **`Observable::Notify()` no longer reads a destroyed `*this`.** An observer is
  allowed to destroy the Observable from inside its callback (e.g. a bound widget
  being torn down); `Notify` now copies the value to a local before the dispatch
  loop, so it touches no member of `*this` once notification begins.

### Changed
- **Per-frame allocations trimmed on hot widgets.** `DepthLadder::Render` derives
  `maxSize` from the bid/ask level vectors it already built instead of calling
  `OrderBook::MaxSize()` (which rebuilt them — two extra allocations + map walks each
  frame). `Table` no longer materialises a `std::string` per visible cell: the draw
  helpers take `std::string_view` (ImGui begin/end text overloads) and the cell
  formatter returns a view into the stored cell on the common path, allocating only
  when a unit must be appended.
- **Docs accuracy.** The `SolveFlex` worked example in `core/flex_layout.h` and
  `docs/ARCHITECTURE.md` showed a numerically impossible result (200/200, "300px
  free"); corrected to the solver's real output (166.667/233.333, 200px free). The
  README test-count prose no longer contradicts the badge, and the `CLAUDE.md`
  trading row reflects the shipped widgets.

### Added
- **`std::span` overloads for `PlotLine`/`PlotBars`/`PlotScatter`** (`ext/plot.h`,
  `@experimental`) — bounds-safe, taking the shorter of the two spans so mismatched
  lengths cannot over-read.
- **`network::SplitUrl()`** — the URL scheme/host/path split is now a pure, unit-
  tested free function (declared in `network.h`); `HttpClient::Get`/`Post` share it
  instead of duplicating the slash-finding.
- **`MultiHandleSlider::GetRangeMin()`/`GetRangeMax()`** accessors.
- `explicit` on the single-argument `sqlite::Transaction` and `NodeEditor`
  constructors.
- Tests: `UndoStack` max-depth eviction / redo-tail truncation / `RedoDepth`;
  `MultiHandleSlider` tick + range + render; `network::SplitUrl` (new
  `tests/network/url_parse_test.cc`); and an LTTB-decimator perf budget in
  `tests/bench`.

### Deferred
- Migrating `core/error.h`'s hand-rolled `Result<T>` to `std::expected` is a
  semver-major change (its `value()`/`error()` surface is governed/tested) and is
  intentionally left for a future major release.

## [3.18.0] - 2026-06-28

> **P1 hardening: build robustness, header hygiene, and the never-tested module
> seams.** Continues acting on the audit — the optional modules and backend
> configurations now build in the shapes the docs advertise, public headers stop
> leaking their dependencies, and the async/lifecycle code paths get real tests.

### Fixed
- **`UNIGUI_BACKEND_DX11=OFF` now links on Windows.** `dx11_renderer.cc` was
  compiled unconditionally while the d3d11/dxgi libraries linked only under
  `UNIGUI_BACKEND_DX11`, so the documented "disable DX11" path produced unresolved
  externals. It is now gated like `dx12_renderer.cc` (and `CreateDX11Renderer()` is
  declared only under `UNIGUI_HAS_DX11`); `app.cc`/`backend_factory.h` already
  guarded their references. A new `windows-msvc-debug-no-dx11` preset exercises it.
- **`sqlite::Row::Get(const char*)` was declared but never defined** — a guaranteed
  linker error for any caller. It is now implemented (each `Row` carries the column
  names parallel to its values), returning "" for an unknown column.
- **`EventBus::Shutdown()` now drains pending async events before stopping.** The
  worker thread broke out of its loop on the shutdown flag *before* draining the
  queue, silently dropping already-published `PublishAsync` events. It now drains,
  then stops; the contract is documented and tested.

### Changed
- **Public headers no longer leak their implementation dependencies.** `config.h`
  drops `cpptoml`, `nlohmann/json`, and an unused `<any>` (moved into `config.cc`);
  `network.h` drops `<httplib.h>` (used only in the `.cc`); `undo_stack.h` drops a
  dead `<stdexcept>`; `trayicon.h` and `ipc/shmem.h` no longer pull `<windows.h>`
  (the tray menu handle is type-erased to `void*`; the shmem mapping headers move to
  the `.cc`); and the `unigui.h` umbrella drops its unused `<windows.h>`/GLFW-native
  block. Combined with the project-wide `WIN32_LEAN_AND_MEAN`/`NOMINMAX` from 3.17.0,
  consumers stop inheriting heavy system includes and macro pollution.

### Added
- **`Bus::CreateForTesting()`** — a test-only seam to construct a standalone
  `EventBus` so `Shutdown()`/draining can be exercised without tearing down the
  shared singleton.
- Tests: EventBus `PublishAsync` delivery + `Shutdown` drain + idempotency; Plugin
  `Manager::Reload` (unknown → null, built-in-without-path → null, instance survives);
  and `sqlite::Row::Get(const char*)` by column name.

## [3.17.0] - 2026-06-28

> **Security & correctness hardening, plus the optional-module build resurrected.**
> A multi-dimension audit surfaced four reachable bugs — all fixed with regression
> tests — and revealed that the optional modules (IPC / network / config / SQLite)
> had never actually compiled on Windows. They now build, test, and ship with a
> dedicated preset.

### Security
- **`ipc::SharedMemory::Write`/`Read` — integer-overflow bounds check allowed
  out-of-bounds reads/writes across the (untrusted) shared-memory boundary.**
  `offset + size <= size_` wraps in unsigned arithmetic (e.g. `offset == SIZE_MAX`),
  defeating the guard so `memcpy` runs outside the mapped view. It now checks
  `offset <= size_ && size <= size_ - offset`. Fixed in both `src/ipc/ipc.cc` and
  `src/ipc/shmem.cc`, with an overflow-offset regression test.

### Fixed
- **`TabWidget` use-after-free when a tab's content callback adds or removes tabs.**
  The render loop held a reference into `tabs_` across the user callback; an `AddTab`
  (reallocation) or `RemoveTab` (erase) from inside it freed the executing
  `std::function`. Structural mutations issued while rendering are now deferred and
  applied after `EndTabBar`. Reachable from an ordinary open/close-tab button.
- **`WebSocketClient` data race / use-after-free on its callbacks.** `onMsg_` /
  `onOpen_` / `onClose_` were read by the IXWebSocket background thread while the
  setters rewrote them unsynchronized. They are now mutex-guarded, and the message
  handler snapshots the relevant callback under the lock before invoking it outside
  the critical section.
- **CSS gradient parser threw `std::out_of_range` on a gradient with no hex color**
  (e.g. `linear-gradient(to right, red, blue)`), breaking the engine's documented
  non-throwing contract. It now returns cleanly when there is no `#`.
- **Removed the banned throwing `std::stod` from `widgets/form.cc` and the public
  `widgets/datatable.h`**, restoring the project-wide "no throwing parsers" rule.
  Both now use the new non-throwing `unigui::TryToDouble` (see Added).
- **`core/path_util.h` — `PathFromUtf8` now builds under `/Zc:char8_t-`.** Guarded
  behind `__cpp_char8_t`: the `u8string` path when char8_t is on, a `u8path`
  fallback when it is off. No behaviour change where char8_t is enabled.
- **The optional modules now compile and link on Windows.** Gated OFF in every
  preset, they had never been built and had accumulated latent breakage:
  `src/ipc/ipc.cc` carried a duplicate, never-compilable `SharedMemory` definition
  (removed in favour of the canonical `shmem.cc`); the config module linked a
  non-existent `cpptoml::cpptoml` target (the real one is `cpptoml`); SQLite linked
  the deprecated `SQLite::SQLite3`; the network module hit a `<winsock2.h>` /
  `<windows.h>` include-order conflict and was missing `bcrypt` (for mbedtls); and a
  stale config fuzz test constructed the now-singleton `config::Store` directly.

### Added
- **`unigui::TryToDouble(const std::string&, double&)`** in `core/strutil.h` — a
  non-throwing leading-double parser returning `bool` + out-param, preserving the
  numeric/non-numeric distinction that `std::stod` callers relied on.
- **`windows-msvc-debug-modules` CMake preset** (configure + build + test) — builds
  with every optional module ON plus the vcpkg manifest features that supply their
  dependencies, so the module and module-test code the default presets never
  compiled is actually exercised.
- **Project-wide `WIN32_LEAN_AND_MEAN` + `NOMINMAX` on Windows**, so the lean
  `<windows.h>` is used everywhere and the `min`/`max` macros never leak into
  consumer translation units.
- Regression tests: TabWidget add/remove-from-callback (ASan-friendly), SharedMemory
  overflow-offset no-op, CSS named-color/`none` gradients don't throw, a
  concurrent-setter WebSocket smoke test (new `tests/network/`), and the resurrected
  config fuzz target.

## [3.16.0] - 2026-06-26

> **Framework, phases 2 & 4 — the golden path, a flagship app, and the inspector.**
> The component framework now has a documented way in, a real reference app, and
> live introspection — completing the framework-transformation arc.

### Added
- **The framework guide** — `docs/FRAMEWORK.md`: the golden path for building
  apps (Components + reactive State, composition with `Host`, `Store` + `Watch`
  for shared state, effects/lifecycle, `Navigator`, the inspector, and the
  `dsl::Custom` escape hatch), with a "which layer when" table.
- **`examples/framework_demo`** — a complete multi-screen app built entirely in
  the framework idiom: a shared `Store`, a `Navigator` with two screens,
  component-local `State`, `Watch`, an effect with cleanup, and the inspector
  overlay. Runs headless (`--frames N`).
- **`dsl::DrawInspector()`** — a live overlay listing every mounted `Component`
  with its mount/dirty state and rebuild count, backed by a component registry
  (`Component::BuildCount()`, `Component::InspectorName()`).

### Fixed
- **`dsl::Navigator::Render`** now holds a strong reference to the current screen,
  so a screen can safely `Pop()`/`Replace()` itself from a mid-frame callback (it
  destructs after its `Render()` returns rather than under it).

## [3.15.0] - 2026-06-26

> **Framework, phase 3 — the application layer.** Above the component model:
> shared state, routed screens, and a lifecycle for effects.

### Added
- **`dsl::Store<T>`** (`dsl/app.h`) — shared, app-wide reactive state (vs. a
  component-local `State`). `Get`/`Set`/`Update`, feeds `Computed`/`Bind` via
  `AsObservable()`. Held outside the component tree and shared across components.
- **`dsl::Navigator`** — a stack of screens (`Component`s): `Push`/`Pop`/`Replace`
  and `Render()` the top one each frame. Owns its screens; leaving a screen
  unmounts it (running its cleanups + `OnUnmount`).
- **`Component::Watch(source)`** — the bridge from shared state to a component's
  view: re-renders the component whenever `source` (a `Store`, another component's
  `State`, a `Computed`, or a raw `Observable`) changes. Establish in `OnMount()`.
- **`Component::OnCleanup(fn)`** — register an effect teardown run (reverse order)
  when the component unmounts, pairing with setup done in `OnMount()`.
- **`Observable<T>::AsObservable()`** — identity accessor so `Watch` treats
  `Observable`/`State`/`Store`/`Computed` uniformly.
- 7 new tests (Store get/set/update + Computed, Watch-driven re-render, same-value
  no-op, reverse-order cleanups, Navigator push/pop/replace + unmount).

## [3.14.0] - 2026-06-26

> **Framework, phase 1 — the component model.** The keystone that turns UniGUI
> from a widget library into an application framework: self-contained, stateful,
> composable units with reactive state and dirty-tracked rebuilds.

### Added
- **`dsl::Component` + `dsl::State<T>`** (`dsl/component.h`). A `Component`
  overrides `Build()` to declare its view as a DSL node tree from its `State`; the
  framework mounts it once (`OnMount`), rebuilds the tree only when a `State`
  changes (dirty tracking), and renders the cached tree each frame. `State<T>` is
  a reactive cell built on `Observable<T>` — writing change-detects and marks the
  owning component dirty, and it feeds `Computed<T>`/`Bind` via `AsObservable()`.
  Components compose with **`dsl::Host(child)`**, each child keeping its own state
  and dirty tracking. Lifecycle hooks `OnMount`/`OnUnmount`. 8 new tests.
- **`dsl::Custom(draw)`** — a DSL escape hatch node that runs an arbitrary
  immediate-mode (`unigui::im`) draw callback each frame, so any custom drawing —
  or a hosted `Component` — can live inside a declarative tree.

## [3.13.0] - 2026-06-26

> Layout system, rounded out: cross-axis alignment in the FlexRow container,
> flex-wrap line breaking in the solver, and a declarative-DSL flex node —
> implemented in parallel and integrated together.

### Added
- **Flex line wrapping — `unigui::layout::SolveFlexWrap`** (`core/flex_layout.h`).
  Greedily breaks items into lines along the main axis (CSS `flex-wrap: wrap`),
  resolves each line independently with `SolveFlex`, and stacks the lines on the
  cross axis — each line offset by the **sum of the preceding lines' heights**
  (a caller-supplied uniform `lineHeight`, or the per-line tallest `crossSize`
  when auto). Additive; `SolveFlex` is unchanged. 5 new tests.
- **Cross-axis alignment in `Layout::FlexRow`.** `FlexRowOptions::align`
  (`layout::FlexAlign` — Start/Center/End/Stretch) positions each child vertically
  within the row via the solver's `crossOffset`/`crossSize`. The default (Start,
  no per-child `crossSize`) preserves the prior uniform-height layout. The row now
  reserves its footprint with an invisible item so aligned (shorter) children
  can't under-extend the window bounds. 3 new tests.
- **DSL `Flex` node.** `dsl::Flex({...})` describes a horizontal flex row in the
  declarative DSL, rendered through `Layout::FlexRow`; children share the width by
  their flex-grow weight (default equal split), with an optional per-child
  `weights` overload plus `gap` and `justify` (`dsl::FlexJustify`). Children render
  through the same per-node dispatch, so any node nests inside a flex row. 3 new
  tests.

## [3.12.0] - 2026-06-26

### Added
- **`Layout::FlexRow` — an ImGui flex-row container.** Lays children out in a
  horizontal flex line by applying `unigui::layout::SolveFlex`: each `FlexChild`
  pairs a `FlexItem` (basis/grow/shrink/min/max) with a render callback drawn
  inside a child region of its resolved width. Configured via a designated-init
  `FlexRowOptions{width, height, gap, justify}`; the per-row `id` scopes the
  child-region IDs so multiple rows coexist in one window. Hardened against the
  edges an adversarial review surfaced: a child the solver collapses to ~0 width
  is omitted (rather than tripping ImGui's "width 0 = fill remaining" rule and
  ballooning over its neighbours), a no-room / NaN container renders nothing, and
  `gap`/`height` are clamped non-negative. Main-axis only for now (cross-axis
  `align` lands in a later pass). 8 headless tests — turns the flex solver into a
  usable layout widget.

## [3.11.0] - 2026-06-26

### Added
- **Cross-axis alignment for the flex solver.** `SolveFlex` now resolves each
  item's cross-axis offset + size per `FlexParams::align` (new `FlexAlign`:
  Start/Center/End/Stretch), driven by `FlexItem::crossSize` and a container
  `FlexParams::crossSize`. Together with the main-axis pass this makes
  `SolveFlex` a complete 2D flex-line solver (line wrapping still to come). 4 new
  tests. Additive — existing main-axis usage is unchanged.

### Fixed
- **`find_package(unigui)` install on a clean prefix.** `unigui_export.h` was
  installed to an absolute `/unigui` because `GNUInstallDirs` was included *after*
  `add_subdirectory(src)`, leaving `CMAKE_INSTALL_INCLUDEDIR` empty in `src/`'s
  scope when its `install()` rules were captured at configure time. Hoisted
  `GNUInstallDirs` above the subdirectory so the destination resolves to
  `${prefix}/include/unigui`. Caught by the new `install-consume` CI job — it
  passed locally only because the build directory had the variable cached.

## [3.10.0] - 2026-06-25

> Horizon-5 layout: a CSS-flexbox-style main-axis solver — the computational
> foundation of the constraint/flex layout system.

### Added
- **`unigui::layout::SolveFlex`** (`core/flex_layout.h`) — a header-only, pure
  (no-ImGui) flexbox main-axis solver. Given a container length and a list of
  `FlexItem`s (flex-basis + grow/shrink + min/max clamps), it resolves each item's
  main-axis size and offset, with proper freeze-and-redistribute handling of the
  min/max clamps, full `justify-content` (Start/End/Center/SpaceBetween/
  SpaceAround/SpaceEvenly), and inter-item gaps. This is the computational core the
  widget-facing flex container will build on (cross-axis alignment and wrapping
  land in later passes). Exposed via the umbrella `<unigui/unigui.h>`. 13 new
  headless tests.

## [3.9.2] - 2026-06-25

> Performance: the row-vector `Table` widget is now virtualized.

### Changed
- **`Table` virtualizes its rows with `ImGuiListClipper`.** Previously every row
  was laid out and drawn each frame (per-cell text formatting, measuring, and
  custom clip-rect drawing), so a large table cost O(total rows) per frame. It now
  processes only the rows inside the scroll viewport, so a 100k-row `Table`
  renders in time bounded by the visible window — joining `DataTable` and
  `VirtualList`. Assumes uniform row heights (single-line cells or frame-height
  custom renderers), which is how `Table` lays its cells out.

### Added
- **`Table` 100k-row steady-state render benchmark** (`tests/bench/bench_test.cc`)
  with a per-frame budget enforced by the Release CI jobs — locks in the
  virtualized cost and catches any regression that scales with total row count.

## [3.9.1] - 2026-06-25

> Completes the Horizon-5 reactive workstream: the trading models now plug into
> the reactive layer.

### Added
- **Trading value types are reactive-ready.** `Quote`, `Position`, `Order`, and
  `Trade` (`trading/quote.h`) gained defaulted value equality (`operator==`), so
  they drop straight into the reactive layer: `Observable<Quote>` updates are
  change-detected, and `Computed<...>` can derive live metrics from them
  (`Computed<double>{[](const Quote& q){ return q.Mid(); }, quoteObs}`,
  `ChangePct`, `Spread`, …) and feed a sink/widget via `Bind`. No feed logic is
  added to the models — they stay pure data; equality is the only wiring needed.
  5 new headless tests (`tests/trading/reactive_test.cc`).

## [3.9.0] - 2026-06-25

> Horizon-5 reactive layer: derived `Computed<T>` values and first-class data
> binding on the retained widgets.

### Added
- **`Computed<T>` — derived / recomputing observables** (`core/observable.h`). A
  read-only observable whose value is recomputed from one or more source
  observables (or other `Computed`s) whenever any of them changes, notifying its
  own subscribers change-detected. It composes (a `Computed` can be a source for
  another `Computed` or for `Bind`) and supports N-ary, heterogeneous, and even
  zero-source derivations. **Lifetime-safe by design:** it caches each source's
  latest value and computes from the cache, so a source destroyed before the
  `Computed` never dangles — it simply stops contributing. Propagation is
  *eventually consistent* (a multi-path "diamond" graph may briefly observe an
  intermediate value and notify more than once before settling); derive from leaf
  observables for glitch-free results.
- **Widget data binding.** `ValueWidget<T>::BindValue(Observable<T>&, twoWay = true)`
  gives every value widget (CheckBox, Slider, SpinBox, Drag*, Input*, LineEdit,
  PasswordInput, ToggleSwitch, …) two-way binding: the widget adopts the source on
  bind, model changes flow to the widget, and user edits flow back. Buffer-backed
  inputs (LineEdit/PasswordInput) override a new protected `ApplyBoundValue` hook
  so their input buffer stays synced. `Label::BindText` adds one-way text binding.
  The widget owns the subscription (auto-detaches on destruction) and a lifetime
  guard prevents a stale push-back if the source is destroyed first; a two-way
  binding provably can't feed back into a loop (the change-detecting
  `Observable::Set` plus a non-notifying apply path break it).
- **`Observable<T>::Lifetime()`** — a `weak_ptr` token that expires when the
  observable is destroyed (backs the binding's push-back guard).
- 23 new headless tests covering `Computed` derivation / recompute / chaining /
  diamonds / heterogeneous sources / source-death safety, and widget binding
  (two-way, rebind, buffer-sync, no-loop, lifetime safety).

## [3.8.13] - 2026-06-25

> A real drag-and-drop library bug + three showcase interaction fixes surfaced by
> running the demo: cascading combo, empty chart, and a globally-dead Ctrl+S.

### Fixed
- **`AcceptDragDrop<T>` never delivered the dropped value** (`include/unigui/widgets/dragdrop.h`).
  It read `payload->Data` *after* `ImGui::EndDragDropTarget()`, but on the delivery
  frame `EndDragDropTarget()` calls `ClearDragDrop()`, which frees/zeros ImGui's
  payload buffer that `payload->Data` points into — so the wrapper returned a
  nulled/dangling pointer and a completed drop left the receiver at its sentinel.
  The value is now copied into stable storage **before** `EndDragDropTarget()` (with
  a `DataSize == sizeof(T)` guard). Signature unchanged. New regression test
  `tests/widgets/dragdrop_test.cc` scripts a full press→drag→release gesture and
  asserts the value arrives (it returned `-1` before this fix).
- **Showcase: CascadingCombo wasn't cascading.** The demo wired two independent
  fixed lists, so changing the Province never changed the City options. It now
  registers `SetOnChanged` (init-once) to relink the City list via `SetOptions`
  whenever the Province changes — the widget already supported this; the demo
  simply never used it.
- **Showcase: TimeSeriesChart rendered empty.** The demo appended one point per
  frame to an unseeded series, so on first view there was nothing to draw. It now
  seeds ~200 historical mock points once (with strictly-positive, monotonic
  timestamps so `AppendPoint`'s `timestamp < 0` frame-counter fallback can't
  collapse them onto `x = 0`) and keeps appending a live point per frame.
- **Showcase: Ctrl+S did nothing.** `ShortcutManager::Process()` was called inside
  the Utilities tab body, which only runs while that tab is active, so the chord
  was almost never polled. The manager is now file-scope and `Process()` runs every
  frame from the main render callback, so the shortcut fires on any tab.

### Docs
- **`docs/WIDGET_EXAMPLES.md` refreshed against the real headers.** An audit of all
  95 snippets found **65 were stale** — they showed old pointer-binding constructors
  (`make_shared<Slider>("sl", &v, …)` → the value-owning `(name, label, value, min,
  max)`), renamed/removed methods (`SetChild` → `SetContentCallback`, `SetUsage` →
  `SetRatio`, `AddColumn` → columns-in-ctor, `SetLeft/SetRight` →
  `SetContentA/SetContentB`), non-existent enums (`StatusLamp::State::Ok` →
  `Running`; `AlertBar::Severity` removed), and `void`-returning setters wrongly
  chained off `.Render()`. Every snippet now matches the current API; the entry
  count is corrected from 93 to 95 (adds `CommandPalette`, `FileDialog`).
  Content-callback snippets now draw with `unigui::im::*` (e.g. `im::Text`) instead
  of raw `ImGui::`, modelling the wrapper — only the handful of calls with no `im::`
  equivalent (style-var push/pop, `IsKeyChordPressed`) stay raw, and are commented
  to say so.

### Added
- **`examples/unigui_showcase`** — a comprehensive, runnable demo that exercises
  **all 95 widgets** (plus the `PnlText`/`TagList` immediate helpers) across **10
  category tabs** (Buttons · Inputs · Text & Pickers · Display · Indicators · Data ·
  Layout · Charts & Trading · Overlays & Dialogs · Utilities), the full
  `unigui::im` immediate layer, the `WindowScope`/`TabBarScope` RAII guards, and
  live theme/surface switching. Written entirely against the UniGUI public API —
  **zero raw `ImGui::` calls** — to demonstrate the wrapper end to end.
  Headless-friendly (`--frames N`).

## [3.8.12] - 2026-06-25

> CI clang-tidy job + a latent trading-module compile break surfaced by it.

### Fixed
- **Trading module failed to compile under Dear ImGui 1.92.** `DepthLadder` and
  `OrderTicket` used `ImGuiChildFlags_Border`, which 1.92 renamed to
  `ImGuiChildFlags_Borders`. The module is off by default (`UNIGUI_MODULE_TRADING`)
  so neither the default build nor MSVC caught it — clang-tidy did. Renamed both
  call sites; verified by building with `-DUNIGUI_MODULE_TRADING=ON`.
- **clang-tidy CI job (advisory) was failing on spurious errors.** The step linted
  *every* `src/*.cc`, including sources whose module/backend is off in the Linux
  config (config/network/ipc/dx11/dx12/sdl3/vulkan/trading) and a generated font
  header that hadn't been built — producing `file not found` `clang-diagnostic-error`s
  that turned the job red. It now builds first (so generated headers exist) and
  lints only the TUs actually present in `compile_commands.json`, so the job
  reflects real findings on the compiled surface. (Style findings remain advisory
  per `WarningsAsErrors: ''`.)

## [3.8.11] - 2026-06-25

> Closes the last open item from the Round-2 safety-hardening review.

### Added
- **`core/path_util.h` — `PathFromUtf8(std::string_view)`** — construct a
  `std::filesystem::path` from a UTF-8 string portably. A plain `path(std::string)`
  decodes via the platform's native narrow encoding (the **ANSI code page** on
  Windows, not UTF-8), mangling non-ASCII paths; this routes the bytes through
  `std::u8string` to force UTF-8 on every platform (the non-deprecated C++20
  replacement for `std::filesystem::u8path`). Header-only, unit-tested.

### Fixed
- **`Settings` auto-save mangled non-ASCII UTF-8 paths on Windows.**
  `EnableAutoSave(const std::string&)` stores a path that `Shutdown()` passed to
  `Save()` via an implicit `std::string`→`path` conversion — i.e. ANSI-decoded on
  Windows, so a UTF-8 auto-save path with non-ASCII characters was written to the
  wrong location on a non-GBK system. `Shutdown()` now converts via
  `PathFromUtf8`. Also replaced the previous non-ASCII path test (which built the
  directory via an ANSI-decoded `std::string`, masking the bug by being
  self-consistently wrong) with one that creates a correctly-named Unicode
  directory and a real `EnableAutoSave`→`Shutdown`→`Load` round-trip.

## [3.8.10] - 2026-06-25

> **Cross-platform CI hardening — Linux GCC `-Werror`, macOS libc++, headless Windows.**
> The MSVC `/W4 /WX` gate (3.8.4) can't see these: GCC diagnoses unused
> parameters in *uninstantiated* template methods (MSVC doesn't), and libc++
> deletes floating-point `from_chars`.

### Fixed
- **macOS / libc++ build** — `Table` numeric-cell parsing used
  `std::from_chars` with a `double`, which libc++ **deletes** (hard error).
  Switched to portable `std::strtod` (non-throwing, end-pointer validated),
  matching the project's existing `strutil` idiom. Behaviour preserved.
- **Linux GCC `-Werror` build** — `EditableDataGrid`'s four cell-renderer
  lambdas captured `this` implicitly via `[=]` (deprecated in C++20,
  `-Wdeprecated`); now `[=, this]`.
- **Linux GCC `-Werror` build** — `DataTable::SetCellEditable(col, editable)`
  ignored its `editable` argument (always inserted the column), which both
  tripped `-Wunused-parameter` and was a latent bug. It now honours the flag
  (insert when `true`, erase when `false`).
- **Headless Windows CI** — the GLFW/OpenGL `BackendTest` suite hard-`ASSERT`ed
  on GLFW init + a GL 3.3 core context; on GPU-less runners (generic GL 1.1
  driver) all 8 tests failed. They now probe for a real GL 3.3 context in
  `SetUp` and `GTEST_SKIP` the suite when unavailable (mirrors
  `render_integration_test.cc`), so the suite runs on real GPUs and skips
  cleanly in headless CI.

### Removed
- Dead private fields `MultiSplitter::dragIndex_` and `Form::submitted_`
  (never read or written; flagged by Clang `-Wunused-private-field`).

## [3.8.9] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 10 (data density at scale): the two deferred chart items.**

### Added
- **`TimeSeriesChart::UpsertPoint(seriesId, value, timestamp)`** — update-or-append
  keyed by timestamp: if a point already exists at exactly `timestamp` its value is
  replaced in place, otherwise the point is appended (with the usual sliding-window
  trim). This is the live forming-bar/tick pattern — the latest sample updates
  repeatedly at a fixed timestamp before a new one starts — without growing the
  series by one point per frame. (Deferred item from `docs/jzdz-fit-plan.md`.)
- **`TimeSeriesChart::SetSessionAxis(SessionAxis)`** — convenience that installs the
  matching inverse tick formatter so intraday X labels read as wall-clock `HH:MM`
  while the axis stays gap-free (lunch/overnight breaks collapsed). Plot
  `SessionAxis::ToAxis(secOfDay)` coordinates and the labels follow. Wires the
  existing pure `SessionAxis` transform into the chart's X formatter — the second
  deferred chart item. No new per-instance state (the axis is captured by value in
  the formatter).

## [3.8.8] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 6 (HiDPI content-scale).**

### Added
- **`core/dpi.h` — HiDPI content-scale helpers** (pure, header-only). Dear ImGui
  ≥1.92 re-rasterises fonts from `ImGuiStyle::FontScaleDpi` (see `SetContentScale`),
  but platforms report DPI in inconsistent, often fractional forms (GLFW hands back
  `1.4583` on a "150%" monitor) which gives soft glyphs. `DpiToScale(dpi, base=96)`
  maps a raw DPI to a baseline-relative scale; `NormalizeContentScale(raw, min, max,
  step)` clamps and snaps a raw scale to a crisp step (so `1.4583 → 1.5`), guarding
  NaN / non-positive / reversed-bounds inputs. Fully unit-tested.
- **`App::SetContentScaleFromMonitor(rawScale, snap = true)`** — apply a content
  scale taken from a raw platform value (e.g. `glfwGetWindowContentScale`), running
  it through `NormalizeContentScale` first so fractional monitor scales render
  sharply. The crisp-CJK-on-4K path the trading client needs.

## [3.8.7] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 11 (distribution: consumable packaging).**

### Fixed
- **`find_package(unigui)` now works for downstream consumers.** The installed
  `unigui-config.cmake` previously did not re-resolve the public dependencies
  referenced by the exported target's `INTERFACE_LINK_LIBRARIES`, so any consumer
  doing `find_package(unigui CONFIG)` failed at generate time with *"target links
  to imgui::imgui that does not exist"*. The config now emits `find_dependency()`
  calls for every public dependency, generated to mirror `src/CMakeLists.txt`'s
  `find_package()` set exactly (imgui / glfw3 / glad / Freetype / implot / spdlog,
  plus the module + Vulkan/SDL3 deps when those options are on). Verified end to
  end: a standalone project installs, `find_package`-es, links `unigui::unigui`,
  builds, and runs.
- **Package version no longer drifts.** `project(VERSION …)` was hard-coded at
  `3.8.2` while the library had moved to `3.8.6`, so the installed
  `unigui-config-version.cmake` reported the wrong version (breaking consumer
  version checks). The project version is now derived from
  `include/unigui/core/version.h` at configure time — a single source of truth.

### Changed
- **vcpkg port (`ports/unigui/vcpkg.json`) brought up to date** — version
  `3.5.0 → 3.8.7`, description fixed (82 → 95 widgets), and the stale
  `imgui-node-editor` / test-only `gtest` dependencies removed (they are not
  needed to consume the library).

## [3.8.6] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 11 (distribution: add-on widgets).**

### Added
- **`FileDialog` widget** (widget #95) — an in-ImGui file / folder picker, since
  Dear ImGui ships no native dialog. Three modes (`OpenFile`, `SaveFile`,
  `SelectFolder`) rendered as a themed modal: navigate directories (enter sub-dir,
  go up), extension filtering, a filename field for saves, and confirm/cancel
  callbacks. Navigation and path resolution (`SetDirectory`/`NavigateInto`/
  `NavigateUp`/`Entries`/`ResolvedPath`/`Confirm`) are exposed as plain methods so
  the behaviour is unit-tested against a real temp directory without a GL context.
  Pairs with `BasketTicket`'s `SetOnImportRequested` host hook.
- **`detail::ListDirectory` / `detail::ExtensionMatches`** — the reusable,
  non-throwing filesystem-listing core behind the dialog (directories-first sort,
  case-insensitive extension filter, dotfile gating; uses `std::error_code`
  overloads so a permission error or missing path returns `false` rather than
  throwing).

## [3.8.5] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 11 (distribution: add-on widgets).**

### Added
- **`CommandPalette` widget** (widget #94) — a VS-Code-style (Ctrl+P) fuzzy-searchable
  command launcher rendered as a centred modal popup. Register commands once
  (`AddCommand(id, title, action)`, with optional `category`/`shortcut`/`enabled`);
  the palette filters and ranks them as the user types, runs the chosen command on
  Enter/click, and closes. Up/Down navigate, Esc dismisses. The query → ranked-results
  → execute pipeline (`SetQuery`/`Matches`/`Execute`) is exposed directly so the
  ordering behaviour is testable without a GL context.
- **`detail::FuzzyMatch(pattern, text, &score)`** — the reusable subsequence fuzzy
  matcher behind the palette: case-insensitive, with relevance scoring (prefix /
  word-boundary / contiguous-run bonuses, leading-gap penalty). Pure and
  allocation-free; usable anywhere a "type to filter" list is wanted.

## [3.8.4] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 7 (quality gates).**

### Added
- **MSVC warnings-as-errors gate** — new `windows-msvc-debug-werror` CMake preset
  (`/W4 /WX`) and a matching `windows-werror` CI job, mirroring the existing GCC
  `linux-werror` gate on the other major compiler. The whole tree — library, tests,
  and examples — is now verified warning-clean under `/WX` on both MSVC and GCC.

### Changed
- **MSVC builds define `_CRT_SECURE_NO_WARNINGS`** (PRIVATE, in `unigui_set_warnings`)
  so MSVC's C4996 nag on portable Standard C functions (`fopen`, `sscanf`, `_wfopen`)
  no longer fires for our own translation units. PRIVATE — it never leaks to
  downstream consumers, who keep their own warning policy.

### Fixed
- **`DataTable<T>` inline-edit copy** now uses `std::string::copy` instead of the
  deprecated `strncpy`, so consumer builds that include `datatable.h` under `/W4`
  stay warning-clean without relying on `_CRT_SECURE_NO_WARNINGS`. Behaviour is
  unchanged (bounded copy + explicit NUL terminate).
- **Unreferenced-parameter warning** in `table_test.cc` (a checkbox getter lambda).

## [3.8.3] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 9 (accessibility foundation).**

### Added
- **Accessibility seam** (`core/accessibility.h`, `namespace unigui::a11y`) — Dear ImGui
  draws its own widgets, so assistive technology has no native a11y tree to read. This
  module is the missing seam: a small semantic descriptor (`Node{name, description, value,
  Role}`) that widgets report when focused, plus a process-wide focus tracker that fires a
  change event (`SetOnFocusChanged`). A platform bridge (e.g. Windows UI Automation) can
  subscribe and announce the focused element; the model + event stream here is pure and
  headless-testable. Disabled by default (`SetEnabled(false)`) for zero per-frame cost.
- **`Widget::AnnounceAccessible(role, value="")`** — a base-class helper that reports the
  widget to the a11y tracker using its accessible name (falling back to the widget id) and
  description. A no-op unless a11y is enabled. `Button` is the first adopter (announces on
  `ImGui::IsItemFocused()`); other widgets can opt in the same one-line way.

## [3.8.2] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 8 (app-shell parity: layout persistence).**

### Added
- **`MultiSplitter::SerializeLayout()` / `RestoreLayout(s)`** — persist and restore
  split ratios as a compact string (e.g. `"0.28,0.44,0.28"`). `RestoreLayout`
  applies only when the value count matches the current panel count (so a
  stale/foreign layout is ignored) and is non-throwing on malformed input. Fills
  the gap where `imgui.ini` doesn't capture `MultiSplitter` internal ratios.
- **`LayoutStore`** (`core/layout_store.h`) — a tiny named string-value store for
  persisting layout/preference state across runs (split layouts, theme-preset
  name, locale tag, content scale). Plain `name=value` lines; loading skips
  malformed lines (no throw); a missing file is a benign first-run `false`.
  Header-only, unit-tested against a temp file.

## [3.8.1] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 10 (data density at scale).**

### Added
- **`DataTable<T>` row-accessor data source** — `SetDataSource(count, index→const T&)`
  alongside the `const std::vector<T>*` source, so models exposing `Count()`/`GetAt(i)`
  feed the table without copying into a temporary vector every frame.
- **`DataTable<T>::SetCellSignColor(col, valueOf)`** — financial sign colouring: a
  column's text is coloured via the active theme `Up`/`Down` tokens (CN red-up by
  default) by the sign of a per-row value; flat (== 0) stays default. Replaces
  hand-written P&L `SetCellColor` lambdas.
- **`TimeSeriesChart::SetMaxRenderPoints(n)`** — caps stored/plotted points per
  series; oversized series are LTTB-decimated (shape-preserving) to ~`n`, so a
  100k-tick series renders fast without visual loss (uses `core/decimate.h`).
  Adds `GetSeriesPointCount(id)`.

## [3.8.0] - 2026-06-24

> **Roadmap series 3.8.x — Horizon 6 (platform reach & HiDPI), kickoff.** See `ROADMAP.md`.

### Added
- **Toolchain modernization → Dear ImGui 1.92.8 + ImPlot 1.0** (`vcpkg.json` overrides). Picks up
  the 1.92 **dynamic font system** (on-demand glyphs, `style.FontScaleDpi`) — the foundation for
  crisp CJK + HiDPI text — and ImPlot 1.0's `ImPlotSpec` per-call styling. The unused
  `imgui-node-editor` dependency was dropped (it has no imgui-1.92-compatible version in vcpkg and
  was never linked). The whole tree builds + tests clean against the new deps (1000/1000).
- **HiDPI content scale (Horizon 6)** — `App::SetContentScale(float)` / `GetContentScale()` drive
  `ImGuiStyle::FontScaleDpi` so fonts re-rasterise crisply at any scale via the dynamic font system
  (no glyph-range pre-building), and `AppConfig::dpiScaleFonts` opts into Dear ImGui's automatic
  per-monitor font DPI scaling (`io.ConfigDpiScaleFonts`).
- **Series decimation core** — `core/decimate.h` (`LttbIndices`/`Decimate` and `MinMaxBuckets`):
  pure, header-only downsampling for data-dense charts. LTTB preserves visual shape (peaks/troughs);
  MinMaxBuckets guarantees extremes survive (for volatile price/OHLC data). Unit-tested; the
  reusable basis for chart render-point capping (Horizon 10). Addresses ImPlot's large-series
  slowdown without changing the stored data.
- **`im::ButtonVariant::Warning`** — an amber (`0.85, 0.55, 0.13`) button color variant for actions
  that are neither destructive (`Danger`) nor a fresh start (`Success`): e.g. a pod whose trigger
  count is exhausted offering a "Done / re-draft" action. `VariantColor` maps it; all `im::Button`
  call sites accept it.
- **`TimeSeriesChart::SetYAxisMinSpan(double)`** — floors the auto-fit Y-axis height. When the data
  inside the visible X window spans less than the given value, the Y axis is held at exactly that
  span (centered on the data) instead of zooming tighter, so a near-flat series no longer renders
  its micro-noise as full-height swings. `0` (default) disables the floor; no effect when auto-fit
  is off. The visible X window is read via `GetPlotLimits` and cached for the next frame's fit.

### Changed
- **Chart legends now default to the top-right corner with a semi-transparent panel.**
  `TimeSeriesChart` and `CandlestickChart` previously seated the in-plot legend at the top-left
  (`ImPlotLocation_NorthWest`) over an opaque background. They now default to the top-right
  (`ImPlotLocation_NorthEast`) and push a translucent `ImPlotCol_LegendBg` (a slightly darker,
  60%-opacity tint of the chart background) so the legend floats over the series without hiding
  them. Legends remain draggable out of the box — the user can still click-drag the box to any
  corner.

### Fixed
- **Chart code broken against implot 1.0.** `CandlestickChart::DrawVolumePanel` used the obsoleted
  `ImPlot::SetNextFillStyle`, and `TimeSeriesChart::Render` used the removed `ImPlotCol_Line`
  push/pop — neither compiles after the implot 1.0 bump. Both now pass per-call style via
  `ImPlotSpec` (`FillColor` / `LineColor`) to the `PlotBars` / `PlotInfLines` calls.
- `im::Combo` (the `std::vector<std::string>` overload): returned an **empty popup** when the item
  list started with an empty string `""`. It packed items into a `\0`-separated buffer for
  `ImGui::Combo(const char* items_separated_by_zeros)`, whose item counter
  (`while (*p) p += strlen(p)+1;`) stops on the leading `\0` and reports **zero** items — so any
  caller that prepends `""` as a blank/clear option got a blank dropdown. Now drives the popup with
  `BeginCombo`/`Selectable`/`EndCombo`, which renders empty/blank items correctly and stays stable
  across ImGui versions.
- **Input text caret nearly invisible at fractional DPI.** ImGui draws the caret as a 1px line
  scaled by `(int)style._MainScale`, so at e.g. 1.5× DPI it truncates to 1px (imgui #7031). Added
  `im::DrawActiveInputCaret()`, which overlays a thicker, font-scaled caret (matching ImGui's blink)
  at the active input's IME position; the `im::InputText*` helpers and the `LineEdit` widget now
  call it automatically.

## [3.7.0] - 2026-06-15

### Added
- **Trading-client fit, Horizon 1** (driven by the `jzdz_client_suite` audit — see `docs/jzdz-fit-plan.md`):
  - **Theme `Up`/`Down` semantic tokens + `Polarity`**: `theme::Semantic` gains `Up`/`Down`, resolved through a process-wide `theme::SetPolarity()` (`RedUp` = Chinese markets, the default; `GreenUp` = Western). `GetSemanticColor(Up/Down)` and `GetDirectionColor(value)` give correct rise/fall colours per market with no call-site change.
  - **`PnlText`** (`widgets/pnltext.h`): polarity-aware sign-coloured value text (`PnlText`/`StatusText`/`GradedText`) — centralises the most-repeated `TextColored(v>=0?up:down, fmt(v))` idiom. Pure `PnlRole`/`GradedRole` mappings are unit-tested without a frame. Named `PnlText` to avoid colliding with the existing `ValueWidget`.
  - **`TagList`** (`widgets/taglist.h`): inline, wrapping 0..N coloured chip container (semantic role or explicit RGBA) for limit-up/down / status flags — replaces the hand-rolled `SameLine` + per-tag `PushStyleColor` idiom.
  - **`WeakInvokeOnMainThread`** + `LifetimeToken` (`core/main_thread.h`): teardown-safe cross-thread posting; a queued task is silently dropped once its owner token is destroyed — replacing the hand-rolled `shared_ptr<atomic<bool>> alive_` guard.
- **Trading-client fit, Horizon 2 — the editable-grid lever**:
  - **`EditableDataGrid<T>`** (`widgets/editabledatagrid.h`): a `DataTable<T>` with typed per-column cell editors (`SetComboColumn`/`SetIntColumn`/`SetFloatColumn`/`SetButtonColumn`) and a `SetRowReadOnly` predicate that collapses a row's editors to static text ("frozen-when-running"). Editors render through the **stateless `unigui::im` layer** inside the table's per-row `PushID`, so there is **no per-row widget cache** — directly retiring the hand-rolled `static std::map<int,Widget>` grids the audit found across FSA/FAT/CO/AA. Presentation-only (values flow via getter/on-change callbacks).
  - **`DataTable<T>::SetCellRenderer(col, fn)`**: render an arbitrary cell (incl. `im::` editors) inside the row's `PushID` — the hook `EditableDataGrid` is built on; usable directly too.
  - **`WidgetPool<T>`** (`core/widget_pool.h`): keyed cache of retained widget instances for cells that genuinely need stateful widgets (animated `RiskBar`/`Gauge`). Key by stable id; `BeginFrame`/`EndFrame` evicts widgets for rows that disappeared (no leak, no index-reuse bug). Header-only, ImGui-free, unit-tested.
- **Trading-client fit, Horizon 3 (started)**:
  - **`MetricCard`** (`widgets/metriccard.h`): bordered KPI/status tile — optional accent rail, header row (status dot + accent title + right-aligned action slot), and a value/delta/subtext body or a custom draw callback. Delta is sign-coloured via the active `Up`/`Down` polarity. Replaces the hand-rolled "BeginChild + accent bar + status dot + measured button cluster" pod/account card across AA/FAT/CO and the fund panels.
  - **`SessionAxis`** (`core/session_axis.h`): pure, header-only gap-collapsing intraday time axis — maps wall-clock seconds-of-day onto a continuous session axis (lunch break / pre-post gaps collapsed) and back, with an `HH:MM` tick formatter. `SessionAxis::AShareFutures()` ships the CN day session. Pairs with `TimeSeriesChart`'s X formatter so charts have no dead time. Fully unit-tested (round-trip, gap, clamp, format).
  - **`ToggleButton`** (`widgets/togglebutton.h`): bistate action button (Start ⇄ Stop) with per-state label + semantic colour, an enabled-predicate + disabled tooltip, and an on-toggle callback — the run/stop control in all four strategies. Distinct from the boolean `ToggleSwitch`.
  - **`ButtonGroup`** (`widgets/buttongroup.h`): horizontal button cluster with Left/Right/Fill alignment, owning the "measure each button, right-align the cluster" math; composes inside `MetricCard`'s header action slot.
  - **`ConnectionStatusBar`** (`widgets/connection_status.h`): link-health strip composing `StatusLamp` + `Sparkline` with an adaptive, colour-graded latency readout, FPS, and a reconnect countdown — RTT averaging / reconnect FSM stay in the caller.
  - **`format::Latency(µs)`** (`core/format_num.h`): adaptive `µs`/`ms`/`s` latency string — centralises the duplicated connection-readout formatting.
  - **`GroupedRiskTree`** (`widgets/groupedrisktree.h`): a hierarchical account/group risk view built on `TreeView` — each node shows a utilisation bar coloured by warn/danger thresholds, and parent rows roll their children up via `Worst`/`Mean`/`Sum` (the static `ComputeRatio` rollup is pure and unit-tested). Caller supplies leaf ratios + labels; no unit/scaling baked in.
  - **`BasketTicket<T>`** (`widgets/basketticket.h`): an editable basket / program-trading grid — a toolbar (Add / Remove / Import / Submit) over an owned `EditableDataGrid<T>`, with validator-driven invalid-row highlighting, **deferred** row removal (no mid-iteration mutation), and Submit gated on all-valid. Host-driven by design: the embedder owns CSV/XLSX parsing + the file dialog (Import fires a callback; the host calls `SetRows`), and order routing stays in the controller (`onSubmit` hands back the rows). Composes the `EditableDataGrid` lever.
- New headless test files/cases (`pnltext`, `taglist`, `main_thread`, `widget_pool`, `editabledatagrid`, `metriccard`, `session_axis`, plus enhancement cases on combobox/statuslamp/confirmdialog/multisplitter/table).

### Changed
- **Widget enhancements for data-dense/trading UIs**:
  - `DataTable<T>`: `SetEmptyText()` (empty-state row) and `SetCellCheckboxValue(col, get, set)` — a non-UB checkbox column driven by a get/set pair instead of a `bool*` (no `reinterpret_cast` over `uint8_t` flag storage).
  - `ComboBox`: `SetPlaceholder()` + `SetAllowEmpty()` — optional dropdowns now pass the real item list and read back a real index or `-1`, instead of prepending an empty sentinel and doing `+1/-1` arithmetic. (`GetSelectedValue()` is now bounds-safe for an empty selection.)
  - `StatusLamp::SetCaption()` — render the lamp with an adjacent label in one widget.
  - `ConfirmDialog::Open(onConfirm)` / `SetOnConfirm()` — fire a callback on confirm, retiring the parallel pending-action-id state machines.
  - `MultiSplitter::Configure(defs)` (idempotent by panel count) + per-panel `minPx` — removes the `static bool` first-frame guards and ratio-only sizing workarounds.
  - `TimeSeriesChart::AppendSample(id, time, value)` — time-first overload that avoids transposing `AppendPoint`'s value/timestamp arguments.
  - **Trading blotters honour up/down `Polarity`** (`trading/blotters.h`): `DeltaColor`/`SideColor` and the `MakePositionsBlotter`/`MakeOrdersBlotter`/`MakeTradesTape`/`MakeWatchlist` factories take an optional `theme::Polarity` (default `GreenUp` — unchanged Western behaviour; `RedUp` flips to the CN convention where a rise/Buy is red). Verified with `UNIGUI_MODULE_TRADING=ON`.

## [3.6.0] - 2026-06-15

### Added
- **Four new dashboard/data widgets** (extended widgets, `UNIGUI_MODULE_WIDGETS`): all `FluentWidget`-based, `PushID`-safe, draw-list rendered, and headless-tested (40 new test cases).
  - **`Sparkline`** (`widgets/sparkline.h`): compact axis-less trend chart (Line/Area/Bar) for inline use in tables, watchlists, and KPI cards — auto-ranging or fixed range, optional trend colouring (green up / red down), last-point dot, and a rolling `PushValue()`/`SetMaxPoints()` streaming mode. No ImPlot dependency. Closes the roadmap's deferred "in-cell mini sparkline" follow-up (standalone form).
  - **`Gauge`** (`widgets/gauge.h`): circular/radial progress dial — configurable value range, radius/thickness, arc sweep (360° ring or open-bottom speedometer), theme-accent fill, and a centre percent/custom label. Complements the linear `ProgressBar` for dashboards.
  - **`SegmentedControl`** (`widgets/segmentedcontrol.h`): compact single-select button group sharing one rounded frame (the iOS-style `1D / 1W / 1M` selector) with accent highlight, optional fill-width, and an `onChange` callback.
  - **`PriceTicker`** (`widgets/priceticker.h`): horizontally scrolling symbol/price/Δ marquee with green/red ▲/▼ tinting, adjustable speed, and pause — the classic trading header strip. Closes the roadmap's deferred "PriceTicker marquee" follow-up.
- **`unigui::im` A6 — practical-surface completion + coverage tracking** (Horizon 2): the immediate layer now wraps the remaining commonly-needed ImGui controls — `TextUnformatted`/`TextLink`/`TextLinkOpenURL`, tooltips (`BeginTooltip`/`EndTooltip`/`SetTooltip`/`BeginItemTooltip`/`SetItemTooltip`), `BeginDisabled`/`EndDisabled`, low-level combo (`BeginCombo`/`EndCombo`), list box (`BeginListBox`/`EndListBox`), `Selectable` (value + `bool*` overloads), trees & headers (`TreeNode`/`TreeNodeEx`/`TreePop`/`SetNextItemOpen`/`CollapsingHeader` ×2), tab bars (`BeginTabBar`/`EndTabBar`/`BeginTabItem`/`EndTabItem`), `ProgressBar`/`PlotLines`/`PlotHistogram`, color editors/pickers (`ColorEdit3/4`, `ColorPicker3/4`) and conversion (`ColorConvertRGBtoHSV`/`HSVtoRGB`/`Float4ToU32`/`U32ToFloat4`), window-state queries (`IsWindowAppearing`/`Collapsed`/`Focused`/`Hovered`), and misc utilities (`CalcTextSize`, `SetKeyboardFocusHere`, `GetTime`, `GetFrameCount`, `Set/GetMouseCursor`). `im` count 157 → **201 = 100% of ImGui's practical public surface**. Optional `const char*` params accept an empty `string_view` as ImGui's default (`nullptr`). 13 new headless test cases (`tests/im/im_test.cc`, 70 → 83).
- **Reactive data-binding foundation (Horizon 5)**: new header-only `core/observable.h` — `Observable<T>` wraps a value and notifies subscribers on change (change-detecting `Set`, unconditional `ForceSet`, in-place `Mutate`, `Subscribe`/`SubscribeAndFire`). Subscriptions are **RAII** (`Subscription`): they auto-unsubscribe on destruction and safely outlive the observable (shared registry + weak reference, no dangling). A `Bind(source, sink)` helper mirrors a value into any sink immediately and on every change. Move-only so observers are never silently aliased. Lets retained widgets/models update without manual `Set*` plumbing. 13 new tests (`tests/core/observable_test.cc`); exported via the umbrella `<unigui/unigui.h>`.
- **Internationalization — catalog upgrades (Horizon 5)**: `core/locale.h` grows from a flat table into a real translation catalog. `Tr()` now follows a **fallback chain** — current locale → its base language (`zh_CN` → `zh`) → the configured fallback locale (`SetFallback`, default `en_US`) → the key — so partially-translated locales no longer leak raw keys. New `Tr(key, args)` does **positional `{0}`/`{1}` substitution**, and `IsRTL()`/`IsRTL(locale)` detect right-to-left languages (ar/he/fa/ur) as the primitive for RTL-aware widgets (full layout mirroring tracked separately). 11 new tests (`tests/core/locale_test.cc`).
- **Theming-authoring tools complete (Horizon 5)**: new `examples/theme_editor` ties the authoring building blocks together — switch `ThemePreset`/`SurfaceStyle`/font/accent live, export & re-import the active palette as JSON (`ExportThemeJSON`/`ImportThemeJSON`), and pass `--css <file>` to hot-edit a stylesheet while the app runs. Runs headless via `--frames N`.
- **CSS hot-reload from disk (Horizon 5)**: `styling::Engine::LoadFile()` now remembers the loaded stylesheet, and the new `ReloadIfChanged()` re-parses it when its on-disk modification time changes (clearing the previous rules/vars first — single-stylesheet "edit the `.css` and see it update" dev workflow). Adds `Clear()` and `WatchedFile()` accessors. An app can call `ReloadIfChanged()` once per frame to live-edit styles without a restart. 5 new headless tests (`tests/styling/style_test.cc`).
- **Performance benchmarks (Horizon 4)**: new `tests/trading/bench_test.cc` exercises the header-only trading models under high update rates — `OrderBook` over 200k price deltas (with inside-market reads) and 5k full 100-level snapshot rebuilds, plus `OhlcSeries` folding 1M ticks into a rolling bar window and 1k per-frame ImPlot column extractions. Adds a `DataTable` virtual-scroll benchmark at **100k rows** to `tests/bench/bench_test.cc` that warms up the list clipper and asserts the steady-state per-frame cost stays bounded by the visible window (not the total row count). Each carries a generous regression floor (guards against order-of-magnitude slowdowns in the DOM/blotter/table streaming paths). Headless, always built.
- **`scripts/coverage_vs_imgui.py` — wrapper-coverage tracker**: parses `imgui.h` and `im/im.h` and reports the first-class-wrapped % of ImGui's *practical* surface, with a curated, documented exclude list (context/IO/backend/ini plumbing, docking & viewports, `va_list` `*V` overloads, generic `*Scalar` forms, the style/ID/font *stacks* — covered by `core/scope.h` RAII guards — and functions with a richer retained-mode widget equivalent). Supports `--list`, `--json`, and a `--threshold` hard-gate mode. Runs **advisory in `quality.yml`**, closing the deferred `coverage-vs-imgui` CI item and backing the "Wrapper coverage" success metric.
- **Trading toolkit — dashboard example (phase B6)**: new `examples/trading_dashboard` assembles the full toolkit (candlestick chart + volume, DOM ladder, order ticket, and the positions/orders/watchlist/time-&-sales blotters) into one screen, driven by small in-memory models fed a deterministic pseudo-random walk (synthetic feed + OMS). The order ticket's submit callback appends to the orders blotter and the tape. Builds when `UNIGUI_MODULE_WIDGETS` and `UNIGUI_MODULE_TRADING` are on; runs headless via `--frames N`. **Completes Horizon 3 — the trading-client toolkit (B0–B6).**
- **Trading toolkit — blotters / watchlist / tape (phase B5)**: new header-only `include/unigui/trading/blotters.h` (gated by `UNIGUI_MODULE_TRADING`) ships pre-built `DataTable<T>` factories bound to the thin row models — `MakePositionsBlotter` (`Position`), `MakeOrdersBlotter` (`Order`), `MakeTradesTape` (`Trade`, time & sales), and `MakeWatchlist` (`Quote`, quote board). Each wires columns, a financial cell formatter (reusing `core/format_num.h`), sign-aware cell colours (green/red + ▲/▼ delta arrows), and a pinned leading column. The per-row cell formatters (`PositionCell`/`OrderCell`/`TradeCell`/`QuoteCell`) and colour/format helpers (`DeltaColor`, `SideColor`, `WithArrow`, `FormatClock`) are **pure functions**, unit-tested without an ImGui frame. 13 new headless tests.
- **`DataTable` freeze-pane (pinned columns)**: new `DataTable<T>::SetFrozenColumns(n)` / `GetFrozenColumns()` keeps the first *n* columns visible while the rest scroll horizontally (enables `ImGuiTableFlags_ScrollX` and `TableSetupScrollFreeze`), the classic blotter "pin the key columns" behaviour. `n == 0` (default) leaves layout unchanged; composes with the sticky header. Backs the Horizon-3 blotters/DOM work (B3/B5).
- **Umbrella header exposes trading widgets**: `<unigui/unigui.h>` now includes the trading **widget** headers (`candlestick_chart.h`, `depth_ladder.h`, `order_ticket.h`, `blotters.h`) under `UNIGUI_HAS_TRADING`, not just the models — matching the documented "umbrella exposure" of the toolkit.
- **Trading toolkit — order ticket (phase B4)**: new `OrderTicket` widget (`include/unigui/trading/order_ticket.h`, `src/trading/order_ticket.cc`, gated by `UNIGUI_MODULE_TRADING`) — an order-entry form over a single editable `OrderDraft` (symbol, side, type, TIF, qty, limit/stop price). New `OrderType` (Market/Limit/Stop/StopLimit) and `TimeInForce` (Day/GTC/IOC/FOK) enums with `*Name()` helpers; `OrderDraft::NeedsPrice()/NeedsStop()` drive which fields are required and which inputs are enabled. `Validate()` is a **pure, headless-testable** check (symbol via `core/strutil` trim, qty > 0, optional max-qty cap, type-conditional price/stop) returning an `OrderValidation { ok, message }`; `Submit()` validates, snaps prices to the tick size (`format::TickAlign`), and fires the submit callback. UI adds a coloured Buy/Sell toggle, type/TIF combos, disabled-when-not-needed price/stop inputs, an inline validation message, a disabled-until-valid submit button, an optional confirmation modal (`SetConfirm`), and a Ctrl+Enter submit hotkey (reuses `ShortcutManager`). Presentation + thin draft model; library still builds/tests with `UNIGUI_MODULE_TRADING=OFF`. 19 new headless tests.
- **Trading toolkit — depth-of-market ladder (phase B3)**: new `DepthLadder` widget (`include/unigui/trading/depth_ladder.h`, `src/trading/depth_ladder.cc`, gated by `UNIGUI_MODULE_TRADING`) binds to a non-owning `OrderBook` and renders it as a classic vertical price ladder — asks highest→lowest on top, an optional spread/mid divider row, then bids highest→lowest — with per-level horizontal depth bars scaled to `OrderBook::MaxSize()`, aggregated size, and side-tinted price. Drawn via the window draw-list with a full-row hit target. Options: bid/ask colours, translucent depth-bar opacity, spread-row toggle, theme-aware background, configurable row height + size/price column widths, depth limiting, auto-/one-shot centring on the inside market (`SetAutoCenter` / `CenterOnMarket()`), a per-side **click-to-trade** callback (`SetOnLevelClick`), and a fluent `With*` API. Presentation-only and non-owning. Library still builds and passes tests with `UNIGUI_MODULE_TRADING=OFF` (widget + test gated out). 13 new headless tests.
- **Trading toolkit — candlestick / OHLC chart (phase B2)**: new `CandlestickChart` widget (`include/unigui/trading/candlestick_chart.h`, `src/trading/candlestick_chart.cc`, gated by `UNIGUI_MODULE_TRADING`) binds to a non-owning `OhlcSeries` and renders candlesticks (low→high wick + open↔close body, ≥1px so dojis stay visible) directly via the ImPlot plot draw-list with legend + auto-fit integration. Options: bull/bear colours, candle-width fraction, optional **volume sub-panel** (linked-X subplot with bull/bear-coloured bars), OHLCV crosshair hover tooltip (reuses `core/format_num.h`), theme-aware background, date/time X axis, and a fluent `With*` API. The reusable low-level `unigui::trading::PlotCandlesticks()` free function is exposed for callers that drive their own `BeginPlot`/`EndPlot`. Library still builds and passes tests with `UNIGUI_MODULE_TRADING=OFF` (widget + test gated out). 11 new headless tests.
- **`unigui::im` A5 — misc widgets, debug tools, draw-list** (Horizon 2): `InvisibleButton`, `ArrowButton`, `CheckboxFlags` (int + unsigned int overloads), `ColorButton`; `ShowDemoWindow`, `ShowMetricsWindow`, `ShowStyleEditor`; `GetWindowDrawList`, `GetBackgroundDrawList`, `GetForegroundDrawList`. 10 new headless tests (`im` count 142 → 157). _(Practical-surface completion finished in A6 below.)_
- **`unigui::im` A4 — item & input queries** (Horizon 2): Full `IsItem*` family (hovered, active, focused, clicked, visible, edited, activated, deactivated, deactivated-after-edit, toggled-open), `IsAnyItem*`, `GetItemRectMin/Max/Size`; keyboard queries (`IsKeyDown/Pressed/Released`); mouse queries (`IsMouseDown/Clicked/Released/DoubleClicked/Dragging/HoveringRect`, `GetMousePos`, `GetMouseDragDelta`, `ResetMouseDragDelta`). 9 new headless tests added (`im` count 114 → 142).
- **`unigui::im` A3 — popups / modals / menus** (Horizon 2): `OpenPopup` (string + numeric ID), `OpenPopupOnItemClick`, `BeginPopup`, `BeginPopupModal`, `EndPopup`, `CloseCurrentPopup`, `IsPopupOpen`, `BeginPopupContextItem/Window/Void`; `BeginMenuBar/EndMenuBar`, `BeginMainMenuBar/EndMainMenuBar`, `BeginMenu/EndMenu`, `MenuItem` (value + toggle overloads). All string params are `std::string_view`; empty string_view converts to `nullptr` for ImGui's optional-id parameters. 11 new headless tests added (`im` count 96 → 114).
- **`unigui::im` A2 — window / layout / scroll / cursor** (Horizon 2): `SetNextWindow*` (pos, size, size-constraints, content-size, collapsed, focus, scroll, bg-alpha), `BeginChild/EndChild` (string-id and numeric-id overloads), scrolling (`GetScrollX/Y`, `GetScrollMaxX/Y`, `SetScrollX/Y`, `SetScrollHereX/Y`, `SetScrollFromPosX/Y`), `BeginGroup/EndGroup`, `PushClipRect/PopClipRect`, cursor access (`GetCursorScreenPos/SetCursorScreenPos`, `GetCursorPos/X/Y`, `SetCursorPos/X/Y`, `GetCursorStartPos`, `GetContentRegionAvail`, `GetWindowPos/Size/Width/Height`), item-width stack (`PushItemWidth`, `PopItemWidth`, `SetNextItemWidth`, `CalcItemWidth`), `AlignTextToFramePadding`, line-metric getters (`GetTextLineHeight*`, `GetFrameHeight*`), `SetItemDefaultFocus`. 14 new headless tests added.
- **`unigui::im` A1 — inputs/sliders/drags completeness** (Horizon 2): The immediate-mode layer now covers the full scalar × vector family and the vertical slider variants. New functions: `SliderFloat2/3/4`, `SliderAngle`, `SliderInt2/3/4`, `VSliderFloat`, `VSliderInt`, `DragFloat2/3/4`, `DragFloatRange2`, `DragInt2/3/4`, `DragIntRange2`, `InputFloat2/3/4`, `InputInt2/3/4`, `InputDouble`, `InputTextWithHint`. All take `std::string_view` labels/formats and delegate straight to ImGui — no heap allocation for the common case. `docs/API_INDEX.md` updated (im count 22 → 47). 14 new headless test cases added.

### Fixed
- **CSS parser no longer throws on MSVC**: `styling::Engine::Parse()` replaced its three `std::regex` scanners (rule blocks, `@media` inner rules, declarations) with linear hand-written scanning. MSVC's `std::regex` enforces a backtracking-complexity governor that libstdc++/libc++ do not, so the old greedy patterns (e.g. `([^{]+)\s*\{([^}]*)\}`) threw `std::regex_error(error_complexity)` on long/pathological input — breaking the engine's "parsing never throws" contract on Windows (the `FuzzCSS` no-throw targets failed under MSVC). The replacement is allocation-light and O(n), preserving behaviour for valid CSS while handling malformed input without throwing (the two fuzz targets drop from ~36s-and-failing to <0.1s).
- **`Table` numeric-sort parsing is non-throwing (perf)**: `ParseNumericCell` (`widgets/table.cc`) replaced `std::stod`-in-`try/catch` with `std::from_chars`. The old path allocated and threw an exception for every non-numeric cell, and it runs once per row on every sort — so sorting a large text-heavy column threw thousands of exceptions. `from_chars` parses without throwing (and keeps the code within the "no throwing parsers" rule); a leading `+` is honoured as before. New tests cover signed-number ordering and mixed numeric/text sorting.
- **Theme & locale JSON parsers no longer throw on MSVC**: `ImportThemeJSON()` (`theme.cc`) and `Locale::LoadFromFile()` (`locale.cc`) used the same fragile `std::regex` approach and could throw `std::regex_error(error_complexity)` on large/malformed input — `LoadFromFile` reads untrusted `.json` files from disk, so this was an unguarded crash path on Windows. Both now use linear hand-written scanning. `ImportThemeJSON` additionally stops recompiling a regex once per colour slot (×`ImGuiCol_COUNT`), so a theme import is a single pass instead of ~58 regex compiles. New tests cover Export→Import round-tripping, whitespace/empty values, and long-input no-throw. Also dropped a stale `#include <regex>` from `widgets/markdown.cc`.
- **`cmake-msvc.cmd` auto-discovers vcpkg**: when `VCPKG_ROOT` is unset the wrapper now resolves it automatically (vcpkg on `PATH`, then the copy bundled with Visual Studio) so a fresh clone configures with the Ninja presets without any manual environment setup. An explicit `VCPKG_ROOT` still wins.
- **Warnings-clean under `-Werror` + CI gate (Horizon 1)**: the whole tree — library, tests, and examples — now builds clean with `UNIGUI_WARNINGS_AS_ERRORS=ON`, and a new `linux-werror` job in `build.yml` enforces it on every push/PR. Removed stray unused variables/parameters from the `widget_base`/`treeview`/`style_scope` tests, and added `-Wno-missing-field-initializers` to `cmake/CompilerWarnings.cmake` (GCC/Clang): `-Wextra`'s missing-field-initializers warning conflicts with the project's clang-tidy `readability-redundant-member-init` policy (idiomatic partial aggregate initialisation is used throughout), so it is suppressed rather than fought. All tests pass with warnings-as-errors on.
- **`MultiHandleSlider` build with ImGui ≥ 1.90**: `ImGuiButtonFlags_AllowOverlap` was removed upstream; replaced with `ImGui::SetNextItemAllowOverlap()` before the `InvisibleButton` call.
- **`TimeSeriesChart` build with latest ImPlot**: `ImPlotSpec` struct no longer exists; reference-line colour is now applied via `ImPlot::PushStyleColor(ImPlotCol_Line, …)` / `PopStyleColor()`.
- **vcpkg baseline**: updated `builtin-baseline` in `vcpkg.json` from stale commit `f9ffbaa4` to current HEAD `75a2e142` so `vcpkg install` resolves without requiring a `git fetch`.

---

## Unreleased — UI Beautification

### Added
- **Unified style tokens (Step 1)**: New `theme/style_tokens.h` centralises all geometry (rounding, spacing, borders) in `ApplyStyleTokens(ImGuiStyle&)`. Dark/Light themes and every preset call it instead of hand-tuning, so all themes share one consistent geometry. Light contrast was tightened in the same pass.
- **Surface style presets (Step 2)**: New `unigui::theme::SurfaceStyle` (`Solid`, `Glass`, `Frosted`, `Acrylic`, `Minimal`) layers a translucency/“material” pass on top of any colour palette via `ApplySurfaceStyle()` in `theme/surface_style.h`. Frosted glass (毛玻璃 / glassmorphism) is the default — `ThemeConfig::surface` defaults to `SurfaceStyle::Glass` — while every other look keeps a ready-made preset. `SurfaceStyleName()` and `AllSurfaceStyles()` support theme pickers.
- **Theme backdrop**: `GetBackdropColor()` exposes a theme-/material-derived opaque framebuffer clear colour, and the app loop now clears every backend to it so translucent glass surfaces read against a tinted backdrop instead of black. `BackdropColor()` in `theme/surface_style.h` derives it from the window background.
- **Accent & semantic colour tokens (Step 3)**: New `theme/color_tokens.h` derives a theme's full interactive palette — accent → hover → active plus semantic `Success`/`Warning`/`Danger`/`Info` — from a single base accent (reusing `AccentHover`/`AccentActive`). `ApplyColorTokens()` rewrites only the accent-driven ImGui slots (CheckMark, SliderGrab(Active), SeparatorActive, ResizeGrip*, DragDropTarget, NavHighlight, DockingPreview, TextSelectedBg), so it composes with each palette without clobbering Button/Header/Tab/Plot colours. `ApplyTheme()` (Dark/Light) and `ThemeRegistry::Apply()` (all 13 presets) now apply it after the palette and before the surface pass, so every theme shares one consistent accent relationship. The active semantic palette is queryable via `unigui::GetColorTokens()` / `unigui::GetSemanticColor(Semantic)` for widgets that need success/warning/danger/info colours.
- **Theme-driven elevation (Step 4)**: New `fx/elevation.h` ties the `ShadowEffect`/`GlowEffect` primitives to a semantic `Elevation` scale (`None`/`Low`/`Medium`/`High`) and the active surface material. `ElevationPreset(level[, surface])` derives shadow radius/offset/alpha/samples plus an optional rim glow — glass/frosted/acrylic get a softer, more diffuse shadow **plus a bright rim**, solid gets a firmer shadow, and minimal stays quiet. `MakeElevationShadow()`/`MakeElevationGlow()` build ready effect objects. Widgets gain `SetElevation()` / `WithElevation(fx::Elevation)` (on both `Widget` and `FluentWidget<>`), which fills a widget's `ShadowConfig` from the active material's elevation tokens.

### Added
- **Trading toolkit — formatting & models (phases B0–B1)**: first slice of the trading-client toolkit (see `docs/TRADING.md`).
  - `core/format_num.h` (`unigui::format`): locale-neutral, dependency-free formatters — `Thousands`, `Fixed`, `Currency`, `Percent`, `SignedDelta`, `TickAlign`, and `Sign`/`Direction` for colouring P&L cells without pulling in ImGui. Complements the existing Chinese `MoneyCN`/`VolumeCN`.
  - `trading/` header-only models: `Quote`/`Position`/`Order`/`Trade` row types (with derived getters like `UnrealizedPnL`, `Spread`, `ChangePct`, `Remaining`), `OrderBook` (aggregated depth-of-market: snapshots/deltas, best bid/ask, spread, mid, top-N levels), and `OhlcSeries` (rolling candlestick aggregator: ticks → fixed-interval OHLC bars + ImPlot-ready column extractors).
  - New `UNIGUI_MODULE_TRADING` CMake option (default OFF) gating the toolkit; the models are header-only so their GoogleTest suites build unconditionally. Library still builds/tests with the module off.
- **CI coverage floor (advisory)**: the `quality.yml` coverage job now computes total line coverage from the lcov summary and compares it against a configurable `COVERAGE_FLOOR` (currently 30%), surfacing a GitHub warning when coverage drops below it. Advisory for now — like the existing clang-tidy gate — so it never breaks the build; it can be flipped to a hard gate once the headless baseline is confirmed.
- **Compiler-warning configuration**: new `cmake/CompilerWarnings.cmake` applies a base warning level to the library — `/W4` on MSVC, `-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor` on GCC/Clang — so issues are visible during development. The flags are `PRIVATE` and never leak onto downstream embedders. A new opt-in `UNIGUI_WARNINGS_AS_ERRORS` option (off by default) promotes warnings to errors (`-Werror`/`/WX`), available via the `linux-gcc-debug-werror` preset for CI/strict builds.
- **Expanded parser fuzzing**: new no-GL fuzz targets beyond the existing CSV/JSON suites — `test_css_fuzz` hammers the CSS style engine (`styling::Engine::Parse`) with random bytes, CSS-token-biased input, and structural edge cases (unbalanced braces, malformed selectors/declarations, `@media`/`var()`/`linear-gradient()` fragments, embedded NULs, 100k-char declarations); `test_config_fuzz` feeds malformed TOML/JSON/INI through `config::Store::Load*` via temp files (random bytes, token-biased input, unterminated strings, out-of-range numbers, missing files). The property under test is that parsing never throws or crashes. (DSL has no string-parser surface — it is a programmatic builder — so it is exercised by its unit tests rather than a fuzz target.)
- **API-stability policy & annotations**: New `docs/API_STABILITY.md` makes the public-API contract explicit — semver scope for `include/unigui/**`, three stability tiers (Stable / Experimental / Internal) with a current per-module classification, and a deprecation lifecycle (deprecate in a MINOR → grace period → remove in a MAJOR). New header `<unigui/core/api.h>` provides the `UNIGUI_DEPRECATED("msg")`, `UNIGUI_EXPERIMENTAL`, and `UNIGUI_INTERNAL` markers so the tiers are greppable and self-documenting at the declaration site; the `ext/` headers (`node_editor.h`, `plot.h`) are now marked experimental. `core/version.h` gains `UNIGUI_VERSION_NUMBER`, `UNIGUI_MAKE_VERSION()`, and `UNIGUI_VERSION_AT_LEAST()` for compile-time version/feature detection.

### Changed
- **Consolidated the `src/v2/` layout (no more parallel code paths)**: the `v2/` directory was a leftover from the v2.9.0 namespace removal, not a second implementation — after that change its files were the *sole* implementations of the optional modules. They (and their GoogleTest suites) have been relocated from `src/v2/` and `tests/v2/` into the mirrored `src/<module>/` and `tests/<module>/` layout (`dsl/`, `styling/`, `events/`, `plugin/`, `config/`, `sqlite/`, `ipc/`, `network/`, plus `fonts/font_manager.cc`), matching the project's header/impl mirror convention. CMake source lists updated accordingly; no behavioural change.
- **Input widgets honour the disabled state**: `ComboBox`, `InputText`, `InputFloat`, and `InputInt` now wrap their Dear ImGui calls in `BeginDisabled()/EndDisabled()` when `SetEnabled(false)` is set — matching `Button`. Forms can lock a field (e.g. once a row has been committed) and the widget greys out and stops accepting input instead of silently staying editable.
- **Higher-contrast table zebra striping**: the auto-derived `ImGuiCol_TableRowBgAlt` now blends `WindowBg` 13% toward the text colour (was a barely-perceptible tweak), so alternating rows are clearly distinguishable across **every** theme. This reduces the risk of mis-reading or editing the wrong row in dense parameter tables.
- **`RiskBar` is fully theme-aware**: the track uses `ImGuiCol_FrameBg`; the fill uses semantic `Success`/`Warning`/`Danger` colours instead of hard-coded hex; and the centred label now picks a light/dark foreground per theme and draws a 1px opposite-colour outline so it stays legible over both the filled and empty portions of the bar.
- **`SliderBar` is theme-aware and more flexible**: track / reference markers / handles follow the theme `FrameBg` / `Text` / accent and semantic danger colours. New `SetLeftPanelWidth(0)` hides the built-in Add/OK/Rbk/Sub panel so the bar can span the full available width (caller renders its own header); new `SetWarnRatio(r)` draws a configurable red warning band from `r`→full (default 90%) with 0%/100% endpoint markers; `GetBarLocalX()` / `GetBarWidth()` expose post-render geometry for overlaying widgets under the handles.

### Fixed
- **`DataTable` row selection with checkbox columns**: the row-click `Selectable` is now rendered on the first *non-checkbox* column instead of always column 0, so rows stay selectable even when column 0 is a checkbox.
- **`DataTable` transparent cell colour**: a `SetCellColor`/cell-colour callback returning a colour with alpha 0 now means *"no override"* and keeps the default text colour, instead of pushing a fully-transparent (invisible) text colour and erasing the cell text.

---

## v3.5.0 (2026-06-02) — TreeView/CascadingCombo UX, Table Sorting & Docs Overhaul

### Added
- **`CascadingCombo` layout & width control**: New `SetLayout(Layout::Horizontal|Vertical)` arranges the per-level combos side-by-side or stacked; `SetItemWidth(float)` sets a global combo width with per-level overrides via `SetItemWidth(int level, float)` / `Level::width`; `SetSpacing(float)` controls the gap in horizontal layout. Fluent `WithLayout/WithItemWidth/WithSpacing` wrappers included. `SetHorizontal(bool)` is kept as a convenience wrapper.
- **`DataTable` checkbox columns**: `SetCellCheckbox(int col, fn)` renders an inline checkbox column; `SpanAllColumns` is disabled automatically when checkbox columns are present so clicks land correctly.
- **`Table` cell embedding**: Cells can now host custom-rendered content via a cell renderer callback, allowing arbitrary widgets/markup inside table cells instead of plain text only.
- **`Table` column sorting**: `SetSortable()` enables interactive column sorting backed by Dear ImGui sort specs, with a numeric-aware default comparator and support for per-column custom comparators (`SortComparator`).
- **Documentation overhaul**: Rewrote `docs/WIDGET_API.md` as a verified, categorized reference covering all 82 widgets with constructors, methods, and examples. Added `docs/TREEVIEW.md` and `docs/CASCADINGCOMBO.md` in-depth guides. Synced `README.md` / `README_zh.md` (correct widget/test counts, version, new components, guide links).

### Changed
- **Chart theme background**: Charts now follow the active theme background.
- **`StatusLamp` glow**: Added a glow effect, with vertical glow padding included in the widget bounds.
- **Broader character set**: Expanded the font glyph coverage.

### Fixed
- **`Table` sort performance**: Default sort parses each cell into a sort key once and reorders an index permutation, avoiding repeated parsing on every comparison.

---

## v3.4.1 (2026-06-02) — PanelBox Bounds Fix

### Fixed
- **`PanelBox` ImGui bounds growth**: `PanelBox::Render()` now submits a real `Dummy()` item covering the full panel bounds instead of only moving the cursor with `SetCursorScreenPos()`. This removes Dear ImGui boundary warnings when `PanelBox` is used inside nested splitter/child layouts and keeps parent windows sizing correctly.

---

## v3.4.0 (2026-06-01) — API Ergonomics + Developer Tooling

### Added
- **`unigui::RunApp(config, callback, maxFrames=0)`**: One-call entry point — `Init` + main loop + `Shutdown` with init-failure handling. Reduces a typical `main()` to a single expression returning an exit code.
- **`Run(callback, maxFrames=0)`**: Optional frame-count cap on the main loop; useful for CI smoke runs, screenshots, and headless tests without writing a manual `while` loop.
- **Widget fluent API**: `Widget::With*` chainable configuration wrappers — `WithTooltip`, `WithEnabled`, `WithVisible`, `WithUserData`, `WithAccessibleName`, `WithAccessibleDescription`, `WithMinSize`, `WithMaxSize`, `WithShadow`. Return `Widget&` for one-liner setup, e.g. `btn.WithTooltip("Save").WithEnabled(false).WithShadow()`.
- **`scripts/check_env.ps1`**: Toolchain self-check script — detects VS/CMake/Ninja/vcpkg, flags stale MSVC toolsets on `PATH`, prints PASS/WARN/FAIL with concrete fix suggestions.
- **`scripts/build.ps1`**: One-command configure+build+test wrapper (supports `-Preset`, `-Test`, `-Clean`, `-SkipCheck`).
- **`cmake-msvc.cmd` enhanced**: Locates Visual Studio dynamically via `vswhere` — works across all editions and version upgrades without hard-coded paths. Adds friendly error messages for missing C++ workload, `cl.exe`, or `cmake`.

### Fixed
- **`Run()` double event poll**: The old implementation called `PollEvents()` explicitly then `NewFrame()` called it again. Fixed so only `NewFrame()` polls.
- **`version.h` alignment**: Bumped to match the canonical project version (was stuck at 0.1.0).

### Changed
- **Tests**: 597 → 598 (+1 `FluentApi_ChainsAndAppliesState`)
- **`hello_unigui` example**: Refactored to demonstrate `RunApp` and the new fluent widget API.

---

## v3.3.1 (2026-06-01) — 9 New Widgets + Customer Requirements

### Added
- **9 new widgets**: PanelBox, RiskBar, StatusLamp, AlertBar, ConfirmDialog, CascadingCombo, SliderBar, FuturesRiskBar, CollapsibleTree (TreeView enhanced)
- **TreeView enhancements**: RowRenderer callback, icon/suffix/progress/color fields on TreeNode, leaf markers
- **DataTable enhancements**: Row click callback (SetRowClickCallback/SetSelectedRow), sort indicators (DefaultSort arrows), SetColumnMinWidth
- **Theme persistence**: ThemeRegistry::GetCurrentThemeName()
- **Font scale API**: unigui::SetFontScale() / GetFontScale()

### Changed
- **Tests**: 449 → 579 (+130)
- **Widgets**: 74 → 83

### Fixed
- DataTable StickyHeader: added TableSetupScrollFreeze(0,1) for proper header freeze
- Emoji rendering: FontManager::LoadSystemEmoji() auto-loads Segoe UI Emoji on Windows
- CascadingCombo: simplified to BeginCombo/EndCombo pattern for MSVC compatibility

# Changelog

## v3.3.0 (2026-06-01) — ID Safety + New Widgets + Developer Tooling

### Added
- **6 new widgets**: `CollapsingHeader`, `Selectable`, `ColorEdit`, `DragFloat`, `DragInt`, `ListBox` — all with PushID/PopID ID safety.
- **ID Safety (PushID/PopID)**: All 62 widget Render() methods now auto-scope ImGui IDs via `PushID(name)/PopID()`. Zero ID collisions regardless of label duplication. Non-Widget classes (Badge, Shimmer, Skeleton) use `PushID(this)`.
- **`.clang-format`**: Code style config (4-space indent, K&R braces).
- **`.clang-tidy`**: Static analysis config — bugprone, performance, modernize, readability, cppcoreguidelines checks.
- **Coverage**: `windows-clang-coverage` CMake preset with source-based coverage instrumentation (`llvm-profdata` + `llvm-cov`). HTML report at `cmake --build <dir> --target coverage`.
- **Clang-tidy preset**: `windows-clang-tidy` — runs clang-tidy on every compile.
- **`cmake-msvc.cmd`**: Portable MSVC build wrapper (uses vswhere to locate VS).
- **`compile_commands.json`**: Auto-generated in all builds (IDE + clang-tidy support).
- **`cmake --build <dir> --target lint`**: Standalone clang-tidy across all sources.
- **`cmake --build <dir> --target coverage`**: Full coverage pipeline (test → merge → HTML report).
- **ASAN presets**: `windows-msvc-debug-asan` and `linux-gcc-debug-asan`.
- **API documentation**: `docs/WIDGET_API.md` — 1746 lines, 74+ widgets with C++23 examples.

### Changed
- **Test suite**: 245 → 285 tests (100% pass on both MSVC and Clang).
- **Widget count**: 68 → 74.
- **CMakePresets.json**: Expanded from 6 to 10 presets (added clang-tidy, clang-coverage, MSVC-asan, Linux-asan).
- **`.bashrc`**: Added useful aliases (`cl`, `ct`, `cb`, `ctb`).

### Fixed
- **ID collisions**: 47 widgets were missing PushID/PopID scoping — all now fixed.
- **badge.cc**: Fixed non-Widget class incorrectly receiving Widget-only API calls.
- **listbox.cc**: Fixed ImGui::ListBox getter signature (captureless lambda → function pointer).

## v3.2.7 (2026-05-28) — Group Rows + Cross-Platform Foundation

### Added
- **DataTable group rows**: `SetGroups(vector<GroupInfo>)`. Collapsible group headers (▼/▶), per-group sort mini-headers with ▲/▼ 3-state toggle, "Ungrouped" separator section.
- **DataTable context menu**: `SetContextMenu(fn(row))` — right-click popup.
- **DataTable column reorder / FlashRow**: `SetColumnReorderable`, `FlashRow(row,color,duration)`.
- **TimeSeriesChart**: crosshair formatter, multi Y-axis, reference lines, X-axis formatter, rubber band zoom.
- **ProgressBar gradient**: `SetGradient(t1,c1,t2,c2,c3)`.
- **MultiSplitter ratio persistence**: `GetRatios()/SetRatios()`.
- **InputInt/InputFloat suffix**: `SetSuffix(string)`.
- **Window/TabWidget**: `SetCloseToTray`, `SetTabShortcut`.
- **unigui::format**: `MoneyCN(amount)` → "5300万", `VolumeCN(vol)` → "1500手".
- **Linux**: Fedora 43/GCC 15.2 compile — 225/225 targets, 236/244 tests.
- **macOS**: Metal renderer ObjC++ (code ready, untested).
- **Emscripten**: HTML shell + platform backend (code ready, untested).
- **CI/CD**: GitHub Actions Win/Lin/Mac 3-job matrix.

### Fixed
- All interactive widgets: `PushID(name)` scoping prevents same-label ID collisions.
- DataTable: column header sort by correct column (was always col 0); default string sort; multi-select.
- TimeSeriesChart: SetupAxis now inside BeginPlot block.
- EventBus exit crash: `~Bus()` destructor calls `Shutdown()`.

## v3.2.5 (2026-05-27) — Widget ID Sanitation + DataTable Sort

### Fixed
- **Widget ID collisions**: `PushID(name)/PopID()` on Button, CheckBox, ToggleSwitch, ComboBox, LineEdit, InputInt, InputFloat. Multiple widgets with same label no longer conflict.
- **DataTable sort**: `TableSetupColumn(user_id=ci)` — clicking column header now sorts by that column (was always column 0).
- **DataTable default sort**: columns without `SetSortCompare` auto-sort via `CellFormatter` string compare.

### Changed
- v3_overview: stable 5-panel demo (Theme/Table+Btn/Btn+Toast/Badge+Text/Anim), pure UniGUI API.

## v3.2.4 (2026-05-27)

### Added
- INTEGRATION.md with CRT troubleshooting, CMake CRT diagnostic

## v3.2.2 (2026-05-26) — Inline Editing & Filtering

### Added
- **InvokeOnMainThread / ProcessMainThreadTasks**: cross-thread dispatcher for UI updates from network/IO callbacks.
- **DataTable\<T\> inline editing**: `SetCellEditable(col, bool)`, double-click enters
  InputText popup, Enter commits via `SetOnCellCommit(CellCommitFn)`, Escape cancels.
- **DataTable\<T\> text filtering**: `SetFilterText(string)` + `SetFilterFn(FilterFn)`.
  Rows not matching filter text (searched across all columns via CellFormatter) are hidden.

## v3.2.1 (2026-05-26) — Data Widgets

### Added
- **DataTable\<T\>**: high-performance template data table with zero-copy data source,
  virtual scrolling (ImGuiListClipper), column sorting, row colouring (profit/loss),
  cell formatting, selection + double-click callbacks. Header-only (`include/unigui/widgets/datatable.h`).
- **MultiHandleSlider**: multi-handle draggable slider bar with tick management,
  per-tick color, custom per-tick overlay rendering, current-position marker line.
- **TimeSeriesChart**: real-time time-series plot via implot with sliding window,
  auto-fit Y axis, crosshair toggle, legend, grid color. `AppendPoint()` with timestamp.

## v3.2.0 (2026-05-26) — Cross-Platform + Polish

### Added
- **Linux**: Full compilation support (Fedora 43, GCC 15.2, 225/225 targets, 236/244 tests). `cmake/embed_font.py` cross-platform font embedding, platform-aware CJK font paths (Windows/MSYH, macOS/PingFang, Linux/NotoSansCJK).
- **macOS**: Metal backend ObjC++ implementation (MTLDevice, CommandQueue, ImGui_ImplMetal_Init/Shutdown/Render). CMake `-fobjc-arc` + Metal.framework/QuartzCore.framework linkage. Platform-aware CJK fallback.
- **Emscripten/Web**: Full platform backend (canvas sizing, emscripten_set_main_loop, HTML shell template with spinner and Module bridge).
- **CI/CD**: GitHub Actions cross-platform matrix — Windows (MSVC), Linux (Ubuntu+GCC), macOS (Clang).
- **Card**: `SetBorderColor(ImU32)`, `SetBorderRadius(float)`, proper padding via WindowPadding.
- **SkeletonScreen**: built-in shimmer animation via `SetShimmer(bool, speed)`.
- **ThemeRegistry**: `SetOnChange(std::function<void(std::string)>)` callback for theme switch notifications.

### Fixed
- CMake minimum lowered 3.31→3.26 for Rocky/Fedora compatibility.
- `vcpkg.json`: DX11/DX12 bindings split to Windows-only platform.
- `app.cc::NewFrame()`: DX11 resize code wrapped in `#ifdef UNIGUI_HAS_DX11`.
- `std::find` → `#include <algorithm>` added (4 files) for GCC 15 strictness.
- `webgpu/emscripten/metal` backend stubs compiled on all platforms (not just WIN32).
- `config/database/ipc` headers guarded by `UNIGUI_HAS_*` preprocessor defines in `unigui.h`.

### Changed
- **AnimationState** docs: `progress` = target, `Update(dt)` return = eased current value.
- **FontManager::Build()** doc warning: rebuilding atlas invalidates ImFont* pointers.

## v3.1.0 (2026-05-26) — Stability

### Fixed
- **EventBus exit crash**: `~Bus()` destructor now calls `Shutdown()` to join worker thread (244/244 tests pass)
- **Toast triple-bug**: double-animation, `Hide()` deadlock, z-order burying via per-message windows + `BringWindowToDisplayFront`
- **CSS @media** evaluates `min-width`/`max-width`/`min-height`/`prefers-color-scheme`

## v3.0.0 (2026-05-26) — UI Beautification

### Added
- **fx/easing**: 10 easing curves (linear, quad, cubic, expo, elastic, bounce) + CSS aliases + `ParseEasing()`
- **fx/effects**: `ShadowEffect` (multi-pass blur), `GlowEffect` (radial rings), `BlurEffect` (glass morphism), `GradientBrush` (horizontal/vertical/multi-stop), `Effects` factory
- **fx/animation**: `AnimationState` with Play/Stop/Loop/PingPong/`onComplete`, `AnimationManager` singleton
- **fx/transition**: `Fade`, `SlideIn`, `Scale`, `CrossFade`, `PageSwitch`, `Appear`, `Disappear` (header-only)
- **CSS Engine v2**: 16 → 70 properties, `linear-gradient()` parser, `transition` shorthand, `:active`/`:focus`/`:disabled`/`:first-child` pseudo-classes, `@media` block detection
- **10 built-in themes**: Material Dark/Light, Fluent Dark/Light, Dracula, Nord, Gruvbox, Catppuccin Mocha, Solarized Dark/Light, TokyoNight, OneDark, Everforest
- **ThemeRegistry**: `Register`/`Get`/`List`/`Apply` + `RegisterAllThemes()` auto-init
- **New widgets**: `Card` (Elevated/Outlined/Filled), `Shimmer` (animated sweep), `Badge` (Dot/Count/Label), `SkeletonScreen` (placeholder blocks), `HeroSection` (gradient banner + CTA), `GradientText` (per-char interpolation)
- **Widget polish**: Button animated hover, Toast eased fade-in, ProgressBar animated fill, ToggleSwitch alpha pulse, TabWidget crossfade, Panel shadow via `WidgetBase::SetShadow()`
- **Demos**: `v3_overview` (all features), `theme_demo` (auto-cycle), `widget_gallery` updated

### Changed
- `Toast` now renders in `Render()` (on top of all windows), per-message independent ImGui windows

## v2.9.0 (2026-05-26) — De-v2 Namespace

### Breaking
- **All `v2::` namespace removed**: `unigui::v2::EventBus` → `unigui::events::Bus`, etc.
- Headers moved: `include/unigui/v2/*` → `include/unigui/<module>/*`

## v2.8.0 (2026-05-26) — Modular CMake

### Added
- 20 configurable modules via `-DUNIGUI_MODULE_*=ON/OFF`
- 3 presets: recommended (default), minimal (~200 targets), full (~470 targets)
- Conditional deps: SQLite3, ZeroMQ

## v1.10.0 (2026-05-26) — TrayIcon Polish

### Added
- TrayIcon::UpdateTooltip(title) — dynamic tooltip via NIM_MODIFY
- TrayMenuItem submenus — children field for recursive submenus
- TrayMenuItem::isSeparator — menu separators (MF_SEPARATOR)
- ShowNotification(title, msg, NotifyType) — Info/Warning/Error icons
- Toast dismiss callback — Show(msg, type, dur, onDismiss)
- PasswordInput demo in widget_gallery (strength indicator)

### Changed
- Version: 1.9.0 → 1.10.0
- Tests: 202/202 pass

### Fixed
- **TrayIcon**: proper right-click menu via TrackPopupMenu, icon resource ID parameter, notification title/msg param pass-through.
- **GetNativeWindowHandle()**: public API in app.h, returns HWND on Windows.
- **GLFW headers**: auto-included in unigui.h (glfw3.h + glfw3native.h), no user include needed.
- **Toast::SetPosition(anchor, x, y)**: control notification position (top-left/right, bottom-left/right).

### Changed
- vcpkg version synced with CMake version (1.9.0)
- Version: 1.8.0 → 1.9.0
- Tests: 202/202 pass

## v1.8.0 (2026-05-26) — 1.x Series Final

### Finalized
- **API Freeze**: Public API locked for 1.x → 2.0 migration. No breaking changes in 1.x series.
- **CHANGELOG unified**: All versions from v0.1.0 through v1.8.0 in a single document.
- **202 tests**: Full regression suite, 100% pass rate maintained.
- **9 releases**: v1.0.0 through v1.8.0 delivered in a single session.

### What Changed From v0.1.0
- Windows backend: OpenGL → **DX11** (AMD GPU stable)
- Font: system-dependent → **JetBrains Mono Nerd Font embedded**
- Widgets: 6 → **55**
- Tests: 74 → **202**
- Backends: 1 → **7 (4 runtime-ready)**
- Enterprise: i18n, Settings persistence, UndoStack, serialization
- Performance: VirtualList 10k items < 100ms

## v1.7.0 (2026-05-26) — Performance Optimization
- Extended benchmarks: 100 buttons, 100 labels, VirtualList 10k, Form 20 fields
- VirtualList: 10,000 items < 100ms (ImGuiListClipper)
- Tests: 200 → 202

## v1.6.0 (2026-05-26) — Advanced Features
- Layout::HBox/VBox/BeginHSplit/EndHSplit declarative helpers
- Window::SetDropCallback for file drag-drop
- Clipboard::Copy/Paste wrapping ImGui clipboard
- Animate::FadeIn/SlideIn/Lerp/FadeScope animation system

## v1.5.1 (2026-05-26) — System Tray + Widget Docs
- TrayIcon: Shell_NotifyIcon (Windows), menu + notifications
- README widget quick-reference table

## v1.5.0 (2026-05-26) — User Feedback Fixes
- ImPlot::CreateContext/DestroyContext auto-managed
- DX12 renderer conditional compile
- GLFW_EXPOSE_NATIVE_WIN32 handled internally
- GetNativeWindowHandle() unified API

## v1.4.0 (2026-05-26) — i18n & Settings
- Locale::LoadFromFile(JSON) + built-in en_US/zh_CN/ja_JP
- Settings::EnableAutoSave + MRU (AddRecentFile/GetRecentFiles)
- hello_unigui i18n switch demo

## v1.3.0 (2026-05-26) — Platform Hardening
- Render integration test (manual, GPU required)
- Fuzz test: 100 random widget iterations
- Font smoke test: embedded font loading

## v1.2.0 (2026-05-26) — Widget Ecosystem
- VirtualList: 100k+ entries with ImGuiListClipper
- MultiCombo: multi-select checkboxes + preview
- PropertyGrid: two-column property editor
- SearchBox: filtered dropdown suggestions
- Toast: singleton Info/Success/Warn/Error notifications
- PasswordInput: strength indicator (0-4)
- Wizard: multi-step with Next/Previous/Finish

## v1.1.0 (2026-05-26) — Embedded Fonts
- JetBrains Mono Nerd Font embedded via CMake pipeline
- LoadDefaultFont: AddFontFromMemoryTTF
- CJK merge: optional system font merge
- DPI update: FontGlobalScale instead of reload

## v1.0.0 (2026-05-26) — First Stable Release
- DX11 backend: Windows default, AMD-stable
- DPI auto-scaling: GetDpiForWindow
- CJK font support: Microsoft YaHei merge
- Window resize: DX11 swapchain resize
- Auto-wrap text: Panel default
- Popup input priority: Window NoInputs
- spdlog structured logging
- 48+ widgets, 186 tests

## v0.5.0 (2026-05-25) — Enterprise
- Locale i18n, Settings INI, UndoStack
- Form/Table/Theme serialization
- Widget accessibility & size constraints
- Performance benchmarks

## v0.4.0 (2026-05-25)
- NodeEditor groundwork
- RichText, ImageButton, Markdown widgets
- Undo/Redo for LineEdit/MultiLine
- Form validation, ComboBox icons, Table columns
- plot_demo example

## v0.3.2 (2026-05-25)
- DX12 renderer, WebGPU/Metal/Emscripten stubs

## v0.3.1 (2026-05-25)
- Multi-Viewport, DockSpace, ContextMenu, DragDrop, ShortcutManager

## v0.3.0 (2026-05-25)
- widget_gallery, form_demo, CI matrix, vcpkg port

## v0.2.3 (2026-05-25)
- DX11 HWND runtime, Doxygen, GitHub Actions CI

## v0.2.2 (2026-05-25)
- SDL3+Vulkan backend, 8 widgets (CheckBox through TabWidget)

## v0.1.0 (2026-05-25)
- GLFW+OpenGL3, Dark theme, 6 widgets, App Bootstrap, 74 tests
