# TeamkillerUniGUI Documentation

> **Version 3.8.9** · C++23 · Dear ImGui 1.92 · [GitHub](https://github.com/Teamkiller131/TeamkillerUniGUI)

English | [中文索引](README_zh.md)

## Start here

| Document | Description |
|----------|-------------|
| [GETTING_STARTED.md](GETTING_STARTED.md) | Install, build, first app (`RunApp`), backends |
| [EXAMPLES.md](EXAMPLES.md) | Cookbook: composition, theme, tables, charts, threading, DSL |
| [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) | **One minimal example per widget** (93 entries) |
| [WIDGET_API.md](WIDGET_API.md) | **Full API reference** — signatures, fluent API, TreeView & CascadingCombo inline |
| [API_INDEX.md](API_INDEX.md) | Master index: widgets + `im` + DSL + core |
| [MODULES.md](MODULES.md) | Optional CMake modules (DSL, SQLite, IPC, …) |
| [TRADING.md](TRADING.md) | Trading toolkit: financial formatting + models (widgets upcoming) |
| [API_STABILITY.md](API_STABILITY.md) | API contract: semver, stability tiers, deprecation policy |
| [../INTEGRATION.md](../INTEGRATION.md) | Embed as **Git submodule** + vcpkg (recommended) |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Build/CRT/CI FAQ (not in README) |

Redirects (content merged into WIDGET_API): [TREEVIEW.md](TREEVIEW.md), [CASCADINGCOMBO.md](CASCADINGCOMBO.md).

## API surface (what “93 widgets” means)

| Layer | Location | Count |
|-------|----------|------:|
| Retained widgets + layout | `include/unigui/widgets/` | **93** catalog entries |
| Immediate mode | `unigui::im` in `im/im.h` | **22** functions |
| Declarative DSL (optional) | `unigui::dsl` in `dsl/dsl.h` | **18** builders |
| App / theme / scopes | `app/`, `theme/`, `core/` | **15+** |

The README badge counts **retained** components only. Use [API_INDEX.md](API_INDEX.md) for the full picture.

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
| **`unigui::dsl`** | Declarative UI trees (optional module) | [WIDGET_API — DSL](WIDGET_API.md#declarative-dsl-uniguidsl), [MODULES.md](MODULES.md) |

## Example program

After building with `UNIGUI_BUILD_EXAMPLES=ON`:

```text
build/<preset>/examples/hello_unigui/hello_unigui.exe --frames 300
```

Source: [`examples/hello_unigui/main.cc`](../examples/hello_unigui/main.cc) — `RunApp`, `Window`, `Panel`, fluent `Button`, `unigui::im`.

## Repository

- **Clone**: `git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git`
- **Issues / PRs**: GitHub only
