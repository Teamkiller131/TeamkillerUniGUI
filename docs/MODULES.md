# CMake modules & backends

All options are set **before** `add_subdirectory(TeamkillerUniGUI)` (or in your preset).

## Core (default ON)

| Option | Description |
|--------|-------------|
| `UNIGUI_MODULE_WIDGETS` | Widget library (required for normal apps) |
| `UNIGUI_MODULE_FONTS` | Font manager + fallback chains |
| `UNIGUI_MODULE_EVENTS` | `unigui::events::Bus` |
| `UNIGUI_MODULE_PLUGIN` | DLL plugin manager |
| `UNIGUI_MODULE_DSL` | Declarative `unigui::dsl` builders |
| `UNIGUI_MODULE_STYLING` | CSS-like `unigui::styling::Engine` |
| `UNIGUI_BUILD_TESTS` | GoogleTest suite |
| `UNIGUI_BUILD_EXAMPLES` | `examples/hello_unigui` |

## Optional (default OFF)

| Option | Headers / namespace | Extra vcpkg deps |
|--------|---------------------|------------------|
| `UNIGUI_MODULE_SQLITE` | `<unigui/sqlite/database.h>` | sqlite3 |
| `UNIGUI_MODULE_CONFIG` | `<unigui/config/config.h>` | cpptoml, nlohmann-json |
| `UNIGUI_MODULE_IPC` | `<unigui/ipc/ipc.h>` | zeromq |
| `UNIGUI_MODULE_NETWORK` | `<unigui/network/network.h>` | cpp-httplib, ixwebsocket |

When OFF, related headers are not compiled in; `unigui.h` guards them with `UNIGUI_HAS_*`.

## Backends (Windows)

| Option | `BackendType` | Notes |
|--------|---------------|-------|
| `UNIGUI_BACKEND_DX11` | `DX11` | **Default on Windows**, production |
| `UNIGUI_BACKEND_DX12` | `DX12` | Optional |
| GLFW + GL3 | `GLFW_GL3` | Default on non-Windows |
| SDL3 + Vulkan | preset `*-sdl3-vulkan-*` | Production |

```cpp
unigui::AppConfig cfg;
cfg.backend = unigui::BackendType::DX11;
```

## Minimal consumer (widgets only)

```cmake
set(UNIGUI_MODULE_DSL OFF)
set(UNIGUI_MODULE_STYLING OFF)
set(UNIGUI_MODULE_PLUGIN OFF)
set(UNIGUI_MODULE_EVENTS OFF)
set(UNIGUI_BUILD_TESTS OFF)
set(UNIGUI_BUILD_EXAMPLES OFF)
add_subdirectory(third_party/TeamkillerUniGUI)
```

## vcpkg features (standalone UniGUI repo)

See root `vcpkg.json`. Parent projects should list imgui/glfw3/freetype/implot/spdlog in **their** `vcpkg.json` when using submodules — see [INTEGRATION.md](../INTEGRATION.md).

## Sub-module documentation in README

EventBus, styling, plugins, fonts, config, SQLite, IPC are summarized in the main [README.md](../README.md#api-overview). Widget-level API remains in [WIDGET_API.md](WIDGET_API.md).
