# v2.0 Completion + v2.1 Roadmap

## v2.0 Remaining Tasks (Tasks 15-20)

> Status: Core v2.0 done (104 tests, both backends compile). These 6 integration tasks remain.
> Execute via: `/start-work unigui-v2.0` then complete tasks 15-20.

### Task 15: BackendFactory & VulkanContext Integration

**Files to create**:
- `include/unigui/backend/vulkan_context.h` — `VulkanContext` struct owning all Vulkan resources
- `src/backend/vulkan_context.cc` — `InitVulkanContext(SDL_Window*, w, h)` / `DestroyVulkanContext()`

**Files to modify**:
- `src/CMakeLists.txt` — add `backend/vulkan_context.cc` to SDL3_VULKAN sources
- `include/unigui/unigui.h` — include `vulkan_context.h`

**Implementation**:
```cpp
struct VulkanContext {
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanPipeline pipeline;
    std::vector<FrameResources> frames;
};
VulkanContext InitVulkanContext(SDL_Window* window, int w, int h);
void DestroyVulkanContext(VulkanContext& ctx);
```

### Task 16: App Bootstrap Vulkan Init Path

**Modify**: `src/app/app.cc`

**What to do**:
- Add static `VulkanContext g_vulkanCtx;` (module-level, alongside existing `g_platform`/`g_renderer`)
- In `Init()`: when `config.backend == SDL3_Vulkan`:
  1. Create SDL3 platform via `CreateSDL3Platform()`
  2. Call `g_platform->Init(nullptr)` to create SDL3 window
  3. Get `SDL_Window*` from platform (need `GetWindowHandle()` virtual method on `PlatformBackend`)
  4. Call `g_vulkanCtx = InitVulkanContext(window, config.width, config.height)`
  5. Create `VulkanRenderer(&g_vulkanCtx.device, &g_vulkanCtx.swapchain, &g_vulkanCtx.pipeline, &g_vulkanCtx.frames)`
  6. Call `g_renderer->Init(context)`
- In `Shutdown()`: call `DestroyVulkanContext(g_vulkanCtx)` before platform shutdown
- In `Render()`: after renderer draws, Vulkan present loop is already internal

**Also modify**: `PlatformBackend` — add `virtual void* GetWindowHandle() const { return nullptr; }` for SDL window access

### Task 17: hello_unigui v2 Widgets Demo

**Modify**: `examples/hello_unigui/main.cc`

Add a second tab/window showing v2 widgets:
- CheckBox with onChange callback
- Slider (float, 0-100)
- ProgressBar with state toggle
- RadioGroup (3 options)
- ComboBox with 4 items
- LineEdit with email validator
- GroupBox with content
- TabWidget with 2 sub-tabs

### Task 18: vulkan_triangle Example

**Create**: `examples/vulkan_triangle/main.cc` + `CMakeLists.txt`

Minimal Vulkan demo:
- SDL3 window + Vulkan surface
- Clear to dark color
- Render one ImGui draw call (e.g. triangle via ImDrawList)
- Present 10 frames, exit cleanly
- `--frames N` flag for CI

**Build**: Add to top-level CMakeLists.txt conditionally on SDL3_VULKAN

### Task 19: README v2.0 Update

Update `README.md`:
- v2.0 badge + features
- SDL3+Vulkan build instructions
- BackendType selection examples
- New widget API snippets
- v1→v2 migration (backward compatible, no changes needed)

### Task 20: CHANGELOG v2.0

Create `CHANGELOG.md`:
- v2.0 release notes
- Breaking changes: none
- New features: SDL3 platform, Vulkan renderer, 8 widgets, BackendType
- v1 compatibility statement

---

## v2.1 Plan: Tier 2 Widgets + Light Theme + DX11

### Scope

| Category | Items | Tasks Est. |
|----------|-------|-----------|
| **Widgets (8)** | TreeView, ListView, Dialog, MenuBar, StatusBar, ToolBar, Table, ColorPicker | 8 |
| **Theme** | Light theme preset (53 colors) | 1 |
| **Backend** | DirectX 11 renderer (Windows) | 3 |
| **Backend** | Metal renderer (macOS) | 1 |
| **Integration** | BackendType::DX11, BackendType::Metal | 2 |
| **Docs** | README + CHANGELOG | 2 |
| **Total** | | **~17 tasks** |

### Task Breakdown

**Wave 1 (Foundation)**: Light theme, BackendType::DX11/Metal enums
**Wave 2 (Widgets)**: 8 widgets in parallel (all thin ImGui wrappers)
**Wave 3 (Backends)**: DX11 renderer, Metal stub, CMake conditional
**Wave 4 (Integration)**: BackendFactory, App Bootstrap, examples
**Wave 5 (Docs)**: README, CHANGELOG
**Wave FINAL**: F1-F4 review

### Widget Details

| Widget | ImGui API | Complexity | Notes |
|--------|-----------|------------|-------|
| `TreeView` | `TreeNodeEx` | Medium | Recursive tree with expand/collapse state |
| `ListView` | `ListBox` + `Selectable` | Low | Single/multi-select, optional icons |
| `Dialog` | `OpenPopup`/`BeginPopupModal` | Medium | Modal with title, message, buttons |
| `MenuBar` | `BeginMenuBar`/`BeginMenu` | Medium | Standalone from Window (v1 has built-in) |
| `StatusBar` | `BeginMainMenuBar`-style bottom bar | Low | Text + progress indicator |
| `ToolBar` | Horizontal `Button` + `SameLine` | Medium | Icon buttons with tooltips |
| `Table` | `BeginTable` | High | Sortable columns, filter, selection |
| `ColorPicker` | `ColorEdit3`/`ColorEdit4` | Low | RGB/RGBA color editor |

### DX11 Backend (Windows only)

- `src/backend/dx11_renderer.cc`: ImGui DX11 backend wrapper
- Requires `imgui[dx11-binding]` in vcpkg.json
- DX11 boilerplate is ~200 lines (much simpler than Vulkan)
- HWND from Win32 platform or SDL `SDL_GetPointerProperty(SDL_PROP_WINDOW_WIN32_HWND_POINTER)`

### Metal Backend (macOS only)

- `src/backend/metal_renderer.mm`: ImGui Metal backend (Objective-C++)
- Requires `imgui[metal-binding]` in vcpkg.json
- ~100 lines for basic Metal setup

### Light Theme

- 53-color light theme preset (white/gray backgrounds, dark text, blue accent)
- `ThemePreset::Light` enum value
- `ApplyTheme({ThemePreset::Light})` applies all 53 colors inverted from dark

### Scope Boundaries

**INCLUDE (v2.1)**:
- 8 Tier 2 widgets, Light theme, DX11 renderer (Windows), Metal renderer stub (macOS)
- BackendType entries: DX11, Metal
- CMake conditional compilation for DX11/Metal

**EXCLUDE (v3+)**: Multi-viewport, WebGPU, Emscripten, Node editor, ImPlot, hot-reload, plugin system
