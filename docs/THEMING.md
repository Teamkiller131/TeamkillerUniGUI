# Theming & CSS Styling Guide

TeamkillerUniGUI (v4.7.0) ships a layered theming system on top of Dear ImGui's
`ImGuiStyle`. It is built from four composable layers:

1. **Geometry tokens** — unified rounding / spacing / border sizes shared by every theme (`theme::StyleTokens`).
2. **Colour palette** — the Dark/Light base palettes plus 13 named registry presets.
3. **Surface material** — a translucency/"glass" layer applied on top of any palette (`theme::SurfaceStyle`).
4. **Accent & semantic tokens** — a single base accent that drives the `accent → hover → active` interaction colours plus the semantic `success/warning/danger/info` palette (`theme::ColorTokens`).

On top of those, an optional **CSS-like styling engine** (`unigui::styling::Engine`) lets you load a
`.css` file from disk, hot-reload it while the app runs, and apply CSS-style selectors to the global
ImGui style.

All public APIs live in the `unigui` namespace (with `unigui::theme` and `unigui::styling`
sub-namespaces) and are pulled in by the umbrella header:

```cpp
#include <unigui/unigui.h>   // brings in theme.h, surface_style.h, color_tokens.h,
                             // style_scope.h, presets/registry.h, styling/style_engine.h
```

---

## API at a glance

### Top-level theme API (`<unigui/theme/theme.h>`, namespace `unigui`)

| Symbol | Signature | Purpose |
|--------|-----------|---------|
| `ThemePreset` | `enum class { Dark, Light }` | Base palette selector. |
| `ThemeConfig` | struct (see below) | Full theme configuration passed to `ApplyTheme`. |
| `ApplyTheme` | `void ApplyTheme(const ThemeConfig& config)` | Apply colours + geometry + surface + DPI + queue font rebuild. |
| `DetectDPIScale` | `float DetectDPIScale(void* native_window)` | System DPI factor; 1.0 on failure. |
| `LoadDefaultFont` | `void LoadDefaultFont(float size_pixels, const char* ttf_path = nullptr)` | Load embedded font + CJK/emoji merge. |
| `HasPendingFontRebuild` | `bool HasPendingFontRebuild()` | True if a font-atlas rebuild is queued. |
| `ApplyPendingFontRebuild` | `void ApplyPendingFontRebuild()` | Rebuild the atlas (call before `NewFrame`). |
| `SetFontScale` / `GetFontScale` | `void SetFontScale(float)` / `float GetFontScale()` | Wrap `io.FontGlobalScale`. |
| `ExportThemeJSON` | `std::string ExportThemeJSON()` | Serialise current ImGui colours to JSON. |
| `ImportThemeJSON` | `bool ImportThemeJSON(const std::string& json)` | Load colours from JSON. |
| `GetBackdropColor` | `ImVec4 GetBackdropColor()` | Opaque framebuffer clear colour for the active theme + surface. |
| `GetColorTokens` | `const theme::ColorTokens& GetColorTokens()` | Active accent/semantic palette. |
| `GetSemanticColor` | `ImVec4 GetSemanticColor(theme::Semantic role)` | Active colour for a semantic role. |
| `BeginTextWrap` / `EndTextWrap` | `void BeginTextWrap(float width = 0.0f)` / `void EndTextWrap()` | Text-wrap scope. |

### Surface material (`<unigui/theme/surface_style.h>`, namespace `unigui::theme`)

| Symbol | Signature / values |
|--------|--------------------|
| `SurfaceStyle` | `enum class { Solid, Glass, Frosted, Acrylic, Minimal }` (default `Glass`) |
| `SurfaceTokens` | struct of per-material alpha multipliers + rim/border overrides |
| `SurfacePreset` | `SurfaceTokens SurfacePreset(SurfaceStyle)` |
| `ApplySurfaceStyle` | `void ApplySurfaceStyle(ImGuiStyle&, const SurfaceTokens&)` and `…(ImGuiStyle&, SurfaceStyle)` |
| `ActiveSurfaceStyle` | `SurfaceStyle ActiveSurfaceStyle()` |
| `SurfaceStyleName` | `const char* SurfaceStyleName(SurfaceStyle)` |
| `AllSurfaceStyles` | `const std::array<SurfaceStyle, 5>& AllSurfaceStyles()` |
| `BackdropColor` | `ImVec4 BackdropColor(const ImVec4& windowBg, SurfaceStyle)` |

### Accent & semantic tokens (`<unigui/theme/color_tokens.h>`, namespace `unigui::theme`)

| Symbol | Signature / values |
|--------|--------------------|
| `Semantic` | `enum class { Accent, Success, Warning, Danger, Info, Up, Down }` |
| `Polarity` | `enum class { GreenUp, RedUp }` (default `RedUp`) |
| `ColorTokens` | struct `{ accent, accent_hover, accent_active, success, warning, danger, info; bool dark }` |
| `DeriveColorTokens` | `ColorTokens DeriveColorTokens(const ImVec4& accent, bool dark)` |
| `ApplyColorTokens` | `void ApplyColorTokens(ImGuiStyle&, const ColorTokens&)` and `…(ImGuiStyle&, const ImVec4& accent, bool dark)` |
| `ActiveColorTokens` | `const ColorTokens& ActiveColorTokens()` |
| `GetSemanticColor` | `ImVec4 GetSemanticColor(Semantic)` |
| `GetDirectionColor` | `ImVec4 GetDirectionColor(double value, ImVec4 flat = …, double eps = 0.0)` |
| `SetPolarity` / `GetPolarity` | `void SetPolarity(Polarity)` / `Polarity GetPolarity()` |
| `AccentFromStyle` | `ImVec4 AccentFromStyle(const ImGuiStyle&)` |
| `StyleIsDark` | `bool StyleIsDark(const ImGuiStyle&)` |

### Geometry tokens (`<unigui/theme/style_tokens.h>`, namespace `unigui::theme`)

| Symbol | Signature |
|--------|-----------|
| `StyleTokens` | struct of rounding / border / spacing / padding fields |
| `ApplyStyleTokens` | `void ApplyStyleTokens(ImGuiStyle&, const StyleTokens& = StyleTokens{})` |
| `Clamp01`, `Lighten`, `Darken`, `WithAlpha` | colour helpers |
| `AccentHover`, `AccentActive` | accent derivations |

### Theme registry (`<unigui/theme/presets/registry.h>`, namespace `unigui::theme`)

| Symbol | Signature |
|--------|-----------|
| `ThemePreset` (registry struct) | `{ std::string name; std::string description; std::function<void(ImGuiStyle&)> apply; }` |
| `ThemeRegistry::Instance` | `static ThemeRegistry& Instance()` |
| `ThemeRegistry::Register` | `void Register(ThemePreset)` |
| `ThemeRegistry::Get` | `const ThemePreset* Get(const std::string& name) const` |
| `ThemeRegistry::List` | `std::vector<std::string> List() const` |
| `ThemeRegistry::Apply` | `bool Apply(const std::string& name)` |
| `ThemeRegistry::GetCurrentThemeName` | `std::string GetCurrentThemeName() const` |
| `ThemeRegistry::SetOnChange` | `void SetOnChange(std::function<void(const std::string&)>)` |
| `RegisterAllThemes` | `void RegisterAllThemes()` |

### CSS styling engine (`<unigui/styling/style_engine.h>`, namespace `unigui::styling`)

| Symbol | Signature |
|--------|-----------|
| `Engine::Instance` | `static Engine& Instance()` |
| `Engine::LoadFile` | `int LoadFile(const std::string& path)` → rules parsed |
| `Engine::ReloadIfChanged` | `bool ReloadIfChanged()` → true if a reload happened |
| `Engine::WatchedFile` | `const std::string& WatchedFile() const` |
| `Engine::Clear` | `void Clear()` |
| `Engine::Parse` | `int Parse(const std::string& css)` → rules parsed |
| `Engine::Apply` | `void Apply(type, className="", idName="", hovered=false, active=false, focused=false, disabled=false, childIndex=-1)` |
| `Engine::ApplyAll` | `void ApplyAll()` |
| `Engine::SetVar` / `GetVar` | `void SetVar(name, value)` / `std::string GetVar(name) const` |
| `Engine::EvaluateMedia` | `void EvaluateMedia(float viewWidth, float viewHeight, bool darkMode = true)` |

---

## 1. Applying a theme

`ApplyTheme` is the single entry point that wires up the whole theme stack. It must be called
**after** the ImGui context exists (i.e. after `unigui::Init`). It applies, in order: the unified
geometry tokens, the Dark or Light colour palette, auto-derived table colours, the accent/semantic
tokens, the surface material, the backdrop colour, DPI scaling, and (if the font size changed) a
queued font-atlas rebuild.

### `ThemeConfig`

```cpp
struct ThemeConfig {
    ThemePreset preset = ThemePreset::Dark;
    float dpi_scale = 0.0f;          // 0 = auto-detect from system DPI
    float font_size = 16.0f;         // logical px at 96 DPI (scaled by auto-DPI)
    const char* font_path = nullptr; // nullptr = auto-detect system CJK font
    bool emoji_fallback = true;      // auto-load system emoji font
    theme::SurfaceStyle surface = theme::SurfaceStyle::Glass;
};
```

`ThemePreset` has exactly two values — `ThemePreset::Dark` and `ThemePreset::Light`. (Note: there is
a *separate* `ThemePreset` **struct** inside `unigui::theme` used by the registry — see §5. The
fully-qualified `unigui::ThemePreset` enum and `unigui::theme::ThemePreset` struct do not collide.)

### Minimal app applying a preset + surface

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.width  = 1000;
    cfg.height = 700;
    cfg.title  = "Theming demo";
    if (!unigui::Init(cfg))
        return 1;

    // Dark palette with the frosted-glass surface material (the project default),
    // 16 logical px font, auto-detected DPI.
    unigui::ThemeConfig theme;
    theme.preset  = unigui::ThemePreset::Dark;
    theme.surface = unigui::theme::SurfaceStyle::Glass;
    theme.font_size = 16.0f;
    unigui::ApplyTheme(theme);

    while (!unigui::ShouldClose()) {
        unigui::NewFrame();           // applies any queued font rebuild internally
        ImGui::Begin("Hello");
        ImGui::Text("Themed window");
        ImGui::End();
        unigui::Render();             // built-in loop clears to GetBackdropColor()
    }
    unigui::Shutdown();
    return 0;
}
```

You can also pre-seed the theme through `AppConfig::theme`:

```cpp
unigui::AppConfig cfg;
cfg.theme = {unigui::ThemePreset::Light, /*dpi_scale=*/0.0f, /*font_size=*/18.0f};
cfg.theme.surface = unigui::theme::SurfaceStyle::Acrylic;
```

### Switching theme at runtime

`ApplyTheme` is cheap and idempotent — call it again whenever the user changes a setting. A font-size
change is deferred: `ApplyTheme` flags a rebuild that the built-in loop drains in `NewFrame` via
`ApplyPendingFontRebuild`. If you drive ImGui yourself, call it manually:

```cpp
if (unigui::HasPendingFontRebuild())
    unigui::ApplyPendingFontRebuild();  // BEFORE ImGui::NewFrame(), never mid-frame
```

---

## 2. Surface materials (the "glass" layer)

A surface material multiplies the alpha of the window/child/popup/frame/menubar/title surfaces and
optionally brightens the border into a "rim" — it composes on top of *any* palette without touching
hue. `SurfaceStyle` has five values:

| Value | Look |
|-------|------|
| `SurfaceStyle::Solid` | Flat, fully opaque — classic ImGui look, no translucency. |
| `SurfaceStyle::Glass` | Frosted glass / glassmorphism — translucent surfaces + bright rim. **(default)** |
| `SurfaceStyle::Frosted` | Heavier translucency than Glass, stronger rim. |
| `SurfaceStyle::Acrylic` | Fluent-style acrylic — translucent with a firmer tint and border. |
| `SurfaceStyle::Minimal` | Near-opaque, borderless and quiet. |

`ApplyTheme` applies the surface for you from `ThemeConfig::surface`. To enumerate materials for a
picker, use `AllSurfaceStyles()` and `SurfaceStyleName()`:

```cpp
for (unigui::theme::SurfaceStyle s : unigui::theme::AllSurfaceStyles()) {
    bool selected = (s == theme.surface);
    if (ImGui::RadioButton(unigui::theme::SurfaceStyleName(s), selected)) {
        theme.surface = s;
        unigui::ApplyTheme(theme);   // re-apply so backdrop + alphas update
    }
}
```

Query the active material anywhere with `unigui::theme::ActiveSurfaceStyle()`.

### Applying a material directly to a style

If you build your own palette and want to layer a material onto it (e.g. inside a registry preset),
call `ApplySurfaceStyle` **after** setting colours and **before** `ImGuiStyle::ScaleAllSizes` so the
border-size overrides get DPI-scaled too:

```cpp
ImGuiStyle& style = ImGui::GetStyle();
// ... populate style.Colors ...
unigui::theme::ApplySurfaceStyle(style, unigui::theme::SurfaceStyle::Frosted);
style.ScaleAllSizes(dpi);
```

The per-material multipliers live in `SurfaceTokens` (returned by `SurfacePreset(style)`): the alpha
fields multiply the matching surface colour's existing alpha (`window_alpha`, `child_alpha`,
`popup_alpha`, `frame_alpha`, `menubar_alpha`, `title_alpha`, `border_alpha`), `rim_highlight` /
`rim_strength` lighten the border into a glass rim, and the `*_border` fields (negative sentinel =
"leave the geometry token untouched") override the window/frame/popup border sizes.

---

## 3. The backdrop-clear requirement (important)

Translucent surface materials (`Glass` / `Frosted` / `Acrylic`) **reveal whatever is painted behind
the ImGui windows**. If the framebuffer is cleared to black, glass surfaces read as muddy and the
material does not "pop". The toolkit therefore derives an opaque tinted **backdrop colour** from the
window background on every `ApplyTheme()` call, and the renderer must clear to it.

`GetBackdropColor()` returns that colour. The built-in render loop already does the right thing — in
`src/app/app.cc` it issues, every frame:

```cpp
ImVec4 bg = unigui::GetBackdropColor();
g_renderer->SetClearColor(bg.x, bg.y, bg.z, bg.w);
```

**If you use `unigui::Init` / `unigui::NewFrame` / `unigui::Render` (or `unigui::Run` / `RunApp`),
this is automatic — you do not need to do anything.**

If you wire ImGui into your *own* renderer/backend instead, you must replicate it: never hard-code a
black/opaque clear. Clear to `GetBackdropColor()` each frame:

```cpp
// Custom render loop — replicate the backdrop clear yourself.
ImVec4 bg = unigui::GetBackdropColor();
myRenderer.SetClearColor(bg.x, bg.y, bg.z, bg.w);   // the RendererBackend interface
myRenderer.RenderDrawData(ImGui::GetDrawData());     // then clear + render ImGui draw data
```

(`SetClearColor(float r, float g, float b, float a)` and `RenderDrawData(ImDrawData*)` are the two
relevant `unigui::RendererBackend` virtuals.)

For opaque materials (`Solid` / `Minimal`) the backdrop simply mirrors the window background, so the
same code is always correct. Before the first `ApplyTheme()` call, `GetBackdropColor()` defaults to
the Dark window background.

---

## 4. Customising accent & semantic colours

Every theme expresses its interactive colours (`CheckMark`, `SliderGrab`, `SeparatorActive`,
`ResizeGrip*`, `DragDropTarget`, `NavHighlight`, `DockingPreview`, `TextSelectedBg`) from a single
base accent, and exposes a consistent semantic palette. The active palette is queryable via
`unigui::GetColorTokens()` / `theme::ActiveColorTokens()`, and individual roles via
`unigui::GetSemanticColor(role)`.

`Semantic` roles: `Accent`, `Success`, `Warning`, `Danger`, `Info`, `Up`, `Down`. `Info` follows the
accent; `Up`/`Down` resolve through the active `Polarity`.

### Overriding the accent

`ApplyTheme` derives the accent from the palette, but you can re-derive and re-apply the accent slots
afterwards with `ApplyColorTokens`:

```cpp
unigui::ApplyTheme(theme);                       // base palette + default accent

// Re-tint every accent-driven slot to a custom magenta accent.
ImGuiStyle& style = ImGui::GetStyle();
ImVec4 accent(0.85f, 0.30f, 0.70f, 1.0f);
bool dark = (theme.preset == unigui::ThemePreset::Dark);
unigui::theme::ApplyColorTokens(style, accent, dark);  // updates the active token set too
```

`ApplyColorTokens` only rewrites the accent-semantic interaction slots — it never touches surface
alphas, geometry, or a preset's independent Button/Header/Tab/Plot palette, so it composes cleanly.

### Using semantic colours in your widgets

```cpp
ImVec4 ok   = unigui::GetSemanticColor(unigui::theme::Semantic::Success);
ImVec4 bad  = unigui::GetSemanticColor(unigui::theme::Semantic::Danger);
ImGui::TextColored(ok,  "Saved");
ImGui::TextColored(bad, "Failed");
```

### Financial up/down polarity

For trading UIs, `Up`/`Down` resolve through a process-wide polarity. The default is `RedUp` (the CN
market convention: up = red); call `SetPolarity` for Western markets:

```cpp
unigui::theme::SetPolarity(unigui::theme::Polarity::GreenUp);  // Western: up = green

double pnl = -1.5;
ImVec4 c = unigui::theme::GetDirectionColor(pnl);   // Down colour for a negative value
ImGui::TextColored(c, "%+.2f", pnl);
```

### Geometry tokens

To change rounding/spacing/borders globally, fill a `StyleTokens` and call `ApplyStyleTokens`
(colours are left untouched, so order relative to the palette doesn't matter):

```cpp
unigui::theme::StyleTokens geo;
geo.window_rounding = 10.0f;
geo.frame_rounding  = 6.0f;
geo.window_padding  = ImVec2(16.0f, 16.0f);
unigui::theme::ApplyStyleTokens(ImGui::GetStyle(), geo);
```

### Scoped, temporary overrides

For a one-off, per-section colour/var change that auto-reverts, use the RAII `StyleScope` from
`<unigui/theme/style_scope.h>`:

```cpp
{
    unigui::StyleScope scope;
    scope.PushColor(ImGuiCol_Text, ImVec4(1, 0.8f, 0.2f, 1));
    scope.PushVar(ImGuiStyleVar_FrameRounding, 12.0f);
    ImGui::Button("Highlighted");
}   // all pushes popped here automatically
```

`StyleScope` is move-only and exposes both a scalar `PushVar(ImGuiStyleVar, float)` and a vector
`PushVar(ImGuiStyleVar, const ImVec2&)` overload.

---

## 5. Named theme presets (the registry)

Beyond Dark/Light, 13 hand-tuned palettes are registered by `RegisterAllThemes()`. Each is a
`unigui::theme::ThemePreset` struct (`name`, `description`, `apply`) stored in the singleton
`ThemeRegistry`.

The registered names are exactly:

```
Material Dark, Material Light, Fluent Dark, Fluent Light, Dracula, Nord,
Gruvbox, Catppuccin Mocha, Solarized Dark, Solarized Light, TokyoNight,
OneDark, Everforest
```

`ThemeRegistry::Apply(name)` runs the preset's `apply`, re-derives the table colours and the
accent/semantic tokens from that preset's own accent (via `AccentFromStyle` / `StyleIsDark`), records
it as current, and fires the optional `onChange` callback.

> `unigui::Init()` already calls `RegisterAllThemes()` for you, so the registry is populated as soon
> as the app is initialised — the explicit call below is optional. `RegisterAllThemes()` is
> idempotent (it just re-inserts into the registry map), so calling it again is harmless.

```cpp
#include <unigui/unigui.h>

unigui::theme::RegisterAllThemes();     // optional — Init() already did this

auto& reg = unigui::theme::ThemeRegistry::Instance();
reg.SetOnChange([](const std::string& name) {
    std::printf("Theme changed to %s\n", name.c_str());
});

std::vector<std::string> names = reg.List();   // for a picker
reg.Apply("Nord");                             // switch theme by name
std::string active = reg.GetCurrentThemeName();
```

> Registry presets set colours and geometry but do **not** apply a surface material or update the
> backdrop colour themselves. If you mix registry presets with translucent surfaces, apply the
> surface and refresh the backdrop yourself (e.g. call `ApplySurfaceStyle` then derive
> `BackdropColor(style.Colors[ImGuiCol_WindowBg], surface)`), or drive everything through
> `ApplyTheme` and only use the registry for the palette.

The **`examples/theme_demo`** app cycles through every registered theme automatically (`--frames N`,
`--interval N`, `--manual`) — a good reference for the registry workflow. (It reads the registry via
`ThemeRegistry::Instance().List()`, relying on the registration done in `Init()`.)

---

## 6. Theme export / import (JSON)

The current ImGui colour table can be round-tripped to a compact JSON object keyed by ImGui colour
slot name (`"WindowBg":[r,g,b,a]`, …). Only colours are serialised — not geometry, surface, or
semantic tokens.

```cpp
// Export the live palette and persist it.
std::string json = unigui::ExportThemeJSON();
std::ofstream("my_theme.json") << json;

// Later: load it back. Returns true; unknown / missing keys are skipped safely.
std::ifstream f("my_theme.json");
std::string json2((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
unigui::ImportThemeJSON(json2);
```

`ImportThemeJSON` is forgiving: any key it doesn't recognise is ignored, and a slot whose array is
absent keeps its current value. The parser is hand-written and non-throwing.

---

## 7. The CSS-like styling engine

`unigui::styling::Engine` is an optional layer (guarded by `UNIGUI_HAS_STYLING`) that parses a
CSS-ish stylesheet and maps its properties onto the global `ImGuiStyle`. It is a **process-wide
singleton** reached via `Engine::Instance()`.

The supported syntax is a pragmatic subset of CSS:

- **Selectors** parse a `Type.class#id:pseudo` shape — e.g. `Window`, `Button.primary`,
  `#submit`, `Button:hover`. The pseudo-class is split on `:`, the id on `#`, the class on `.`, and
  the remainder is the type.
- **Pseudo-classes** recognised by `Apply`: `:hover`, `:active`, `:focus`, `:disabled`,
  `:first-child`. (`:last-child` and `:checked` need per-widget context and currently never match.)
- **Variables**: `$name` references resolve against variables set with `SetVar` (a value beginning
  with `$` is substituted at parse time).
- **`@media` blocks**: `@media (min-width: 800px) { … }`, plus `max-width`, `min-height`, and
  `prefers-color-scheme: dark|light`, evaluated by `EvaluateMedia`.

### Properties

Most properties map a `#RRGGBB` value to a single ImGui colour slot. The recognised colour keys
include: `bg`, `frame-bg`, `text`, `text-disabled`, `text-secondary`, `border`, `border-color`,
`border-hover`, `button`, `button-hover`, `button-active`, `bg-hover`, `bg-active`, `bg-secondary`,
`bg-tertiary`, `header`, `header-hover`, `header-active`, `header-bg`, `header-text`, `title-bg`,
`title-bg-collapsed`, `title-text`, `accent-hover`, `separator`, `separator-hover`, `scrollbar-bg`,
`scrollbar-grab`, `scrollbar-grab-hover`, `tab`, `tab-hover`, `tab-active`, `tab-unfocused`,
`popup-bg`, `dock-bg`, `modal-dim`, `nav-highlight`, `drag-drop-target`.

Special, non-colour-slot properties:

| Property | Effect |
|----------|--------|
| `accent` | Tints `CheckMark`, `SliderGrab`, `ResizeGrip`, `PlotHistogram`, `TabActive` from one colour. |
| `rounding` / `border-radius` | Sets all rounding (window/frame/grab/tab/child/popup/scrollbar). |
| `border-radius-top-left/-top-right/-bottom-left/-bottom-right` | Window / Frame / Grab / Tab rounding individually. |
| `border-width` | `WindowBorderSize`. |
| `padding`, `padding-x`, `padding-y` | Window/frame padding. |
| `spacing`, `spacing-x`, `spacing-y` | Item spacing. |
| `indent` | `IndentSpacing`. |
| `scrollbar-size` | `ScrollbarSize`. |
| `alpha` / `opacity` | `style.Alpha`. |
| `min-width`, `min-height` | `WindowMinSize`. (`max-width`/`max-height` reset that axis to 0 — ImGui has no max.) |
| `columns` | `ColumnsMinSpacing`. |
| `box-shadow`, `blur`, `bg-gradient` / `gradient` | Effect hints (partially mapped to `BorderShadow` / rounding). |
| `font-size`, `font-family`, `font-weight`, `text-align`, `line-height`, `transition`, `animation` | Parsed but applied elsewhere or stored as hints (no direct `ImGuiStyle` change). |

> The engine writes into the **global** `ImGuiStyle`. CSS rules therefore act as a global theme
> override; they are not scoped per-widget on their own. Use `ApplyAll()` to apply every rule once
> after (re)loading.

### Loading and applying a stylesheet

```cpp
#include <unigui/styling/style_engine.h>

auto& css = unigui::styling::Engine::Instance();

// Optionally define variables before loading.
css.SetVar("brand", "#e94560");

int rules = css.LoadFile("assets/app.css");   // returns number of rules parsed
css.ApplyAll();                                // push every rule into ImGui's global style
```

Example `app.css`:

```css
Window {
    bg: #14141a;
    rounding: 8;
    padding: 14;
}
Button {
    accent: #4a90d9;
    button: #2a2a32;
    button-hover: #34343e;
}
#submit {
    button: #2e8b57;
}
@media (min-width: 1200px) {
    Window { padding: 20; }
}
```

You can also parse from a string instead of a file with `Parse(css)`, and clear all parsed state
(rules, `@media` blocks, and variables) with `Clear()` while keeping the watched path.

### Hot-reloading from disk

`LoadFile` records the file path and its modification time. Each frame, call `ReloadIfChanged()`: if
the file's mtime changed it clears previously-parsed rules/variables, re-parses the file, and returns
`true` — your cue to re-apply. This is the "edit the `.css` and watch it update live" dev workflow.

```cpp
#include <unigui/unigui.h>
#ifdef UNIGUI_HAS_STYLING
#include <unigui/styling/style_engine.h>
#endif

int main() {
    unigui::AppConfig cfg;
    cfg.title = "CSS hot-reload";
    if (!unigui::Init(cfg))
        return 1;

    unigui::ApplyTheme(unigui::ThemeConfig{});   // base theme first

#ifdef UNIGUI_HAS_STYLING
    auto& css = unigui::styling::Engine::Instance();
    css.LoadFile("assets/app.css");
    css.ApplyAll();
#endif

    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

#ifdef UNIGUI_HAS_STYLING
        // Poll the watched stylesheet once per frame — the heart of hot-reload.
        if (css.ReloadIfChanged())
            css.ApplyAll();           // file changed on disk → re-apply the new rules
#endif

        ImGui::Begin("Live CSS");
        ImGui::Text("Edit assets/app.css and save to see changes.");
        ImGui::End();

        unigui::Render();
    }
    unigui::Shutdown();
    return 0;
}
```

`WatchedFile()` returns the currently-watched path (empty if `LoadFile` was never used), handy for a
status line.

### Responsive `@media`

`@media` rules are stored separately and applied only when their condition matches. Drive them from
your viewport size each frame:

```cpp
ImVec2 vp = ImGui::GetIO().DisplaySize;
bool dark = (theme.preset == unigui::ThemePreset::Dark);
unigui::styling::Engine::Instance().EvaluateMedia(vp.x, vp.y, dark);
```

---

## 8. The Theme Editor example

**`examples/theme_editor`** is a live theme-authoring playground that ties everything together:

- switch `ThemePreset` (Dark/Light), `SurfaceStyle`, font size, and accent live (re-calling
  `ApplyTheme`);
- export the active palette to JSON and re-import it (`ExportThemeJSON` / `ImportThemeJSON`);
- hot-reload a CSS stylesheet from disk — run it with `--css path/to/style.css` and edit the file
  while it runs (`styling::Engine::ReloadIfChanged()` + `ApplyAll()`).

Both example apps are headless-friendly via `--frames N` (render N frames and exit), making them
usable in CI smoke runs:

```bash
./build/examples/theme_editor/theme_editor --frames 10 --css assets/app.css
./build/examples/theme_demo/theme_demo --frames 60 --interval 30
```

---

## Cheat sheet

```cpp
// 1. Apply a preset + surface (auto backdrop-clear if you use the built-in loop)
unigui::ThemeConfig t;
t.preset = unigui::ThemePreset::Dark;
t.surface = unigui::theme::SurfaceStyle::Glass;
unigui::ApplyTheme(t);

// 2. Custom backend? clear to the backdrop every frame
ImVec4 bg = unigui::GetBackdropColor();
renderer.SetClearColor(bg.x, bg.y, bg.z, bg.w);

// 3. Custom accent
unigui::theme::ApplyColorTokens(ImGui::GetStyle(), ImVec4(0.85f,0.3f,0.7f,1), /*dark=*/true);

// 4. Named registry theme (Init() already called RegisterAllThemes())
unigui::theme::ThemeRegistry::Instance().Apply("Nord");

// 5. Export / import
auto json = unigui::ExportThemeJSON();
unigui::ImportThemeJSON(json);

// 6. CSS hot-reload
auto& css = unigui::styling::Engine::Instance();
css.LoadFile("app.css"); css.ApplyAll();
if (css.ReloadIfChanged()) css.ApplyAll();
```
