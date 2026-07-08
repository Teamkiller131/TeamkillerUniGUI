# UniGUI Widget API Reference

> **Version**: 4.8.0 (C++23) · **Widgets**: 95 · **Backend**: Dear ImGui (docking + multi-viewport)
>
> **Documentation index**: [docs/README.md](README.md) · **Alphabetical index**: [API_INDEX.md](API_INDEX.md) · **Cookbook**: [EXAMPLES.md](EXAMPLES.md) · **Per-widget examples**: [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md)
>
> UniGUI is a modern C++23 wrapper around Dear ImGui. Every widget lives in `namespace unigui`.
> Each constructor takes a `std::string name` as its first argument — this becomes the ImGui ID and the
> library calls `PushID`/`PopID` for you automatically, so identical labels never collide.

---

## Table of Contents

1. [Getting Started](#1-getting-started)
2. [Core Concepts](#2-core-concepts)
3. [Containers & Layout](#3-containers--layout)
4. [Text & Display](#4-text--display)
5. [Buttons & Actions](#5-buttons--actions)
6. [Text Input](#6-text-input)
7. [Numeric Input](#7-numeric-input)
8. [Selection & Pickers](#8-selection--pickers)
9. [Lists, Tables & Trees](#9-lists-tables--trees)
10. [Navigation](#10-navigation)
11. [Dialogs & Feedback](#11-dialogs--feedback)
12. [Forms & Properties](#12-forms--properties)
13. [Charts & Domain Widgets](#13-charts--domain-widgets)
14. [Loading & Skeleton](#14-loading--skeleton)
15. [Utilities](#15-utilities)
16. [Dedicated Component Guides](#16-dedicated-component-guides)

---

## 1. Getting Started

### Minimal app — one call

`RunApp` performs `Init` + the render loop + `Shutdown`, returning `0` on success or `1` if init failed:

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "My App";
    // cfg.backend = unigui::BackendType::DX11; // default on Windows
    return unigui::RunApp(cfg, [] {
        ImGui::ShowDemoWindow();   // raw ImGui works and is auto-themed
    });
}
```

Pass an optional `maxFrames` to stop after N frames (handy for CI / screenshots):

```cpp
return unigui::RunApp(cfg, myUiCallback, /*maxFrames=*/10);
```

### Manual loop

When you need setup/teardown between frames:

```cpp
unigui::AppConfig cfg;
if (!unigui::Init(cfg)) return 1;
while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    myUi();                 // create + Render() your widgets here
    unigui::Render();
}
unigui::Shutdown();
```

### Top-level API

| Function | Description |
|----------|-------------|
| `bool Init(const AppConfig& cfg)` | Create window + backend. Returns `false` on failure. |
| `void Shutdown()` | Tear down backend + window. |
| `bool NewFrame()` | Begin a frame (polls events). |
| `void Render()` | End + present the frame. |
| `bool ShouldClose()` | `true` when the window is closing. |
| `void Run(callback, maxFrames = 0)` | Drive the loop with a per-frame callback. |
| `int RunApp(cfg, callback, maxFrames = 0)` | `Init` + `Run` + `Shutdown` in one call. |
| `void* GetNativeWindowHandle()` | Native OS window handle (HWND etc.). |

`AppConfig` fields: `int width`, `int height`, `const char* title`, `ThemeConfig theme`, `BackendType backend`.

---

## 2. Core Concepts

### Create once, render every frame

Construct widgets **once** (outside the loop, usually as `shared_ptr`), then call `Render()` on each every frame:

```cpp
auto btn = std::make_shared<unigui::Button>("btn_ok", "OK");
unigui::Run([&] {
    btn->Render();
    if (btn->WasClicked()) doSave();
});
```

### Automatic ID safety

You never call `ImGui::PushID`/`PopID`. The unique `name` you pass to each constructor becomes the ID,
so two buttons with the same visible label but different names never conflict:

```cpp
auto ok     = std::make_shared<unigui::Button>("btn_ok",     "OK");
auto okAgain= std::make_shared<unigui::Button>("btn_ok2",    "OK"); // no collision
```

### Fluent configuration (`With*`)

Every widget inherits chainable wrappers from the base `Widget` class:

```cpp
btn->WithTooltip("Ctrl+S — Save")
   .WithEnabled(dirty)
   .WithShadow();
```

Available on all widgets: `WithTooltip`, `WithEnabled`, `WithVisible`, `WithUserData`,
`WithAccessibleName`, `WithAccessibleDescription`, `WithMinSize`, `WithMaxSize`, `WithShadow`.
Base setters: `Show/Hide/IsVisible`, `SetTooltip`, `SetEnabled/IsEnabled`, `SetFocused/IsFocused`,
`SetAccessibleName/Description`, `SetMinSize/SetMaxSize`, `SetShadow`, `SetUserData/GetUserData`.

Widgets that derive from the CRTP base `FluentWidget<Derived>` (e.g. `Button`) keep the **derived
type** through the chain, so base helpers and widget-specific helpers compose:

```cpp
btn->WithTooltip("Ctrl+S — Save")  // Widget helper → Button&
   .WithPrimary()                  // Button helper → Button&
   .WithOnClick([]{ save(); });    // Button helper → Button&
```

`Button` adds: `WithLabel`, `WithVariant`, `WithPrimary`, `WithDanger`, `WithSuccess`, `WithSize`, `WithOnClick`.

### Immediate mode (`unigui::im`) vs retained mode

For simple, stateless controls, skip the `shared_ptr`/name/`Render()` ceremony and call a
themed immediate-mode free function from `<unigui/im/im.h>`:

```cpp
namespace im = unigui::im;
if (im::Button("Save", im::ButtonVariant::Primary)) save();
im::Checkbox("Enabled", &enabled);
im::SliderFloat("Gain", &gain, 0.f, 1.f);
im::InputText("Name", &name);           // bound to a std::string
im::Combo("Mode", &mode, {"Fast", "Safe"});
im::SameLine(); im::Text("ok");
```

Functions: `Button`, `SmallButton`, `Text`, `TextWrapped`, `TextDisabled`, `TextColored`,
`BulletText`, `LabelText`, `Checkbox`, `RadioButton`, `SliderFloat`, `SliderInt`, `DragFloat`,
`DragInt`, `InputInt`, `InputFloat`, `InputText`, `InputTextMultiline`, `Combo`, and layout
helpers `SameLine`, `NewLine`, `Spacing`, `Separator`, `SeparatorText`, `Dummy`, `Indent`,
`Unindent`, `Bullet`.

**Choosing a layer**: use `unigui::im` for one-off, stateless controls; use the retained-mode
widget classes when you need persistent state, validation, undo/redo or serialization. The two
layers coexist — immediate-mode functions live in `unigui::im` to avoid clashing with the
retained-mode widget *classes* of the same name in `unigui`.

### RAII scopes

Move-only guards in `<unigui/core/scope.h>` balance ImGui's `Begin*/Push*` with their matching
`End*/Pop*` automatically:

```cpp
if (unigui::WindowScope w{"Settings"}) {
    unigui::IDScope id{"row"};
    unigui::DisabledScope d{readOnly};
    im::Button("Apply");
}   // End() / PopID() / EndDisabled() run automatically, in reverse order
```

Available: `WindowScope`, `ChildScope`, `IDScope`, `DisabledScope`, `GroupScope`, `TabBarScope`,
`TabItemScope` (plus the existing `StyleScope` in `<unigui/theme/style_scope.h>`).

### Factory helpers

`<unigui/core/make.h>` trims the `std::make_shared` boilerplate and can auto-name widgets:

```cpp
auto btn = unigui::Make<unigui::Button>("save", "Save");   // explicit name
auto lbl = unigui::MakeNamed<unigui::Label>("Read-only");  // auto unique name
```

### Declarative DSL (`unigui::dsl`)

`<unigui/dsl/dsl.h>` builds a UI as a tree of value-type builder calls and renders it through the
themed `unigui::im` layer. Stateful controls bind to an external variable via pointer or keep their
state in the retained node, so re-`Render()`-ing the same tree preserves user input:

```cpp
using namespace unigui::dsl;
bool enabled = true; float gain = 0.5f;

auto ui = Window("Demo", VBox({
    Text("Welcome!"), Separator(),
    HBox({ Button("Save", ButtonVariant::Primary, []{ save(); }),
           Button("Exit", []{ std::exit(0); }) }),
    CheckBox("Enabled", &enabled),
    SliderFloat("Gain", &gain, 0.f, 1.f),
    If([&]{ return enabled; }, Text("…running")),
    For(3, [](int i){ return Label("Row " + std::to_string(i)); }),
}));
Render(ui);  // each frame
```

Builders: `Window`, `VBox`, `HBox`, `Label`, `Text`, `TextWrapped`, `TextDisabled`, `BulletText`,
`Button` (optional `ButtonVariant`), `CheckBox`, `SliderFloat`, `InputText` (each bound or
node-stated), `Separator`, `Spacing`, `If`, `IfElse`, `For`.

### Window

Top-level application window that owns panels and can persist its layout.

```cpp
Window(std::string name, std::string title);
```

| Method | Purpose |
|--------|---------|
| `void AddPanel(std::shared_ptr<Panel> panel)` | Dock a panel into the window. |
| `void RemovePanel(const std::string& name)` | Remove a panel by name. |
| `void SetSize(float w, float h)` / `void SetPosition(float x, float y)` | Geometry. |
| `void SetMenuBarEnabled(bool)` / `bool HasMenuBar() const` | Menu bar toggle. |
| `void SetOnClose(std::function<void()>)` | Close handler. |
| `void SetCloseToTray(bool)` | Minimize-to-tray instead of close. |
| `void SetDropCallback(std::function<void(std::vector<std::string>)>)` | OS file-drop paths. |
| `std::string SaveLayout() const` / `void RestoreLayout(const std::string& json)` | Persist docking layout. |

```cpp
auto win = std::make_shared<unigui::Window>("main", "Dashboard");
win->SetMenuBarEnabled(true);
win->AddPanel(std::make_shared<unigui::Panel>("left", "Explorer"));
```

### Panel

Collapsible titled container; render children inside its content callback.

```cpp
Panel(std::string name, std::string title);
void SetContentCallback(std::function<void()> cb);
void SetWrapEnabled(bool on);
void SetTitle(std::string); const std::string& GetTitle() const;
bool IsCollapsed() const;
```

### PanelBox

Dark panel with a title bar and optionally tinted content area.

```cpp
PanelBox(std::string name, std::string title);
void SetTintColor(ImU32 color);
void SetContentCallback(std::function<void()> cb);
void SetTitle(std::string); const std::string& GetTitle() const;
```

### GroupBox

Titled box that groups arbitrary content.

```cpp
GroupBox(std::string name, std::string title);
void SetTitle(std::string);
void SetContentCallback(std::function<void()> cb);
```

### Card

Elevated surface with shadow, rounded corners, optional title/footer.

```cpp
Card(const std::string& title = "");
enum Variant { Elevated, Outlined, Filled };
void SetTitle(const std::string&);
void SetContent(std::function<void()>); void SetFooter(std::function<void()>);
void SetVariant(Variant);
void SetShadow(bool); void SetShadowRadius(float);
void SetPadding(float); void SetBorderColor(ImU32); void SetBorderRadius(float);
```

### CollapsingHeader

Expandable section with a content callback.

```cpp
CollapsingHeader(std::string name, std::string label, bool default_open = false);
bool IsOpen() const; void SetOpen(bool);
void SetContentCallback(std::function<void()> cb);
void SetOnToggle(std::function<void(bool)> fn);
```

### TabWidget

Tabbed interface with closable tabs and keyboard shortcuts.

```cpp
TabWidget(std::string name);
struct TabPage { std::string name; std::string label; std::function<void()> content_callback; bool closable = false; };
void AddTab(TabPage page); void RemoveTab(const std::string& tab_name);
int GetActiveTab() const; void SetActiveTab(int index);
void SetTabShortcut(int index, ImGuiKey key);
```

```cpp
auto tabs = std::make_shared<unigui::TabWidget>("editor");
tabs->AddTab({"file1", "main.cpp", []{ ImGui::TextUnformatted("..."); }, true});
```

### Splitter

Resizable two-panel split (drag the divider).

```cpp
Splitter(std::string name, Orientation orientation = Horizontal, float split = 0.5f);
enum Orientation { Horizontal, Vertical };
float GetSplit() const;
void SetContentA(std::function<void()> cb); void SetContentB(std::function<void()> cb);
```

### MultiSplitter

N-panel resizable layout with drag handles.

```cpp
MultiSplitter(std::string name, Orientation ori = Horizontal); // enum { Horizontal, Vertical }
void AddPanel(float ratio, std::function<void()> content);
std::vector<float> GetRatios() const; void SetRatios(const std::vector<float>&);
```

### ScrollArea

Scrollable content region.

```cpp
ScrollArea(std::string name, float width = 0, float height = 200);
void SetContentCallback(std::function<void()> cb);
void SetSize(float w, float h);
```

### DockSpace (`space.h`)

```cpp
DockSpace(std::string name);   // host ImGui docking; Render() each frame
```

### HBox / VBox (`layout.h`) — RAII helpers

Scope-based horizontal/vertical layout. Construct on the stack; items added until it goes out of scope.

```cpp
{ unigui::HBox row(8.0f);          // 8px spacing between items
  btnA->Render();
  unigui::HBox::VSeparator();
  btnB->Render(); }                // row ends at brace
```

### Separator / Space

```cpp
Separator(std::string name, std::string label = "");  void SetLabel(std::string);
```

---

## 4. Text & Display

### Label

```cpp
Label(std::string name, std::string text = "");
void SetText(std::string); const std::string& GetText() const;
```

### RichText

Inline styled spans (bold/italic/color).

```cpp
RichText(std::string name, std::string text = "");
struct RichTextSpan { std::string text; bool bold=false; bool italic=false; ImVec4 color=ImVec4(1,1,1,1); };
void SetText(std::string); std::string GetText() const;
void SetSpans(std::vector<RichTextSpan>);
void AddSpan(std::string text, ImVec4 color, bool bold = false, bool italic = false);
```

```cpp
auto rt = std::make_shared<unigui::RichText>("rt");
rt->AddSpan("Error: ", ImVec4(1,0.3f,0.3f,1), /*bold=*/true);
rt->AddSpan("disk full", ImVec4(1,1,1,1));
```

### Markdown

Renders `#` headers, `**bold**`, `*italic*`, `- lists`, and `[links](url)`.

```cpp
Markdown(std::string name, std::string markdown = "");
void SetMarkdown(std::string md); const std::string& GetMarkdown() const;
void SetLinkCallback(std::function<void(const std::string& url)> cb);
void SetMaxWidth(float w);
```

### Image

```cpp
Image(std::string name, void* textureID = nullptr, float w = 0, float h = 0);
enum ScaleMode { Fit, Stretch, Original };
void SetTexture(void* tex, float w, float h);
void SetScaleMode(ScaleMode);
```

### ProgressBar

```cpp
ProgressBar(std::string name, float fraction = 0.0f);
enum State { Normal, Warning, Error };
void SetFraction(float); float GetFraction() const;
void SetState(State);
void SetOverlayText(std::string);
void SetGradient(float t1, ImU32 c1, float t2, ImU32 c2, ImU32 c3);
```

### Badge / Tag

```cpp
Badge(const std::string& label = "");
enum Variant { Dot, Count, Label };
void SetText(const std::string&); void SetVariant(Variant); void SetColor(ImU32); void SetCount(int);
```

```cpp
Tag(std::string name, std::string text, std::array<float,3> color = {0.2f,0.5f,1.0f});
void SetText(std::string); void SetColor(std::array<float,3>);
void SetRemovable(bool); bool RemoveClicked() const;
```

### StatusBar

```cpp
StatusBar(std::string name, std::string text = "");
void SetText(std::string); const std::string& GetText() const;
```

### StatusLamp

Glossy circular LED with glow and named states. See [StatusLamp tips](#statuslamp-glow).

```cpp
StatusLamp(std::string name, State state = Off);
enum State { Off, Running, Draft, Error, Warning, Paused };
void SetState(State); State GetState() const;
void SetTooltip(std::string); void SetRadius(float); void SetColor(ImU32 rgba);
void SetGlowEnabled(bool on);
```

### Gauge

Circular/radial progress dial — a ring or speedometer arc with a centre label.
Complements the linear `ProgressBar` for dashboards/KPI tiles.

```cpp
Gauge(std::string name, float value = 0.f);
void SetValue(float); float GetValue() const;
void SetRange(float min, float max); float GetFraction() const; // clamped 0..1
void SetRadius(float); void SetThickness(float);
void SetSweepDegrees(float deg);     // 360 = full ring, 270 = open-bottom dial
void SetTrackColor(ImU32); void SetFillColor(ImU32);
void SetShowPercent(bool); void SetCenterLabel(std::string);
// Fluent: WithValue/WithRange/WithRadius/WithThickness/WithSweepDegrees/
//         WithTrackColor/WithFillColor/WithShowPercent/WithCenterLabel
```

---

## 5. Buttons & Actions

### Button

```cpp
Button(std::string name, std::string label);
enum ColorVariant { Default, Primary, Danger, Success };
enum Size { Small, Medium, Large };
bool WasClicked() const;
void SetLabel(std::string); const std::string& GetLabel() const;
void SetColorVariant(ColorVariant); void SetSize(Size);
void SetOnClick(std::function<void()> fn);
```

```cpp
auto save = std::make_shared<unigui::Button>("save", "Save");
save->SetColorVariant(unigui::Button::Primary);
save->SetOnClick([]{ doSave(); });
```

### IconButton

```cpp
IconButton(std::string name, std::string icon, std::string label = "");
bool WasClicked() const;
void SetIcon(std::string); void SetLabel(std::string); void SetEnabled(bool);
```

### ImageButton

```cpp
ImageButton(std::string name, std::string label = "");
void SetImage(ImTextureID textureID, float width, float height);
void SetLabel(std::string); const std::string& GetLabel() const;
bool WasClicked() const;
void SetEnabled(bool); bool IsEnabled() const;
void SetFramePadding(float x, float y);
```

### Hyperlink

```cpp
Hyperlink(std::string name, std::string label, std::string url = "");
void SetURL(std::string); void SetLabel(std::string); bool WasClicked() const;
```

### ToggleSwitch

Animated boolean switch (a `ValueWidget<bool>`).

```cpp
ToggleSwitch(std::string name, std::string label, bool on = false);
bool IsOn() const; void SetOn(); void SetOff(); void Toggle();
void SetOnChange(/* from ValueWidget<bool> */);
```

### CheckBox

```cpp
CheckBox(std::string name, std::string label, bool checked = false);
bool IsChecked() const; void SetChecked(bool);
const std::string& GetLabel() const;
```

### RadioGroup

```cpp
RadioGroup(std::string name, std::vector<std::string> options, int selected = 0);
int GetSelected() const; void SetSelected(int index);
const std::vector<std::string>& GetOptions() const;
void SetOnChange(std::function<void(int)> callback);
```

### ToolBar

```cpp
ToolBar(std::string name);
void SetItems(std::vector<ToolBarItem> items);
```

### SegmentedControl

Compact single-select button group sharing one rounded frame (the iOS-style
`1D / 1W / 1M` selector). More compact than a `TabWidget`; the selection is
accent-highlighted and clicks fire `onChange`.

```cpp
SegmentedControl(std::string name, std::vector<std::string> segments = {});
void SetSegments(std::vector<std::string>); void AddSegment(std::string); void Clear();
void SetSelected(int); int GetSelected() const; std::string GetSelectedLabel() const;
void SetOnChange(std::function<void(int, const std::string&)>);
void SetSegmentPadding(float); void SetFillWidth(bool);
// Fluent: WithSegments/WithSelected/WithOnChange/WithSegmentPadding/WithFillWidth
```

### ToggleButton

Bistate action button (Start ⇄ Stop) — flips label + semantic colour by state,
with an enabled-predicate + disabled tooltip and an on-toggle callback. Distinct
from the boolean `ToggleSwitch`.

```cpp
ToggleButton(std::string name, std::string offLabel = "Start", std::string onLabel = "Stop");
bool IsOn() const; void SetOn(bool); bool WasToggled() const;
// Fluent: WithLabels(off,on)/WithColors(theme::Semantic off,on)/WithOnToggle(cb)/
//         WithEnabledPredicate(p)/WithDisabledTooltip(t)/WithButtonSize(w,h)
```

### ButtonGroup

Horizontal button cluster with Left / Right / Fill alignment — owns the
"measure + right-align N buttons" math; composes inside `MetricCard` headers.

```cpp
ButtonGroup(std::string name);
enum class Align { Left, Right, Fill };
ButtonGroup& AddButton(std::string label, std::function<void()> onClick);
ButtonGroup& AddTintedButton(std::string label, std::function<void()> onClick, theme::Semantic);
ButtonGroup& WithAlign(Align); ButtonGroup& WithButtonWidth(float); ButtonGroup& WithSpacing(float);
```

---

## 6. Text Input

### InputText

```cpp
InputText(std::string name, std::string label, std::string value = "", ImGuiInputTextFlags flags = 0);
void SetHint(std::string); void SetPassword(bool); void SetMultiline(bool); void SetReadOnly(bool);
```

### LineEdit

Single-line input with validation and undo/redo history.

```cpp
LineEdit(std::string name, std::string label, std::string value = "");
void SetValue(std::string); void SetPlaceholder(std::string);
void SetValidator(std::function<bool(const std::string&)> fn); bool HasError() const;
void SetPasswordMode(bool); void SetMultiline(bool); void SetReadOnly(bool); void SetMaxLength(int);
void Undo(); void Redo(); bool CanUndo() const; bool CanRedo() const;
int GetUndoDepth() const; int GetRedoDepth() const;
```

```cpp
auto email = std::make_shared<unigui::LineEdit>("email", "Email");
email->SetValidator([](const std::string& s){ return s.find('@') != std::string::npos; });
```

### MultiLine

```cpp
MultiLine(std::string name, std::string text = "", int maxLines = 10);
void SetText(std::string); std::string GetText() const;
void SetMaxLines(int); void SetEditable(bool);
void Undo(); void Redo(); bool CanUndo() const; bool CanRedo() const;
```

### PasswordInput

Masked input with a strength score (0–4) and show/hide toggle.

```cpp
PasswordInput(std::string name, std::string label, std::string value = "");
void SetValue(std::string); void SetShowStrength(bool); int GetStrengthScore() const;
```

### SearchBox

Search input with a filtered suggestion dropdown.

```cpp
SearchBox(std::string name, std::string hint = "Search...");
void SetItems(std::vector<std::string> items);
const std::string& GetQuery() const; std::vector<std::string> GetMatches() const;
void SetOnSelect(std::function<void(const std::string&)> fn);
void SetOnChange(std::function<void(const std::string&)> fn);
```

### CommandPalette

VS-Code-style (Ctrl+P) fuzzy-searchable command launcher, rendered as a centred
modal popup. Register commands once; the palette filters/ranks them as the user
types, runs the chosen command on Enter/click, then closes. Up/Down navigate,
Esc dismisses. The query → results → execute pipeline is exposed directly so it
is testable without a GL context.

```cpp
CommandPalette(std::string name = "command_palette");
struct Command { std::string id, title, category, shortcut; std::function<void()> action; bool enabled = true; };
CommandPalette& AddCommand(Command cmd);
CommandPalette& AddCommand(std::string id, std::string title, std::function<void()> action);
bool RemoveCommand(const std::string& id); void ClearCommands();
std::size_t CommandCount() const; bool HasCommand(const std::string& id) const;
void Open(); void Close(); void Toggle(); bool IsOpen() const;
CommandPalette& SetPlaceholder(std::string s); CommandPalette& SetMaxResults(int n);
void SetQuery(const std::string& q); const std::string& GetQuery() const;
std::vector<std::string> Matches() const;   // ranked ids, best first
bool Execute(const std::string& id);          // run + close; returns true if a command ran

// Reusable pure matcher behind the ranking:
namespace detail { bool FuzzyMatch(std::string_view pattern, std::string_view text, int& outScore); }
```

### FileDialog

In-ImGui file / folder picker (Dear ImGui ships no native dialog). Three modes,
rendered as a themed modal; navigation + path resolution are plain methods so
they are testable without a GL context.

```cpp
FileDialog(std::string name = "file_dialog");
enum class Mode { OpenFile, SaveFile, SelectFolder };
FileDialog& SetMode(Mode m); Mode GetMode() const;
FileDialog& SetDirectory(const std::string& dir); const std::string& GetDirectory() const;
FileDialog& SetFilters(std::vector<std::string> exts);   // {".csv",".txt"}; empty = all
FileDialog& SetFilename(const std::string& name);         // SaveFile field
FileDialog& SetTitle(std::string t); FileDialog& SetShowHidden(bool on);
void Open(); void Close(); bool IsOpen() const;
void SetOnConfirm(std::function<void(const std::string& path)> fn);
void SetOnCancel(std::function<void()> fn);
const std::string& GetSelectedPath() const;
// Headless-testable navigation/state:
std::vector<detail::DirEntry> Entries() const;
bool NavigateInto(const std::string& dirName); bool NavigateUp();
void SelectFile(const std::string& name); bool MatchesFilter(const std::string& filename) const;
std::string ResolvedPath() const;   // path Confirm() would produce ("" if incomplete)
std::string Confirm();              // record + fire onConfirm + close

// Reusable non-throwing filesystem core:
namespace detail {
  struct DirEntry { std::string name; bool isDir; std::uintmax_t size; };
  bool ExtensionMatches(const std::string& filename, const std::vector<std::string>& exts);
  bool ListDirectory(const std::string& dir, const std::vector<std::string>& exts,
                     bool showHidden, std::vector<DirEntry>& out);
}
```

---

## 7. Numeric Input

### InputInt / InputFloat

```cpp
InputInt(std::string name, std::string label, int value = 0, int min = 0, int max = 100);
void SetRange(int min, int max); void SetSuffix(std::string);

InputFloat(std::string name, std::string label, float value = 0.0f, float min = 0.0f, float max = 100.0f);
void SetRange(float min, float max); void SetFormat(const char* fmt); void SetSuffix(std::string);
```

### Slider\<T>

Templated slider; `GetValue()/SetValue()` inherited from `ValueWidget<T>`.

```cpp
Slider(std::string name, std::string label, T value = T{}, T min = T{}, T max = T{100});
void SetRange(T min, T max);
void SetFormat(const char* fmt); const char* GetFormat() const;
```

### SpinBox\<T>

```cpp
SpinBox(std::string name, std::string label, T val = T{}, T mn = T{}, T mx = T{100}, T step = T{1});
void SetRange(T min, T max); void SetStep(T step);
```

### DragFloat\<T> / DragInt\<T>

Drag-to-adjust numeric inputs.

```cpp
DragFloat(std::string name, std::string label, float value = 0.0f, float speed = 1.0f, float vmin = 0.0f, float vmax = 0.0f);
DragInt(std::string name, std::string label, int value = 0, float speed = 1.0f, int vmin = 0, int vmax = 0);
bool WasChanged() const;  // GetValue()/SetValue() inherited
```

### MultiHandleSlider

Multiple draggable handles on one bar (e.g. range/markers).

```cpp
MultiHandleSlider(std::string name);
struct SliderTick { int id = -1; float position = 0.f; ImU32 color = IM_COL32(14,165,233,255); };
enum Orientation { Horizontal, Vertical };
void SetTicks(const std::vector<SliderTick>&); const std::vector<SliderTick>& GetTicks() const;
void AddTick(SliderTick); void RemoveTick(int id);
void SetRange(float min, float max);
void SetOnTickChanged(std::function<void(int id, float newPos)> fn);
void SetTickOverlay(std::function<void(int id, int index, float x, float barWidth)> fn);
void SetCurrentMarker(float pos, ImU32 color);
```

---

## 8. Selection & Pickers

### ComboBox

```cpp
ComboBox(std::string name, std::string label, std::vector<std::string> items = {}, int selected = 0);
int GetSelectedIndex() const; void SetSelectedIndex(int);
const std::string& GetSelectedValue() const;
const std::vector<std::string>& GetItems() const; void SetItems(std::vector<std::string>);
void SetOnChange(std::function<void(int)> callback);
void SetEditable(bool); void SetSearchable(bool);
void SetItemIcon(int index, ImTextureID textureID); ImTextureID GetItemIcon(int index) const;
```

### MultiCombo

Multi-select dropdown with checkboxes.
Width auto-expands from the current/available text and reserves extra space for the arrow button.

```cpp
MultiCombo(std::string name, std::string label, std::vector<std::string> items = {});
const std::vector<std::string>& GetItems() const; void SetItems(std::vector<std::string>);
bool IsSelected(int index) const; void SetSelected(int index, bool sel);
std::vector<int> GetSelectedIndices() const; void SetSelectedIndices(const std::vector<int>&);
std::string GetPreview() const; void SetOnChange(std::function<void()> fn);
```

### CascadingCombo

Multi-level linked dropdowns (e.g. province → city → district). Header: `#include <unigui/widgets/cascadingcombo.h>`.

#### `Level` model

```cpp
struct Level {
    std::string              label;           // level caption (tooltip by default)
    std::vector<std::string> options;
    int                      selectedIndex = 0;
    float                    width = 0.f;   // per-level width (px); 0 = global width
};
```

#### API

```cpp
CascadingCombo(std::string name, std::vector<Level> levels = {});
enum class Layout { Vertical, Horizontal };

void SetLevels(std::vector<Level>);
void SetOptions(int level, std::vector<std::string> options);  // clamps index if shorter
int         GetSelectedIndex(int level) const;
std::string GetSelectedText(int level) const;

void SetLayout(Layout);
void SetHorizontal(bool on);              // Layout::Horizontal if true
void SetItemWidth(float width);             // global combo width (<=0 = auto)
void SetItemWidth(int level, float width);  // per-level override
void SetSpacing(float px);                  // horizontal gap (<0 = theme default)
void SetShowLabels(bool on);                // default false (label → tooltip)

CascadingCombo& WithLayout(Layout);
CascadingCombo& WithItemWidth(float);
CascadingCombo& WithSpacing(float);
CascadingCombo& WithShowLabels(bool);

void SetOnChanged(std::function<void(int level, int index)> fn);
```

Width priority: `Level::width` → `SetItemWidth(global)` → ImGui auto (+ arrow padding).

#### Example: horizontal three-level cascade

```cpp
auto cc = std::make_shared<unigui::CascadingCombo>("region");
cc->SetLevels({
    {"省", {"江苏", "浙江"}},
    {"市", {"南京", "苏州"}},
    {"区", {"玄武区", "鼓楼区"}},
});
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal)
   .WithItemWidth(120.f)
   .WithSpacing(8.f);
cc->SetItemWidth(0, 150.f);
cc->SetOnChanged([&](int level, int index) {
    if (level == 0) cc->SetOptions(1, CitiesOf(index));
    if (level == 1) cc->SetOptions(2, DistrictsOf(index));
});
cc->Render();
```

**Notes:** linkage logic is yours in `OnChanged`; `SetOptions` resets out-of-range indices to 0.

### Selectable

```cpp
Selectable(std::string name, std::string label, bool selected = false);
bool IsSelected() const; void SetSelected(bool); bool WasClicked() const;
void SetOnClick(std::function<void()> fn);
```

### ColorEdit / ColorPicker

```cpp
ColorEdit(std::string name, std::string label, float r=1, float g=1, float b=1, float a=1);
ImVec4 GetColor() const; void SetColor(float r, float g, float b, float a = 1.0f);
bool WasChanged() const; void SetOnChange(std::function<void(ImVec4)> fn);

ColorPicker(std::string name, std::string label, std::array<float,3> color = {0,0,0});
std::array<float,3> GetColor() const; void SetColor(std::array<float,3>);
std::array<float,4> GetColorRGBA() const; void SetColorRGBA(std::array<float,4>);
void SetAlpha(bool on); void SetOnChange(std::function<void(std::array<float,3>)> callback);
```

### DatePicker

```cpp
DatePicker(std::string name, std::string label);
std::array<int,3> GetDate() const;     // {year, month, day}
void SetDate(int y, int m, int d);
void SetOnChange(std::function<void(int,int,int)> cb);
```

### FilePath / DirPath

Native OS file/folder pickers.

```cpp
FilePath(std::string name, std::string label, Mode mode = Open);  // enum Mode { Open, Save }
std::string GetPath() const; void SetPath(std::string);
void SetFilter(std::string); void SetTitle(std::string); void SetMode(Mode);
void SetOnPathChanged(std::function<void(std::string)> cb);

DirPath(std::string name, std::string label);
std::string GetPath() const; void SetPath(std::string); void SetTitle(std::string);
void SetOnPathChanged(std::function<void(std::string)> cb);
```

---

## 9. Lists, Tables & Trees

### ListBox

```cpp
ListBox(std::string name, std::string label, std::vector<std::string> items = {}, int selected = -1);
int GetSelectedIndex() const; void SetSelectedIndex(int); std::string GetSelectedValue() const;
const std::vector<std::string>& GetItems() const; void SetItems(std::vector<std::string>);
void SetOnChange(std::function<void(int)> cb);
```

### ListView

Single- or multi-select scrollable list.

```cpp
ListView(std::string name, std::vector<std::string> items = {});
int GetSelected() const; void SetItems(std::vector<std::string>);
void SetOnSelect(std::function<void(int)> callback);
void SetMultiSelect(bool on); std::vector<int> GetSelectedItems() const;
```

### VirtualList

Virtual scrolling for 100k+ rows via `ImGuiListClipper`.

```cpp
VirtualList(std::string name, int itemCount = 0);
void SetItemCount(int n); int GetItemCount() const;
void SetItemGetter(std::function<std::string(int)> fn);
void SetOnSelect(std::function<void(int)> fn);
int GetSelected() const; void SetSelected(int idx);
```

```cpp
auto vl = std::make_shared<unigui::VirtualList>("rows", 100000);
vl->SetItemGetter([](int i){ return "Row #" + std::to_string(i); });
```

### Table

String-cell table with sorting, resizing, custom cell rendering, and CSV I/O.
Full guide for cell embedding & sorting is folded into this section.

```cpp
Table(std::string name, std::vector<std::string> columns);
enum class Alignment { Left, Center, Right };
using CellRenderer   = std::function<bool(int row, int col)>;  // return true if you drew the cell
using SortComparator = std::function<bool(const std::string& a, const std::string& b)>;
void AddRow(std::vector<std::string> row); void ClearRows();
int RowCount() const; int ColumnCount() const;
const std::string& CellText(int row, int col) const;
int GetSelectedRow() const; void SetOnSelect(std::function<void(int)> callback);
void SetSortable(bool on); void SetResizable(bool on);
void SortByColumn(int col, bool ascending = true);
void SetColumnAlignment(int col, Alignment);
void SetColumnUnit(int col, std::string unit);
void SetColumnSortComparator(int col, SortComparator cmp);
void SetCellRenderer(CellRenderer fn);
void SaveColumnWidths(); void RestoreColumnWidths();
std::string ExportCSV() const; bool ImportCSV(const std::string& csv);
```

```cpp
auto t = std::make_shared<unigui::Table>("grid", std::vector<std::string>{"Name","Qty"});
t->AddRow({"Apples", "12"});
t->SetSortable(true);                 // numeric-aware sort on click
t->SetColumnAlignment(1, unigui::Table::Alignment::Right);
t->SetColumnUnit(1, "手");            // renders 12手 and sorts 8 / 702 numerically
t->SetCellRenderer([&](int r, int c){ // embed a widget in a cell
    if (c == 1) { ImGui::ProgressBar(0.4f, ImVec2(-1,0)); return true; }
    return false;                     // false → fall back to text
});
```

### DataTable\<T>

Templated, high-performance table bound to your `std::vector<T>` data source: virtual scroll,
sorting, filtering, grouping, inline editing, checkbox columns, row/cell coloring.

```cpp
DataTable(std::string name, std::vector<ColumnDef> columns);
struct ColumnDef { std::string name; float width = 100.f; bool sortable = true; bool resizable = true; };
struct GroupInfo { std::string label; int startRow = 0, endRow = -1; bool expanded = true; int sortCol = -1; bool sortAsc = true; };
void SetDataSource(const std::vector<T>* data); const std::vector<T>* GetDataSource() const;
void SetCellFormatter(CellFormatter fmt);
void SetRowColor(RowColorFn); void SetCellColor(CellColorFn); void SetCellBold(CellBoldFn);
void SetSortCompare(int col, SortCompare); int GetSortColumn() const; bool GetSortAscending() const;
void SetMultiSelect(bool); int GetSelectedRow() const; std::vector<int> GetSelectedRows() const;
void SetOnSelect(SelectFn); void SetOnDoubleClick(DoubleClickFn); void SetOnSelectionChanged(std::function<void()>);
void SetContextMenu(std::function<void(int row)>); void SetRowClickCallback(std::function<void(int row)>);
void SetSelectedRow(int);
void SetColumnMinWidth(int col, float); void SetColumnAutoWidth(int col, bool); void SetColumnReorderable(bool);
void FlashRow(int row, ImU32 color, float duration);
void SetGroups(const std::vector<GroupInfo>&); void ToggleGroup(int idx);
void SetCellEditable(int col, bool); void SetOnCellCommit(CellCommitFn);
void SetCellCheckbox(int col, CellCheckboxFn);        // inline checkbox column
void SetFilterText(const std::string&); const std::string& GetFilterText() const; void SetFilterFn(FilterFn);
void SetVirtualScroll(bool); void SetStickyHeader(bool); void ScrollToRow(int row);
void SetFrozenColumns(int); int GetFrozenColumns() const;     // pinned leading cols
void SetEmptyText(std::string);                                // empty-state row
void SetCellCheckboxValue(int col, get, set);                  // non-UB checkbox (get/set)
void SetCellRenderer(int col, std::function<void(int row, const T&)>); // arbitrary cell (im:: editors)
```

### EditableDataGrid\<T>

A `DataTable<T>` with typed per-column **cell editors** rendered through the
stateless `unigui::im` layer — so per-row editors need **no `static std::map`
widget cache**. A `SetRowReadOnly` predicate freezes a row's editors to text.

```cpp
EditableDataGrid(std::string name, std::vector<DataTable<T>::ColumnDef> columns);
EditableDataGrid& SetComboColumn(int col, itemsFn, getSel, onChange);
EditableDataGrid& SetIntColumn(int col, getVal, onChange, int step = 1);
EditableDataGrid& SetFloatColumn(int col, getVal, onChange, const char* fmt = "%.2f");
EditableDataGrid& SetButtonColumn(int col, labelFn, onClick);
EditableDataGrid& SetRowReadOnly(std::function<bool(int,const T&)>);   // frozen-when-running
```

### BasketTicket\<T>

Editable basket / program-trading grid: a toolbar (Add / Remove / Import /
Submit) over an owned `EditableDataGrid<T>`, with validator-driven invalid-row
highlighting and **deferred** removal. Host owns CSV/XLSX parsing.

```cpp
BasketTicket(std::string name, std::vector<DataTable<T>::ColumnDef> columns);
EditableDataGrid<T>& Grid();
void SetRows(std::vector<T>); const std::vector<T>& Rows() const; void AddRow(T); void RemoveRow(int);
std::size_t RowCount() const; std::size_t ValidCount() const; bool AllValid() const;
BasketTicket& SetRowFactory(std::function<T()>); BasketTicket& SetValidator(std::function<bool(const T&)>);
BasketTicket& SetOnImportRequested(std::function<void()>);            // host owns the dialog
BasketTicket& SetOnSubmit(std::function<void(const std::vector<T>&)>);
```

### GroupedRiskTree

Hierarchical account/group risk view on `TreeView`: each node shows a
threshold-coloured utilisation bar, and parent rows roll children up via
Worst / Mean / Sum.

```cpp
GroupedRiskTree(std::string name);
enum class Rollup { Worst, Mean, Sum };
struct RiskNode { std::string label; double ratio = 0.0; std::vector<RiskNode> children; };
void SetData(RiskNode root); void SetRollup(Rollup); void SetThresholds(double warn, double danger);
void SetHideRoot(bool);
static double ComputeRatio(const RiskNode&, Rollup);   // pure, unit-tested
```

### TreeView

Hierarchical tree with multi-select, built-in composite rows, segmented label colors, and full custom row rendering. Header: `#include <unigui/widgets/treeview.h>`.

#### `TreeNode` / `TextSpan`

```cpp
struct TextSpan {
    std::string text;
    ImU32 color = 0;   // 0 = theme text color
};

struct TreeNode {
    std::string label;
    std::vector<TreeNode> children;
    bool expanded = false;

    std::string icon;
    std::string suffix;
    ImU32 labelColor = 0;
    ImU32 bgColor = 0;
    float progress = -1.f;      // 0..1 bar; <0 = hidden
    ImU32 progressColor = 0;

    // Non-empty → draw colored runs instead of single `label` tint
    std::vector<TextSpan> spans;
};
```

#### API

```cpp
TreeView(std::string name);
void SetRoot(TreeNode root);
const TreeNode& GetRoot() const;
void SetHideRoot(bool on);
void SetMultiSelect(bool on);
std::vector<int> GetSelectedNodes() const;  // depth-first ids; unstable if tree mutates

// Legacy: draw after default arrow+label inside the node
void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode&)> fn);

// Preferred: render the entire row (expand/select still managed by TreeView)
void SetRowRenderer(std::function<void(int id, int depth, const TreeNode& node, bool selected)> fn);
```

When `SetRowRenderer` is set, built-in `icon` / `suffix` / `progress` are **not** drawn (you own the row). Without it, those fields compose a composite row automatically.

#### Example A — built-in progress rows

```cpp
unigui::TreeNode group;
group.label = "期货账户";
group.children.push_back({"账户 A", {}, false, "", "92%", 0, 0, 0.92f, IM_COL32(46,209,94,255)});
auto tv = std::make_shared<unigui::TreeView>("accounts");
tv->SetRoot(std::move(group));
tv->SetHideRoot(true);
tv->Render();
```

#### Example B — `TextSpan` coloring (e.g. long/short)

```cpp
unigui::TreeNode leaf;
leaf.label = "IF2506";
leaf.spans = {
    {"IF2506 ", 0},
    {"多", IM_COL32(220, 60, 60, 255)},
    {" 12手", 0},
};
```

#### Example C — `SetRowRenderer`

```cpp
tv->SetRowRenderer([](int, int, const unigui::TreeNode& n, bool sel) {
    ImGui::TextUnformatted(n.label.c_str());
    ImGui::SameLine();
    ImGui::ProgressBar(n.progress < 0 ? 0.f : n.progress, ImVec2(120, 0));
    if (ImGui::SmallButton("平仓")) { /* ... */ }
});
```

**Notes:** built-in progress uses ~30% of remaining row width; use `SetRowRenderer` for fixed widths. Selection ids are per-frame — map to stable keys in your model.

---

## 10. Navigation

### MenuBar

```cpp
MenuBar(std::string name);
struct MenuItem { std::string label; std::function<void()> action; };
struct MenuDef  { std::string label; std::vector<MenuItem> items; };
void SetMenus(std::vector<MenuDef> menus);
```

```cpp
auto mb = std::make_shared<unigui::MenuBar>("menu");
mb->SetMenus({{"File", {{"Open", []{}}, {"Quit", []{ std::exit(0); }}}}});
```

### Breadcrumb

```cpp
Breadcrumb(std::string name);
void SetItems(std::vector<std::string> items);
int GetSelected() const; void SetOnSelect(std::function<void(int)> cb);
```

### Wizard

Multi-step flow with Next/Previous and finish/cancel callbacks.

```cpp
Wizard(std::string name, std::string title = "Wizard");
void AddStep(std::string name, std::string title, std::function<void()> renderFn); void Clear();
int GetCurrentStep() const; int GetStepCount() const;
void Next(); void Previous(); void GoTo(int step);
void SetOnFinish(std::function<void()> fn); void SetOnCancel(std::function<void()> fn);
```

---

## 11. Dialogs & Feedback

### Dialog

Modal with OK/Cancel.

```cpp
Dialog(std::string name, std::string title, std::string message);
void Open(); void Close(); bool IsOpen() const;
void SetButtons(std::string okText, std::string cancelText = "");
void SetOnOk(std::function<void()>); void SetOnCancel(std::function<void()>);
bool WasOkClicked() const;
```

### ConfirmDialog

Confirmation popup with optional danger styling.

```cpp
ConfirmDialog(std::string name);
void Open(); bool WasConfirmed() const; bool IsOpen() const;
void SetTitle(std::string); void SetMessage(std::string); void SetIcon(std::string);
void SetConfirmLabel(std::string); void SetCancelLabel(std::string);
void SetConfirmColor(ImU32); void SetDangerStyle(bool on);
```

### Toast

Singleton transient popups.

```cpp
static Toast& Instance();
void Show(std::string msg, ToastType type = ToastType::Info, float duration = 3.0f, std::function<void()> onDismiss = nullptr);
static void Info(std::string);  static void Success(std::string);
static void Warn(std::string);  static void Error(std::string);
void SetPosition(int anchor, float offsetX = 10, float offsetY = 10);
```

```cpp
unigui::Toast::Success("Saved!");   // remember to Render() a Toast instance each frame
```

### AlertBar

Persistent animated banner.

```cpp
AlertBar(std::string name);
void Show(std::string message); void Hide(); bool IsShown() const;
```

### Notification

Queued notifications.

```cpp
Notification(std::string name);
void Show(std::string title, std::string msg, float duration = 3.0f);
size_t PendingCount() const;
```

### Tooltip / ContextMenu

```cpp
unigui::Tooltip::Show("Hello");      // static helper

struct ContextMenuItem { std::string label; std::function<void()> action; bool separator = false; };
static void ContextMenu::Show(const char* id, std::vector<ContextMenuItem> items);
static void ContextMenu::ShowWindow(const char* id, std::vector<ContextMenuItem> items);
```

### TrayIcon

System tray icon with menu + balloon notifications.

```cpp
TrayIcon(std::string name, std::string title = "UniGUI", int iconId = 0);
bool Show(); void Hide();
void SetMenu(std::vector<TrayMenuItem> items);
void UpdateTooltip(std::string title);
void ShowNotification(std::string title, std::string msg, NotifyType type = NotifyType::Info);
void SetOnExit(std::function<void()> cb);
```

---

## 12. Forms & Properties

### Form

Field-based form with validation and JSON serialization.

```cpp
Form(std::string name, std::string title);
void AddTextField(std::string name, std::string label, bool required = false);
void AddCheckbox(std::string name, std::string label);
void AddComboField(std::string name, std::string label, std::vector<std::string> options);
void AddSliderField(std::string name, std::string label, float min = 0, float max = 100);
void AddNumberField(std::string name, std::string label, int min = 0, int max = 100);
std::string GetFieldValue(const std::string& name) const; void SetFieldValue(const std::string& name, std::string value);
std::vector<FormError> Validate() const; const std::vector<FormError>& GetErrors() const;
void SetFieldValidatorRegex(const std::string& name, std::string pattern, std::string errorMsg);
void SetFieldMinMax(const std::string& name, double min, double max);
void SetOnSubmit(std::function<void()> callback);
std::string Serialize() const; bool Deserialize(const std::string& json);
```

```cpp
auto form = std::make_shared<unigui::Form>("signup", "Sign Up");
form->AddTextField("user", "Username", /*required=*/true);
form->SetFieldValidatorRegex("user", "^[a-z0-9_]{3,}$", "lowercase, 3+ chars");
```

### PropertyGrid

Two-column key/value editor (like a VS Properties pane).

```cpp
PropertyGrid(std::string name);
struct PropertyDef { std::string name; std::string label; PropType type = PropType::String; PropValue value;
                     std::vector<std::string> options; float minVal = 0, maxVal = 100; bool readOnly = false; };
void AddProperty(PropertyDef prop); void Clear();
template<typename T> T GetValue(const std::string& name, T defaultVal = T{}) const;
void SetValue(const std::string& name, PropValue val);
void SetOnChange(std::function<void(const std::string& name, const PropValue& val)> fn);
const std::vector<PropertyDef>& GetProperties() const;
```

---

## 13. Charts & Domain Widgets

### TimeSeriesChart

Real-time implot plot with a sliding window, auto-fit Y, crosshair, reference lines, and pan/zoom.

```cpp
TimeSeriesChart(std::string name);
int AddSeries(TimeSeriesDef def); void RemoveSeries(int id);
void AppendPoint(int seriesId, float value, double timestamp = -1.0); void ClearAll();
void AppendSample(int seriesId, double timestamp, float value);  // (when, what) order
void UpsertPoint(int seriesId, float value, double timestamp);   // update-by-ts else append (live bar)
void SetSeriesData(int seriesId, const std::vector<double>& xs, const std::vector<double>& ys);
void SetSlidingWindow(int maxPoints);
void SetMaxRenderPoints(int n);                 // LTTB-decimate huge series to ~n
int GetSeriesPointCount(int seriesId) const;
void SetSessionAxis(SessionAxis axis);          // gap-free intraday X axis, HH:MM labels
void SetYAxisAutoFit(bool on); void SetYAxisRange(double min, double max);
void SetXAxisLabel(const std::string&); void SetYAxisLabel(const std::string&);
void SetCrosshairEnabled(bool); void SetLegendEnabled(bool);
void SetPanEnabled(bool); void SetZoomEnabled(bool); void SetRubberBandZoom(bool on);
void SetGridColor(ImU32 c); void SetThemeBackground(bool on);
void SetCrosshairFormatter(std::function<std::string(double,const std::vector<double>&)> fn);
void SetXAxisFormatter(std::function<int(double,char*,int,void*)> fn);
int AddRefLine(std::string label, double value, ImU32 color); void RemoveRefLine(int id);
```

### RiskBar

Animated progress bar with warn/danger thresholds (finance-oriented).

```cpp
RiskBar(std::string name);
void SetRatio(double); double GetRatio() const; void SetMaxRatio(double);
void SetDisplayText(std::string);
void SetWarnThreshold(double); void SetDangerThreshold(double);
void SetInverted(bool); void SetAnimated(bool);
```

### FuturesRiskBar

Multi-marker risk bar showing actual/estimated/overnight ratios.

```cpp
FuturesRiskBar(std::string name);
void SetAccountName(std::string); void SetMarginText(std::string);
void SetActualRatio(double); void SetEstimatedRatio(double); void SetOvernightRatio(double);
void SetAnimated(bool);
```

### SliderBar

Interactive bar with futures/price tick marks, fill state, and edit/confirm/rollback callbacks.

```cpp
SliderBar(std::string name);
struct Tick { int futuresLots = 0; double price = 0.0; };
void SetMaxValue(int maxLots); void SetTickColors(std::vector<ImU32> colors);
void SetTicks(std::vector<Tick>); std::vector<Tick> GetTicks() const;
int GetActiveTickIndex(double currentPrice, int currentLots) const;
void SetCurrentLots(int lots); void SetActiveFill(int from, int to, ImU32 color);
void SetOnChanged(std::function<void(const std::vector<Tick>&)> fn);
void SetLeftLabel(std::string); void SetLeftSubLabel(std::string);
void SetOnAdd(std::function<void()>); void SetOnConfirm(std::function<void()>);
void SetOnRollback(std::function<void()>); void SetOnSubmit(std::function<void()>);
bool HasUnsavedChanges() const;
```

### HeroSection

Full-width gradient banner with title/subtitle/action.

```cpp
HeroSection(std::string name, std::string title = "", std::string subtitle = "");
void SetTitle(std::string); void SetSubtitle(std::string);
void SetBackground(ImU32 topColor, ImU32 bottomColor);
void SetActionButton(std::string label, std::function<void()> callback); void SetHeight(float h);
```

### Sparkline

Compact axis-less trend chart (Line/Area/Bar) for inline use in tables,
watchlists, and KPI cards. Auto- or fixed-range; no ImPlot dependency.

```cpp
Sparkline(std::string name, Mode mode = Mode::Line);
enum class Mode { Line, Area, Bar };
void SetData(std::vector<float>); void PushValue(float); void Clear();
void SetMaxPoints(std::size_t);            // rolling cap for PushValue (0 = unbounded)
void SetMode(Mode); void SetSize(float w, float h);
void SetLineColor(ImU32); void SetFillColor(ImU32); void SetLineThickness(float);
void SetShowLastDot(bool); void SetColorByTrend(bool);   // green up / red down
void SetRange(float min, float max); void SetAutoRange();
// Fluent: WithData/WithMode/WithSize/WithLineColor/WithFillColor/
//         WithLineThickness/WithShowLastDot/WithColorByTrend/WithRange
```

### PriceTicker

Horizontally scrolling symbol/price/Δ marquee with green/red ▲/▼ tinting — the
classic trading header strip. Adjustable speed and pause.

```cpp
PriceTicker(std::string name, std::vector<Item> items = {});
struct Item { std::string symbol; std::string price; float change = 0.f; };
void SetItems(std::vector<Item>); void AddItem(Item); void Clear();
void SetSpeed(float pxPerSec); void SetPaused(bool);
void SetHeight(float); void SetWidth(float);            // width 0 = full available
void SetUpColor(ImU32); void SetDownColor(ImU32);
float GetScrollOffset() const;
// Fluent: WithItems/WithSpeed/WithPaused/WithHeight/WithWidth
```

### MetricCard

Bordered KPI/status tile — optional accent rail, a header (status dot + accent
title + right-aligned action slot), and a value/delta/subtext body or a custom
draw callback. The delta is sign-coloured via the active `Up`/`Down` polarity.

```cpp
MetricCard(std::string name);
// Fluent: WithTitle/WithStatusDot(theme::Semantic)/WithHeaderActions(fn)/
//         WithValue/WithDelta(value, display)/WithSubtext/WithBody(fn)/
//         WithSize(w,h)/WithAccentRail(bool)
```

### ConnectionStatusBar

Link-health strip composing `StatusLamp` + `Sparkline` with an adaptive,
threshold-graded latency readout (`format::Latency`), FPS, and a reconnect
countdown. RTT averaging / reconnect FSM stay in the caller.

```cpp
ConnectionStatusBar(std::string name);
void PushLatencySample(double us);   // feeds the embedded sparkline
// Fluent: WithConnected(bool)/WithCaption/WithLatencyUs(cur, avg)/
//         WithLatencyThresholds(warnUs, critUs)/WithFps(f)/WithReconnectIn(s)/WithSparkline(bool)
```

---

## 14. Loading & Skeleton

### LoadingIndicator

```cpp
LoadingIndicator(std::string name, float radius = 16.0f);
void SetActive(bool active); bool IsActive() const;
```

### Skeleton (`SkeletonScreen`)

Content placeholder built from blocks/lines/circles, with optional shimmer.

```cpp
SkeletonScreen();
void AddBlock(float width, float height, float x = 0.f, float y = 0.f);
void AddLine(float width, float x = 0.f, float y = 0.f);
void AddCircle(float radius, float x = 0.f, float y = 0.f);
void SetShimmer(bool enable, float speed = 1.2f);
static SkeletonScreen FromSize(float w, float h, int lineCount = 4);
```

### Shimmer

Standalone animated gradient sweep over blocks/circles.

```cpp
Shimmer();
void AddBlock(float width, float height, float x = 0.f, float y = 0.f);
void AddCircle(float radius, float x = 0.f, float y = 0.f);
void Start(); void Stop(); bool IsPlaying() const; void SetSpeed(float s);
```

---

## 15. Utilities

### Animate (`unigui::Animate`)

```cpp
float Animate::FadeIn(float duration = 0.3f); void Animate::FadeInReset();
float Animate::SlideIn(float duration = 0.3f, float fromOffset = -50.0f);
float Animate::Lerp(float current, float target, float speed = 0.1f);
struct Animate::FadeScope { FadeScope(float target = 1.0f, float duration = 0.3f); };
```

### Clipboard (`unigui::Clipboard`)

```cpp
void Clipboard::Copy(const std::string& text);
std::string Clipboard::Paste();
```

### DragDrop (`unigui::DragDrop`)

```cpp
template<typename T> bool DragDrop::BeginDragSource(const char* type, const T& data);
template<typename T> const T* DragDrop::AcceptDragDrop(const char* type);
```

### ShortcutManager

```cpp
ShortcutManager mgr;
mgr.Register(ImGuiKey_S, /*ctrl=*/true, []{ doSave(); }, "Save");
// each frame:
mgr.Process();
```

### ValueWidget\<T> (base)

Base for value-holding widgets (sliders, spinboxes, drag inputs, toggles):

```cpp
T GetValue() const; void SetValue(T val);
void SetOnChange(std::function<void(T)> fn);
```

---

## 16. Deep-dive components (in this file)

| Component | Section |
|-----------|---------|
| TreeView (`TextSpan`, `spans`, `SetRowRenderer`) | [§ TreeView](#treeview) |
| CascadingCombo (`Layout`, `SetShowLabels`, widths) | [§ CascadingCombo](#cascadingcombo) |

Legacy standalone files [TREEVIEW.md](TREEVIEW.md) / [CASCADINGCOMBO.md](CASCADINGCOMBO.md) redirect here.
Per-widget copy-paste: [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md).

### StatusLamp glow

`StatusLamp` draws a glossy LED whose glow can be toggled with `SetGlowEnabled(true)`. The glow's
vertical padding is included in the widget bounds, so it lays out correctly inside tables and rows.

---

> Sub-modules (DSL, EventBus, CSS styling, fonts, plugins, config, SQLite, IPC, networking) are
> documented in the [main README](../README.md#sub-modules). All sub-module headers are pulled in by
> `<unigui/unigui.h>`.
