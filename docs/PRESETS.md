# UI Presets — a decent app in ~30 lines

_Module: `UNIGUI_MODULE_PRESETS` (default **ON**) · namespace `unigui::presets` · headers `<unigui/presets/*.h>`_

![preset_demo — AppShell scaffold hosting the Dashboard preset (dark theme)](assets/preset_demo.png)

*`examples/preset_demo`: AppShell (menu bar, sidebar, status bar, Ctrl+P command palette) hosting Dashboard behind a LoginPage gate — about sixty lines of application code.*

The preset layer sits **above** the widget library: prefab compositions of existing
widgets with sensible theming, layout, and accessibility defaults, so a complete,
decent-looking application takes minutes, not days. Every preset:

- is a regular retained widget (`FluentWidget<T>` — chainable `With*`/`Add*` setters),
- is ID-safe (`PushID(name)`), theme-aware, and a11y-wired (tree registration +
  announcements on page/selection changes),
- **looks decent with nothing configured** beyond the constructor,
- has zero dependencies outside the widget layer (bindings are `std::function`
  getter/setter pairs — trivially attachable to `config::Store`, a `dsl::Store`, or
  plain variables).

Runnable reference: [`examples/preset_demo`](../examples/preset_demo/main.cc) — an app
shell with dashboard, data browser, settings, and live log pages, assembled entirely
from presets.

## AppShell — application chrome

Menu bar, optional toolbar slot, sidebar page navigation, content area, status bar —
one widget, full-viewport.

```cpp
static presets::AppShell shell("shell", "My Tool");
shell.WithMenus({{"File", {{"Quit", [] { /*...*/ }}}}})
     .AddPage("Dashboard", [] { dash.Render(); })
     .AddPage("Settings",  [] { settings.Render(); })
     .WithStatus("Ready");
// per frame:
shell.Render();
```

| API | Notes |
|---|---|
| `AddPage(label, content)` / `AddPage(icon, label, content)` | sidebar entry + page body |
| `WithMenus(std::vector<MenuDef>)` | reuses the `MenuBar` data model; rendered inside the shell window |
| `WithToolbar(fn)` | optional row under the menu bar |
| `SetActivePage(i)` / `GetActivePage()` / `WithOnPageChange(fn)` | switching announces "<label> page" to a11y |
| `SetStatus(text)` / `WithSidebarWidth(w)` | live status line, sidebar sizing |
| `WithCommandPalette()` / `AddCommand(id, title, action)` | built-in **Ctrl+P** palette; every page auto-registers as "Go to <label>", app commands via `AddCommand` |

## SettingsPage — schema-driven settings

Declare rows; the page renders aligned label/control pairs, with a section list when
there is more than one section. `set()` fires only on an actual change; text commits on
Enter.

```cpp
settings.AddSection("General")
        .AddToggle("Dark mode", [] { return dark; }, [](bool v) { dark = v; })
        .AddInt("Rows", [] { return rows; }, [](int v) { rows = v; }, 1, 100)
        .AddCombo("Theme", {"Dark", "Light"}, [] { return t; }, [](int v) { t = v; })
        .AddText("User", [] { return user; }, [](const std::string& v) { user = v; })
        .AddAction("Reset", [] { /*...*/ });
```

Rows added before any `AddSection` land in an implicit "General" section.

## Dashboard — responsive card grid

Cards wrap to the available width (`columns = max(1, avail / minCardWidth)`).

```cpp
dash.AddMetric("Throughput", [] { return FormatRate(); })            // MetricCard-backed
    .AddMetric("P&L", [] { return pnl; }, [] { return delta; })      // with ▲/▼ delta
    .AddCard("Anything", [] { /* arbitrary ImGui/unigui content */ });
```

## MasterDetail — list + detail browser

```cpp
data.WithItems({"EURUSD", "XAUUSD"})
    .WithDetail([](int i) { DrawInstrument(i); })
    .WithEmptyText("Pick an instrument")
    .WithOnSelect([](int i) { /* ... */ });
```

Composes `Splitter` + `ListView`; selection announces to a11y; `SetItems` clamps a
now-invalid selection; `SetSelected` is programmatic (does not fire `WithOnSelect`).

## LogConsole — filterable log panel

Ring-buffered (default 2000 lines), severity-colored via theme semantics, substring
filter + per-level toggles, clipper-virtualized, auto-scroll that sticks to the bottom.

```cpp
logs.Append(presets::LogConsole::Level::Info, "connected");
logs.Append(presets::LogConsole::Level::Error, "feed dropped");
```

**Not thread-safe** — `Append` from the UI thread only (marshal worker-thread logs via
`core/main_thread.h`).

## LoginPage — sign-in / connect scaffold

A centred credentials card: optional logo, title, username, password (with the
PasswordInput visibility toggle + strength meter), optional remember-me, a full-width
submit, and a status/error line. Enter submits; a busy state disables the form while the
host authenticates.

```cpp
login.WithTitle("Connect to feed")
     .WithOnSubmit([&](const std::string& user, const std::string& pass, bool remember) {
         login.SetBusy(true);                 // disables the form, shows "Signing in…"
         Authenticate(user, pass, remember);  // async → SetBusy(false) + SetStatus(...)
     });                                      //         on the UI thread
```

The password value is **never** exposed to the accessibility layer or announced
(`PasswordInput` reports presence only); the status line announces assertively on error.

## WizardFlow — validated multi-step flow

Step indicator (dots + "Step i of N — title"), the current step's content, and a
`[Cancel] [Back] [Next | Finish]` button row. Adds what the bare `Wizard` widget lacks:
**per-step validation gating**, localizable labels, and a11y announcements per transition.

```cpp
flow.AddStep("Welcome", [] { ImGui::TextUnformatted("hi"); })
    .AddStep("License", [&] { ImGui::Checkbox("I agree", &agreed); },
             [&] { return agreed; })            // Next disabled until the gate passes
    .WithOnFinish([] { Install(); });
```

`Next()`/`Finish` respect the current step's gate; `GoTo(i)` is a deliberate jump that
skips gates. Empty flow shows a friendly hint.

## Module gating

`UNIGUI_MODULE_PRESETS` defaults ON and requires `UNIGUI_MODULE_WIDGETS` (it composes
`ListView`/`Splitter`/`MetricCard`). With the module off, the library builds without the
`unigui/presets/` headers and `UNIGUI_HAS_PRESETS` is undefined (the umbrella header
gates on it). Tests live under `tests/presets/`.
