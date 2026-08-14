# TeamkillerUniGUI Documentation

> **Version 4.9.0** · C++23 · Dear ImGui 1.92 · [GitHub](https://github.com/Teamkiller131/TeamkillerUniGUI)

English | [中文索引](README_zh.md)

TeamkillerUniGUI is a C++23 Dear ImGui framework: a backend abstraction, a
dark+light theme engine, 92 retained widgets, a themed immediate layer
(`unigui::im`), a declarative DSL + reactive **component framework**, a flexbox
layout system, CSS-like styling, and optional modules (SQLite, config, IPC,
networking, trading).

## Start here

| Document | Description |
|----------|-------------|
| [GETTING_STARTED.md](GETTING_STARTED.md) | Prerequisites, building per platform, and three "hello world" first-apps (immediate, widget, framework) |
| [ARCHITECTURE.md](ARCHITECTURE.md) | The big picture: the layered architecture and a **"which layer should I use?"** guide |

## Building applications — the framework

| Document | Description |
|----------|-------------|
| [FRAMEWORK.md](FRAMEWORK.md) | **The golden path** — Components + reactive State, composition, Store/Watch, effects, Navigator, the inspector, the `Custom` escape hatch |
| [REACTIVE.md](REACTIVE.md) | Reactive layer: `Observable` / `Computed` / `Bind` / `State` / `Store` and widget data binding |
| [LAYOUT.md](LAYOUT.md) | Layout system: the flexbox solver (`SolveFlex` / `SolveFlexWrap`) and the `Layout::FlexRow` container |

## Layer references

| Document | Description |
|----------|-------------|
| [IM_API.md](IM_API.md) | The immediate layer `unigui::im` — full grouped reference (~250 functions) |
| [DSL.md](DSL.md) | The declarative DSL (view language): builders, nodes, `Render`, `Custom` |
| [THEMING.md](THEMING.md) | Theme engine (presets, surface styles, tokens) + the CSS styling engine & hot-reload |
| [BACKENDS.md](BACKENDS.md) | The backend abstraction (platforms × renderers) and the application loop (`AppConfig`/`Init`/`Run`) |
| [ACCESSIBILITY.md](ACCESSIBILITY.md) | The `unigui::a11y` layer — focus tracking, per-frame element tree, live announcements, keyboard nav, inspector, and writing a screen-reader bridge |

## Widgets & API reference

| Document | Description |
|----------|-------------|
| [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) | **One minimal example per widget** (95 entries) |
| [WIDGET_API.md](WIDGET_API.md) | **Full widget API reference** — signatures, fluent API (TreeView & CascadingCombo inline) |
| [API_INDEX.md](API_INDEX.md) | Master index: widgets + `im` + DSL + framework + reactive + layout + core |
| [EXAMPLES.md](EXAMPLES.md) | Cookbook: composition, theme, tables, charts, threading, DSL |

## Modules, integration & policy

| Document | Description |
|----------|-------------|
| [MODULES.md](MODULES.md) | Optional modules — EventBus, plugins, Config (TOML/JSON), SQLite, IPC, networking, fonts |
| [TRADING.md](TRADING.md) | Trading toolkit: financial formatting + models |
| [PRESETS.md](PRESETS.md) | UI presets — prefab app scaffolds: a decent app in ~30 lines |
| [../INTEGRATION.md](../INTEGRATION.md) | Embed as a **Git submodule** + vcpkg (recommended) |
| [API_STABILITY.md](API_STABILITY.md) | API contract: semver, stability tiers, deprecation policy |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Build / CRT / CI FAQ |

Redirects (content merged into WIDGET_API): [TREEVIEW.md](TREEVIEW.md), [CASCADINGCOMBO.md](CASCADINGCOMBO.md).

## The five layers

UniGUI is built bottom-up; you pick the layer that fits the task (full guide in
[ARCHITECTURE.md](ARCHITECTURE.md)):

| Layer | When to use | Reference |
|-------|-------------|-----------|
| **Component framework** (`unigui::dsl`) | Whole applications — screens, shared state, navigation | [FRAMEWORK.md](FRAMEWORK.md) |
| **Declarative DSL** (`unigui::dsl`) | Describe a view tree without per-frame plumbing | [DSL.md](DSL.md) |
| **Retained widgets** (`unigui::`) | Stateful controls with validation/undo — often leaves inside a component | [WIDGET_API.md](WIDGET_API.md) |
| **Immediate** (`unigui::im`) | One-off controls and custom drawing (the escape hatch via `dsl::Custom`) | [IM_API.md](IM_API.md) |
| **Raw `ImGui::`** | Anything the wrapper doesn't cover, inside `dsl::Custom` | — |

Cross-cutting: **reactive** ([REACTIVE.md](REACTIVE.md)), **layout**
([LAYOUT.md](LAYOUT.md)), **theming** ([THEMING.md](THEMING.md)).

## Header entry points

```cpp
#include <unigui/unigui.h>           // Umbrella: widgets + im + dsl + theme + app + reactive + layout
#include <unigui/app/app.h>          // AppConfig / Init / Run (the application loop)
#include <unigui/im/im.h>            // Immediate-mode helpers (unigui::im::)
#include <unigui/dsl/component.h>    // dsl::Component, State, Host, Custom, DrawInspector
#include <unigui/dsl/app.h>          // dsl::Store, Navigator
#include <unigui/core/observable.h>  // Observable, Computed, Bind, Subscription
#include <unigui/core/flex_layout.h> // SolveFlex / SolveFlexWrap
#include <unigui/core/scope.h>       // WindowScope, IDScope, DisabledScope, …
#include <unigui/widgets/button.h>   // …or include only the widgets you need
```

## Example programs

After building with `UNIGUI_BUILD_EXAMPLES=ON` (the default), examples accept
`--frames N` to render N frames headless and exit:

```text
build/<preset>/examples/framework_demo/framework_demo --frames 10   # the framework idiom
build/<preset>/examples/unigui_showcase/unigui_showcase --frames 10  # all 92 widgets
build/<preset>/examples/hello_unigui/hello_unigui --frames 10        # minimal
```

- [`examples/framework_demo`](../examples/framework_demo/main.cc) — a multi-screen app built entirely in the component framework.
- [`examples/unigui_showcase`](../examples/unigui_showcase/main.cc) — every widget across ten tabs.

## Repository

- **Clone**: `git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git`
- **Issues / PRs**: GitHub.
