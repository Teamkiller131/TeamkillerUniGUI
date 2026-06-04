# TeamkillerUniGUI Documentation

> **Version 3.5.0** · C++23 · Dear ImGui 1.92 · [GitHub](https://github.com/Teamkiller131/TeamkillerUniGUI)

English | [中文索引](README_zh.md)

## Start here

| Document | Description |
|----------|-------------|
| [GETTING_STARTED.md](GETTING_STARTED.md) | Install, build, first app (`RunApp`), backends |
| [EXAMPLES.md](EXAMPLES.md) | Copy-paste recipes (widgets, theme, tables, charts, threading) |
| [WIDGET_API.md](WIDGET_API.md) | **Full API reference** — every widget, methods, snippets |
| [API_INDEX.md](API_INDEX.md) | Alphabetical widget & module index → links into WIDGET_API |
| [TREEVIEW.md](TREEVIEW.md) | TreeView deep dive (composite rows, `spans`, renderers) |
| [CASCADINGCOMBO.md](CASCADINGCOMBO.md) | CascadingCombo layout & width |
| [MODULES.md](MODULES.md) | Optional CMake modules (DSL, SQLite, IPC, …) |
| [../INTEGRATION.md](../INTEGRATION.md) | Embed as **Git submodule** + vcpkg (recommended) |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Build/CRT/CI FAQ (not in README) |

## Header entry points

```cpp
#include <unigui/unigui.h>           // Umbrella: widgets + theme + app
#include <unigui/app/app.h>          // Init / NewFrame / Render / RunApp
#include <unigui/im/im.h>            // Immediate-mode helpers (unigui::im::)
#include <unigui/core/scope.h>       // WindowScope, IDScope, DisabledScope
#include <unigui/core/main_thread.h> // InvokeOnMainThread
#include <unigui/widgets/button.h>   // Or include only what you need
```

## Three API layers

| Layer | When to use | Doc section |
|-------|-------------|-------------|
| **`unigui::im`** | One-off controls, layout helpers | [WIDGET_API §2](WIDGET_API.md#2-core-concepts) |
| **Retained widgets** | Stateful UI, validation, tables | [WIDGET_API §3–15](WIDGET_API.md#3-containers--layout) |
| **`unigui::dsl`** | Declarative UI trees (optional module) | [WIDGET_API §2](WIDGET_API.md#declarative-dsl-uniguidsl), [MODULES.md](MODULES.md) |

## Example program

After building with `UNIGUI_BUILD_EXAMPLES=ON`:

```text
build/<preset>/examples/hello_unigui/hello_unigui.exe --frames 300
```

Source: [`examples/hello_unigui/main.cc`](../examples/hello_unigui/main.cc) — `RunApp`, `Window`, `Panel`, fluent `Button`, `unigui::im`.

## Repository

- **Clone**: `git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git`
- **Issues / PRs**: GitHub only
